#include "pd/renderer/opengl_renderer.hpp"

#include "gl_api.hpp"

#include <SDL3/SDL.h>

#include <array>
#include <string>

namespace pd::renderer {
namespace {
using namespace gl;

#if defined(PD_OPENGL_ES) && PD_OPENGL_ES
constexpr const char* kVertexShaderSource = R"GLSL(
attribute vec2 aPosition;
varying vec2 vUv;
void main() {
    vUv = aPosition * 0.5 + 0.5;
    gl_Position = vec4(aPosition, 0.0, 1.0);
}
)GLSL";

constexpr const char* kFragmentShaderSource = R"GLSL(
precision mediump float;
varying vec2 vUv;
uniform float uTime;
uniform float uPlayerX;
uniform float uPlayerZ;
uniform float uYaw;

float hash(float n) {
    return fract(sin(n) * 43758.5453123);
}

void main() {
    vec2 uv = vUv;
    float horizon = 0.43;
    vec3 skyTop = vec3(0.012, 0.018, 0.042);
    vec3 skyBottom = vec3(0.075, 0.025, 0.095);
    vec3 color = mix(skyBottom, skyTop, smoothstep(0.0, horizon, uv.y));

    float column = floor(uv.x * 25.0);
    float height = 0.045 + hash(column + 7.0) * 0.22;
    float building = step(horizon - height, uv.y) * step(uv.y, horizon);
    float windowNoise = step(0.82, hash(floor(uv.x * 92.0) + floor(uv.y * 54.0) * 13.0));
    vec3 buildingColor = vec3(0.025, 0.035, 0.072) + vec3(0.02, 0.13, 0.17) * windowNoise;
    color = mix(color, buildingColor, building);

    if (uv.y < horizon) {
        float depth = max(horizon - uv.y, 0.012);
        float perspectiveX = (uv.x - 0.5) / depth;
        float perspectiveY = 1.0 / depth;
        float gridX = 1.0 - smoothstep(0.018, 0.045, abs(fract(perspectiveX * 0.34) - 0.5));
        float gridY = 1.0 - smoothstep(0.018, 0.05, abs(fract(perspectiveY * 0.055) - 0.5));
        float grid = max(gridX, gridY);
        float fade = 1.0 - smoothstep(0.0, 0.65, depth);
        color = mix(vec3(0.012, 0.018, 0.034), vec3(0.035, 0.20, 0.24), grid * fade * 0.72);
    }

    float horizonGlow = exp(-abs(uv.y - horizon) * 34.0);
    color += vec3(0.12, 0.025, 0.16) * horizonGlow;

    vec2 center = abs(uv - vec2(0.5));
    float crosshair = (1.0 - smoothstep(0.003, 0.006, center.x)) * step(center.y, 0.018)
                    + (1.0 - smoothstep(0.003, 0.006, center.y)) * step(center.x, 0.018);
    color += vec3(0.24, 0.76, 0.78) * crosshair * 0.72;

    float scanline = 0.96 + 0.04 * sin(uv.y * 720.0);
    float vignette = 1.0 - smoothstep(0.38, 0.78, length((uv - 0.5) * vec2(1.0, 0.8)));
    float pulse = 0.985 + 0.015 * sin(uTime * 1.7 + uYaw + uPlayerX * 0.03 + uPlayerZ * 0.02);
    color *= scanline * vignette * pulse;
    gl_FragColor = vec4(color, 1.0);
}
)GLSL";
#else
constexpr const char* kVertexShaderSource = R"GLSL(
#version 120
attribute vec2 aPosition;
varying vec2 vUv;
void main() {
    vUv = aPosition * 0.5 + 0.5;
    gl_Position = vec4(aPosition, 0.0, 1.0);
}
)GLSL";

constexpr const char* kFragmentShaderSource = R"GLSL(
#version 120
varying vec2 vUv;
uniform float uTime;
uniform float uPlayerX;
uniform float uPlayerZ;
uniform float uYaw;

float hash(float n) {
    return fract(sin(n) * 43758.5453123);
}

void main() {
    vec2 uv = vUv;
    float horizon = 0.43;
    vec3 skyTop = vec3(0.012, 0.018, 0.042);
    vec3 skyBottom = vec3(0.075, 0.025, 0.095);
    vec3 color = mix(skyBottom, skyTop, smoothstep(0.0, horizon, uv.y));

    float column = floor(uv.x * 25.0);
    float height = 0.045 + hash(column + 7.0) * 0.22;
    float building = step(horizon - height, uv.y) * step(uv.y, horizon);
    float windowNoise = step(0.82, hash(floor(uv.x * 92.0) + floor(uv.y * 54.0) * 13.0));
    vec3 buildingColor = vec3(0.025, 0.035, 0.072) + vec3(0.02, 0.13, 0.17) * windowNoise;
    color = mix(color, buildingColor, building);

    if (uv.y < horizon) {
        float depth = max(horizon - uv.y, 0.012);
        float perspectiveX = (uv.x - 0.5) / depth;
        float perspectiveY = 1.0 / depth;
        float gridX = 1.0 - smoothstep(0.018, 0.045, abs(fract(perspectiveX * 0.34) - 0.5));
        float gridY = 1.0 - smoothstep(0.018, 0.05, abs(fract(perspectiveY * 0.055) - 0.5));
        float grid = max(gridX, gridY);
        float fade = 1.0 - smoothstep(0.0, 0.65, depth);
        color = mix(vec3(0.012, 0.018, 0.034), vec3(0.035, 0.20, 0.24), grid * fade * 0.72);
    }

    float horizonGlow = exp(-abs(uv.y - horizon) * 34.0);
    color += vec3(0.12, 0.025, 0.16) * horizonGlow;

    vec2 center = abs(uv - vec2(0.5));
    float crosshair = (1.0 - smoothstep(0.003, 0.006, center.x)) * step(center.y, 0.018)
                    + (1.0 - smoothstep(0.003, 0.006, center.y)) * step(center.x, 0.018);
    color += vec3(0.24, 0.76, 0.78) * crosshair * 0.72;

    float scanline = 0.96 + 0.04 * sin(uv.y * 720.0);
    float vignette = 1.0 - smoothstep(0.38, 0.78, length((uv - 0.5) * vec2(1.0, 0.8)));
    float pulse = 0.985 + 0.015 * sin(uTime * 1.7 + uYaw + uPlayerX * 0.03 + uPlayerZ * 0.02);
    color *= scanline * vignette * pulse;
    gl_FragColor = vec4(color, 1.0);
}
)GLSL";
#endif

GLuint compile_shader(Api& api, GLenum type, const char* source) {
    const GLuint shader = api.CreateShader(type);
    if (shader == 0) {
        SDL_LogError(SDL_LOG_CATEGORY_RENDER, "glCreateShader failed");
        return 0;
    }

    api.ShaderSource(shader, 1, &source, nullptr);
    api.CompileShader(shader);

    GLint compiled = 0;
    api.GetShaderiv(shader, kCompileStatus, &compiled);
    if (compiled == 0) {
        GLint log_length = 0;
        api.GetShaderiv(shader, kInfoLogLength, &log_length);
        std::string log(static_cast<std::size_t>(log_length > 1 ? log_length : 1), '\0');
        api.GetShaderInfoLog(shader, static_cast<GLsizei>(log.size()), nullptr, log.data());
        SDL_LogError(SDL_LOG_CATEGORY_RENDER, "Shader compilation failed: %s", log.c_str());
        api.DeleteShader(shader);
        return 0;
    }
    return shader;
}

GLuint link_program(Api& api, GLuint vertex_shader, GLuint fragment_shader) {
    const GLuint program = api.CreateProgram();
    if (program == 0) {
        SDL_LogError(SDL_LOG_CATEGORY_RENDER, "glCreateProgram failed");
        return 0;
    }

    api.AttachShader(program, vertex_shader);
    api.AttachShader(program, fragment_shader);
    api.BindAttribLocation(program, 0, "aPosition");
    api.LinkProgram(program);

    GLint linked = 0;
    api.GetProgramiv(program, kLinkStatus, &linked);
    if (linked == 0) {
        GLint log_length = 0;
        api.GetProgramiv(program, kInfoLogLength, &log_length);
        std::string log(static_cast<std::size_t>(log_length > 1 ? log_length : 1), '\0');
        api.GetProgramInfoLog(program, static_cast<GLsizei>(log.size()), nullptr, log.data());
        SDL_LogError(SDL_LOG_CATEGORY_RENDER, "Program link failed: %s", log.c_str());
        api.DeleteProgram(program);
        return 0;
    }
    return program;
}
} // namespace

struct OpenGLRenderer::Impl {
    gl::Api api{};
    gl::GLuint program = 0;
    gl::GLuint vertex_buffer = 0;
    gl::GLint time_location = -1;
    gl::GLint player_x_location = -1;
    gl::GLint player_z_location = -1;
    gl::GLint yaw_location = -1;
    int width = 1280;
    int height = 720;
    bool initialized = false;
};

OpenGLRenderer::OpenGLRenderer() : impl_(std::make_unique<Impl>()) {}
OpenGLRenderer::~OpenGLRenderer() { shutdown(); }

bool OpenGLRenderer::initialize() {
    if (impl_->initialized) return true;

    auto& api = impl_->api;
    if (!api.load()) return false;

    const GLuint vertex_shader = compile_shader(api, gl::kVertexShader, kVertexShaderSource);
    if (vertex_shader == 0) return false;

    const GLuint fragment_shader = compile_shader(api, gl::kFragmentShader, kFragmentShaderSource);
    if (fragment_shader == 0) {
        api.DeleteShader(vertex_shader);
        return false;
    }

    impl_->program = link_program(api, vertex_shader, fragment_shader);
    api.DeleteShader(vertex_shader);
    api.DeleteShader(fragment_shader);
    if (impl_->program == 0) return false;

    constexpr std::array<float, 6> fullscreen_triangle{
        -1.0F, -1.0F,
         3.0F, -1.0F,
        -1.0F,  3.0F
    };

    api.GenBuffers(1, &impl_->vertex_buffer);
    api.BindBuffer(gl::kArrayBuffer, impl_->vertex_buffer);
    api.BufferData(
        gl::kArrayBuffer,
        static_cast<gl::GLsizeiptr>(sizeof(fullscreen_triangle)),
        fullscreen_triangle.data(),
        gl::kStaticDraw
    );

    impl_->time_location = api.GetUniformLocation(impl_->program, "uTime");
    impl_->player_x_location = api.GetUniformLocation(impl_->program, "uPlayerX");
    impl_->player_z_location = api.GetUniformLocation(impl_->program, "uPlayerZ");
    impl_->yaw_location = api.GetUniformLocation(impl_->program, "uYaw");

    impl_->initialized = true;
    resize(impl_->width, impl_->height);
    SDL_Log("OpenGL renderer ready. Procedural cyberpunk test scene loaded.");
    return true;
}

void OpenGLRenderer::resize(int width, int height) noexcept {
    impl_->width = width > 0 ? width : 1;
    impl_->height = height > 0 ? height : 1;
    if (impl_->initialized) {
        impl_->api.Viewport(0, 0, impl_->width, impl_->height);
    }
}

void OpenGLRenderer::render(const pd::core::GameState& state, float time_seconds) noexcept {
    if (!impl_->initialized) return;

    auto& api = impl_->api;
    api.Viewport(0, 0, impl_->width, impl_->height);
    api.ClearColor(0.012F, 0.015F, 0.03F, 1.0F);
    api.Clear(gl::kColorBufferBit);

    api.UseProgram(impl_->program);
    if (impl_->time_location >= 0) api.Uniform1f(impl_->time_location, time_seconds);
    if (impl_->player_x_location >= 0) api.Uniform1f(impl_->player_x_location, state.player.x);
    if (impl_->player_z_location >= 0) api.Uniform1f(impl_->player_z_location, state.player.z);
    if (impl_->yaw_location >= 0) api.Uniform1f(impl_->yaw_location, state.player.yaw_radians);

    api.BindBuffer(gl::kArrayBuffer, impl_->vertex_buffer);
    api.EnableVertexAttribArray(0);
    api.VertexAttribPointer(0, 2, gl::kFloat, 0, 0, nullptr);
    api.DrawArrays(gl::kTriangles, 0, 3);
    api.DisableVertexAttribArray(0);
}

void OpenGLRenderer::shutdown() noexcept {
    if (!impl_ || !impl_->initialized) return;
    auto& api = impl_->api;
    if (impl_->vertex_buffer != 0) api.DeleteBuffers(1, &impl_->vertex_buffer);
    if (impl_->program != 0) api.DeleteProgram(impl_->program);
    impl_->vertex_buffer = 0;
    impl_->program = 0;
    impl_->initialized = false;
}

bool OpenGLRenderer::is_initialized() const noexcept {
    return impl_ && impl_->initialized;
}
} // namespace pd::renderer
