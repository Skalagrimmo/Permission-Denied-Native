#include "sdl_application.hpp"

#include "pd/core/game.hpp"
#include "pd/renderer/opengl_renderer.hpp"

#include <SDL3/SDL.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>

namespace pd::platform::sdl {
namespace {
using Clock = std::chrono::steady_clock;
constexpr double kFixedStep = 1.0 / 60.0;
constexpr double kMaxFrameDelta = 0.25;
constexpr float kTouchLookRadiansPerScreen = 4.4F;

struct KeyState {
    bool forward = false;
    bool backward = false;
    bool left = false;
    bool right = false;
    bool sprint = false;
    bool look_left = false;
    bool look_right = false;
};

struct FingerBinding {
    bool active = false;
    SDL_FingerID id = 0;
};

struct TouchControls {
    FingerBinding move_finger{};
    FingerBinding look_finger{};
    FingerBinding sprint_finger{};
    float move_x = 0.0F;
    float move_y = 0.0F;
    float last_look_x = 0.0F;
    float last_look_y = 0.0F;
    float pending_look_radians = 0.0F;
    float pending_look_pitch_radians = 0.0F;
    bool sprint = false;
};

pd::core::InputState make_input(const KeyState& keys, const TouchControls& touch) {
    pd::core::InputState input{};
    input.move_z = static_cast<float>(keys.forward) - static_cast<float>(keys.backward);
    input.move_x = static_cast<float>(keys.right) - static_cast<float>(keys.left);
    if (touch.move_finger.active) {
        // The stick already uses the game's local basis:
        // screen-right is +X and screen-up is +Z (forward). Do not invert it again.
        input.move_x += touch.move_x;
        input.move_z += touch.move_y;
    }
    input.sprint = keys.sprint || touch.sprint;
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

void update_virtual_stick(TouchControls& touch, float finger_x, float finger_y, float aspect) {
    constexpr float kStickCenterX = 0.16F;
    constexpr float kStickCenterY = 0.78F;
    constexpr float kStickRadius = 0.125F;

    const float dx = (finger_x - kStickCenterX) * aspect / kStickRadius;
    const float dy = (kStickCenterY - finger_y) / kStickRadius;
    const float magnitude = std::sqrt(dx * dx + dy * dy);
    constexpr float kDeadzone = 0.10F;
    if (magnitude <= kDeadzone) {
        touch.move_x = 0.0F;
        touch.move_y = 0.0F;
        return;
    }

    const float clamped_magnitude = std::min(magnitude, 1.0F);
    const float remapped_magnitude = (clamped_magnitude - kDeadzone) / (1.0F - kDeadzone);
    const float scale = remapped_magnitude / magnitude;
    touch.move_x = dx * scale;
    touch.move_y = dy * scale;
}

void handle_finger_down(
    TouchControls& touch,
    const SDL_TouchFingerEvent& finger,
    float aspect) {
    const float screen_x = finger.x * aspect;
    const float screen_y = 1.0F - finger.y;

    // Sprint button: fixed at the lower-right corner.
    const float sprint_dx = screen_x - aspect * 0.88F;
    const float sprint_dy = screen_y - 0.21F;
    // Generous touch target: the hit area is much larger than the visible icon.
    if (!touch.sprint_finger.active && std::sqrt(sprint_dx * sprint_dx + sprint_dy * sprint_dy) <= 0.23F) {
        touch.sprint_finger = {true, finger.fingerID};
        touch.sprint = true;
        SDL_Log("Touch sprint down: id=%lld", static_cast<long long>(finger.fingerID));
        // Do not return: the right-hand area is still the camera-look zone.
        // A finger can hold sprint and look at the same time.
    }

    // Movement stick: left/lower quadrant, with a fixed center.
    if (!touch.move_finger.active && screen_x < aspect * 0.43F && finger.y > 0.47F) {
        touch.move_finger = {true, finger.fingerID};
        update_virtual_stick(touch, finger.x, finger.y, aspect);
        SDL_Log("Touch move down: id=%lld axis=(%.2f, %.2f)",
            static_cast<long long>(finger.fingerID), touch.move_x, touch.move_y);
        return;
    }

    // Any other touch on the right side becomes a relative-look gesture.
    if (!touch.look_finger.active && screen_x >= aspect * 0.45F) {
        touch.look_finger = {true, finger.fingerID};
        touch.last_look_x = finger.x;
        touch.last_look_y = finger.y;
        SDL_Log("Touch look down: id=%lld x=%.3f y=%.3f",
            static_cast<long long>(finger.fingerID), finger.x, finger.y);
    }
}

void handle_finger_motion(TouchControls& touch, const SDL_TouchFingerEvent& finger, float aspect) {
    if (touch.move_finger.active && touch.move_finger.id == finger.fingerID) {
        update_virtual_stick(touch, finger.x, finger.y, aspect);
    }

    if (touch.look_finger.active && touch.look_finger.id == finger.fingerID) {
        const float delta_x = finger.x - touch.last_look_x;
        const float delta_y = finger.y - touch.last_look_y;
        touch.pending_look_radians += delta_x * kTouchLookRadiansPerScreen;
        // Screen Y grows downward, so an upward swipe looks upward.
        touch.pending_look_pitch_radians -= delta_y * kTouchLookRadiansPerScreen;
        touch.last_look_x = finger.x;
        touch.last_look_y = finger.y;
    }
}

void handle_finger_up(TouchControls& touch, const SDL_TouchFingerEvent& finger) {
    if (touch.move_finger.active && touch.move_finger.id == finger.fingerID) {
        SDL_Log("Touch move up: id=%lld finalAxis=(%.2f, %.2f)",
            static_cast<long long>(finger.fingerID), touch.move_x, touch.move_y);
        touch.move_finger = {};
        touch.move_x = 0.0F;
        touch.move_y = 0.0F;
    }
    if (touch.look_finger.active && touch.look_finger.id == finger.fingerID) {
        SDL_Log("Touch look up: id=%lld", static_cast<long long>(finger.fingerID));
        touch.look_finger = {};
    }
    if (touch.sprint_finger.active && touch.sprint_finger.id == finger.fingerID) {
        SDL_Log("Touch sprint up: id=%lld", static_cast<long long>(finger.fingerID));
        touch.sprint_finger = {};
        touch.sprint = false;
    }
}

void reset_touch(TouchControls& touch) {
    touch = TouchControls{};
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
    TouchControls touch{};
    bool running = true;
    double accumulator = 0.0;
    auto previous_time = Clock::now();
    auto title_time = previous_time;
    unsigned int frames = 0;

    SDL_Log("Permission Denied native engine initialized.");
#if defined(__ANDROID__)
    SDL_Log("Touch: left stick move, right-side swipe look, lower-right button sprint.");
#else
    SDL_Log("Controls: WASD move, SHIFT sprint, LEFT/RIGHT arrows look, ESC quit.");
#endif

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
                case SDL_EVENT_FINGER_DOWN:
                case SDL_EVENT_FINGER_MOTION:
                case SDL_EVENT_FINGER_UP: {
                    int point_width = 1;
                    int point_height = 1;
                    SDL_GetWindowSize(window, &point_width, &point_height);
                    const float aspect = static_cast<float>(point_width) /
                        static_cast<float>(std::max(point_height, 1));
                    if (event.type == SDL_EVENT_FINGER_DOWN) {
                        handle_finger_down(touch, event.tfinger, aspect);
                    } else if (event.type == SDL_EVENT_FINGER_MOTION) {
                        handle_finger_motion(touch, event.tfinger, aspect);
                    } else {
                        handle_finger_up(touch, event.tfinger);
                    }
                    break;
                }
                case SDL_EVENT_WINDOW_FOCUS_LOST:
                    keys = KeyState{};
                    reset_touch(touch);
                    break;
                default:
                    break;
            }
        }

        const auto now = Clock::now();
        const double frame_delta = std::chrono::duration<double>(now - previous_time).count();
        previous_time = now;
        accumulator += std::min(frame_delta, kMaxFrameDelta);

        const int pending_steps = static_cast<int>(accumulator / kFixedStep);
        const float look_delta_per_step = pending_steps > 0
            ? touch.pending_look_radians / static_cast<float>(pending_steps)
            : 0.0F;
        const float pitch_delta_per_step = pending_steps > 0
            ? touch.pending_look_pitch_radians / static_cast<float>(pending_steps)
            : 0.0F;
        int executed_steps = 0;
        while (accumulator >= kFixedStep) {
            auto input = make_input(keys, touch);
            input.look_delta_radians = look_delta_per_step;
            input.look_delta_pitch_radians = pitch_delta_per_step;
            game.update(input, static_cast<float>(kFixedStep));
            accumulator -= kFixedStep;
            ++executed_steps;
        }
        if (executed_steps > 0) {
            touch.pending_look_radians = 0.0F;
            touch.pending_look_pitch_radians = 0.0F;
        }

        int width = 0;
        int height = 0;
        SDL_GetWindowSizeInPixels(window, &width, &height);
        renderer.resize(width, height);

        pd::renderer::TouchHudState touch_hud{};
#if defined(__ANDROID__)
        touch_hud.visible = true;
#endif
        touch_hud.move_x = touch.move_x;
        touch_hud.move_y = touch.move_y;
        touch_hud.move_active = touch.move_finger.active;
        touch_hud.sprint_active = touch.sprint;
        renderer.render(game.state(), static_cast<float>(game.state().elapsed_seconds), touch_hud);
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
