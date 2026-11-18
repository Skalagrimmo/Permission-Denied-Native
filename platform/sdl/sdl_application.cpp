#include "sdl_application.hpp"

#include "pd/core/game.hpp"
#include "pd/renderer/opengl_renderer.hpp"

#include <SDL3/SDL.h>

#include <algorithm>
#include <chrono>
#include <cstdio>

namespace pd::platform::sdl {
namespace {
using Clock = std::chrono::steady_clock;
constexpr double kFixedStep = 1.0 / 60.0;
constexpr double kMaxFrameDelta = 0.25;

struct KeyState {
    bool forward = false;
    bool backward = false;
    bool left = false;
    bool right = false;
    bool sprint = false;
    bool look_left = false;
    bool look_right = false;
};

pd::core::InputState make_input(const KeyState& keys) {
    pd::core::InputState input{};
    input.move_z = static_cast<float>(keys.forward) - static_cast<float>(keys.backward);
    input.move_x = static_cast<float>(keys.right) - static_cast<float>(keys.left);
    input.sprint = keys.sprint;
    input.look_x = static_cast<float>(keys.look_right) - static_cast<float>(keys.look_left);
    return input;
}

void update_key(KeyState& keys, SDL_Keycode key, bool down) {
    switch (key) {
        case SDLK_W:
        case SDLK_UP: keys.forward = down; break;
        case SDLK_S:
        case SDLK_DOWN: keys.backward = down; break;
        case SDLK_A: keys.left = down; break;
        case SDLK_D: keys.right = down; break;
        case SDLK_LSHIFT:
        case SDLK_RSHIFT: keys.sprint = down; break;
        case SDLK_LEFT: keys.look_left = down; break;
        case SDLK_RIGHT: keys.look_right = down; break;
        default: break;
    }
}
} // namespace

int SdlApplication::run() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "SDL_Init failed: %s", SDL_GetError());
        return 1;
    }

#if defined(__ANDROID__)
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#else
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
#endif
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);

#if defined(__ANDROID__)
    SDL_SetHint(SDL_HINT_ORIENTATIONS, "LandscapeLeft LandscapeRight");
#endif

    SDL_Window* window = SDL_CreateWindow(
        "PERMISSION DENIED // Native Engine",
        1280,
        720,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY
    );

    if (window == nullptr) {
        SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "SDL_CreateWindow failed: %s", SDL_GetError());
        SDL_Quit();
        return 2;
    }

    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    if (gl_context == nullptr) {
        SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "SDL_GL_CreateContext failed: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 3;
    }

    if (!SDL_GL_MakeCurrent(window, gl_context)) {
        SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "SDL_GL_MakeCurrent failed: %s", SDL_GetError());
        SDL_GL_DestroyContext(gl_context);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 4;
    }

    SDL_GL_SetSwapInterval(1);

    pd::renderer::OpenGLRenderer renderer;
    if (!renderer.initialize()) {
        SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION, "OpenGL renderer initialization failed");
        SDL_GL_DestroyContext(gl_context);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 5;
    }

    pd::core::Game game;
    KeyState keys{};
    bool running = true;
    double accumulator = 0.0;
    auto previous_time = Clock::now();
    auto title_time = previous_time;
    unsigned int frames = 0;

    SDL_Log("Permission Denied native engine initialized.");
    SDL_Log("Controls: WASD move, SHIFT sprint, LEFT/RIGHT arrows look, ESC quit.");

    while (running) {
        SDL_Event event{};
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_EVENT_QUIT:
                    running = false;
                    break;
                case SDL_EVENT_KEY_DOWN:
                    if (event.key.key == SDLK_ESCAPE) {
                        running = false;
                    } else {
                        update_key(keys, event.key.key, true);
                    }
                    break;
                case SDL_EVENT_KEY_UP:
                    update_key(keys, event.key.key, false);
                    break;
                case SDL_EVENT_WINDOW_FOCUS_LOST:
                    keys = KeyState{};
                    break;
                default:
                    break;
            }
        }

        const auto now = Clock::now();
        const double frame_delta = std::chrono::duration<double>(now - previous_time).count();
        previous_time = now;
        accumulator += std::min(frame_delta, kMaxFrameDelta);

        const auto input = make_input(keys);
        while (accumulator >= kFixedStep) {
            game.update(input, static_cast<float>(kFixedStep));
            accumulator -= kFixedStep;
        }

        int width = 0;
        int height = 0;
        SDL_GetWindowSizeInPixels(window, &width, &height);
        renderer.resize(width, height);
        renderer.render(game.state(), static_cast<float>(game.state().elapsed_seconds));
        SDL_GL_SwapWindow(window);

        ++frames;
        const double title_elapsed = std::chrono::duration<double>(now - title_time).count();
        if (title_elapsed >= 0.5) {
            const float fps = static_cast<float>(frames / title_elapsed);
            char title[160]{};
            std::snprintf(
                title,
                sizeof(title),
                "PERMISSION DENIED // Native Core // %.0f FPS // X %.1f Z %.1f",
                fps,
                game.state().player.x,
                game.state().player.z
            );
            SDL_SetWindowTitle(window, title);
            frames = 0;
            title_time = now;
        }
    }

    renderer.shutdown();
    SDL_GL_DestroyContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
} // namespace pd::platform::sdl
