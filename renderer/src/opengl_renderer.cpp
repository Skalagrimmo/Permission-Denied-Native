#include "pd/renderer/opengl_renderer.hpp"

#include "gl_api.hpp"
#include "vector_scene.hpp"

#include <SDL3/SDL.h>

#include <array>
#include <cstddef>
#include <string>
#include <vector>

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
uniform float uPitch;
uniform float uAspect;
uniform float uMoveX;
uniform float uMoveY;
uniform float uMoveActive;
uniform float uSprintActive;
uniform float uTouchVisible;

float hash(float n) {
    return fract(sin(n) * 43758.5453123);
}

void main() {
    vec2 uv = vUv;
    // Positive pitch looks upward, moving the horizon lower in the frame.
    float horizon = clamp(0.43 - uPitch * 0.35, 0.22, 0.64);
    vec3 skyTop = vec3(0.012, 0.018, 0.042);
    vec3 skyBottom = vec3(0.075, 0.025, 0.095);
    vec3 color = mix(skyBottom, skyTop, smoothstep(0.0, horizon, uv.y));

    // The distant skyline is a panorama: yaw rotates it across the screen too.
    float panoramaU = fract(uv.x + uYaw * 0.15915494309);
    float column = floor(panoramaU * 25.0);
    float height = 0.045 + hash(column + 7.0) * 0.22;
    float building = step(horizon - height, uv.y) * step(uv.y, horizon);
    float windowNoise = step(0.82, hash(floor(panoramaU * 92.0) + floor(uv.y * 54.0) * 13.0));
    vec3 buildingColor = vec3(0.025, 0.035, 0.072) + vec3(0.02, 0.13, 0.17) * windowNoise;
    color = mix(color, buildingColor, building);

    if (uv.y < horizon) {
        float depth = max(horizon - uv.y, 0.012);
        // Cast a view ray, rotate it by camera yaw, then sample the static world grid.
        // Player translation offsets world coordinates; yaw rotates both axes together.
        float cameraX = (uv.x - 0.5) / depth;
        float cameraZ = 1.0 / depth;
        float yawCos = cos(uYaw);
        float yawSin = sin(uYaw);
        float perspectiveX = cameraX * yawCos - cameraZ * yawSin
                           - (uPlayerX - 2.5) * 0.34;
        float perspectiveY = cameraX * yawSin + cameraZ * yawCos
                           - (uPlayerZ - 2.5) * 0.38;
        float gridX = 1.0 - smoothstep(0.018, 0.045, abs(fract(perspectiveX * 0.34) - 0.5));
        float gridY = 1.0 - smoothstep(0.018, 0.05, abs(fract(perspectiveY * 0.055) - 0.5));
        float grid = max(gridX, gridY);
        float fade = 1.0 - smoothstep(0.0, 0.65, depth);
        color = mix(vec3(0.012, 0.018, 0.034), vec3(0.035, 0.20, 0.24), grid * fade * 0.28);
    }

    float horizonGlow = exp(-abs(uv.y - horizon) * 34.0);
    color += vec3(0.12, 0.025, 0.16) * horizonGlow;

    vec2 center = abs(uv - vec2(0.5));
    float crosshair = (1.0 - smoothstep(0.003, 0.006, center.x)) * step(center.y, 0.018)
                    + (1.0 - smoothstep(0.003, 0.006, center.y)) * step(center.x, 0.018);
    color += vec3(0.24, 0.76, 0.78) * crosshair * 0.72;

    // Native touch HUD: a left movement stick and a right sprint button.
    // Coordinates are measured in screen-height units to keep circular controls round.
    float aspect = max(uAspect, 1.0);
    vec2 hudPosition = vec2(uv.x * aspect, uv.y);
    vec2 stickCenter = vec2(aspect * 0.16, 0.21);
    float stickDistance = length(hudPosition - stickCenter);
    float stickDisk = 1.0 - smoothstep(0.112, 0.132, stickDistance);
    float stickRing = smoothstep(0.112, 0.118, stickDistance)
                    * (1.0 - smoothstep(0.128, 0.134, stickDistance));
    color = mix(color, vec3(0.012, 0.050, 0.070), stickDisk * uTouchVisible * 0.58);
    color += vec3(0.025, 0.40, 0.48) * stickRing * uTouchVisible * 0.75;

    vec2 stickKnobCenter = stickCenter + vec2(uMoveX, uMoveY) * 0.066;
    float stickKnob = 1.0 - smoothstep(0.038, 0.048, length(hudPosition - stickKnobCenter));
    color = mix(
        color,
        vec3(0.10, 0.67, 0.75),
        stickKnob * uTouchVisible * mix(0.38, 0.95, uMoveActive)
    );

    vec2 sprintCenter = vec2(aspect * 0.88, 0.21);
    float sprintDistance = length(hudPosition - sprintCenter);
    float sprintDisk = 1.0 - smoothstep(0.118, 0.136, sprintDistance);
    float sprintRing = smoothstep(0.132, 0.140, sprintDistance)
                     * (1.0 - smoothstep(0.146, 0.154, sprintDistance));
    vec3 sprintColor = mix(vec3(0.035, 0.105, 0.13), vec3(0.08, 0.53, 0.57), uSprintActive);
    color = mix(color, sprintColor, sprintDisk * uTouchVisible * 0.84);
    color += vec3(0.04, 0.57, 0.65) * sprintRing * uTouchVisible * 0.85;
    float sprintGlyph = 1.0 - smoothstep(0.006, 0.012, abs(hudPosition.x - sprintCenter.x));
    sprintGlyph *= 1.0 - smoothstep(0.027, 0.035, abs(hudPosition.y - sprintCenter.y));
    color += vec3(0.52, 0.94, 0.96) * sprintGlyph * uTouchVisible * 0.78;

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
uniform float uPitch;
uniform float uAspect;
uniform float uMoveX;
uniform float uMoveY;
uniform float uMoveActive;
uniform float uSprintActive;
uniform float uTouchVisible;

float hash(float n) {
    return fract(sin(n) * 43758.5453123);
}

void main() {
    vec2 uv = vUv;
    // Positive pitch looks upward, moving the horizon lower in the frame.
    float horizon = clamp(0.43 - uPitch * 0.35, 0.22, 0.64);
    vec3 skyTop = vec3(0.012, 0.018, 0.042);
    vec3 skyBottom = vec3(0.075, 0.025, 0.095);
    vec3 color = mix(skyBottom, skyTop, smoothstep(0.0, horizon, uv.y));

    // The distant skyline is a panorama: yaw rotates it across the screen too.
    float panoramaU = fract(uv.x + uYaw * 0.15915494309);
    float column = floor(panoramaU * 25.0);
    float height = 0.045 + hash(column + 7.0) * 0.22;
    float building = step(horizon - height, uv.y) * step(uv.y, horizon);
    float windowNoise = step(0.82, hash(floor(panoramaU * 92.0) + floor(uv.y * 54.0) * 13.0));
    vec3 buildingColor = vec3(0.025, 0.035, 0.072) + vec3(0.02, 0.13, 0.17) * windowNoise;
    color = mix(color, buildingColor, building);

    if (uv.y < horizon) {
        float depth = max(horizon - uv.y, 0.012);
        // Cast a view ray, rotate it by camera yaw, then sample the static world grid.
        // Player translation offsets world coordinates; yaw rotates both axes together.
        float cameraX = (uv.x - 0.5) / depth;
        float cameraZ = 1.0 / depth;
        float yawCos = cos(uYaw);
        float yawSin = sin(uYaw);
        float perspectiveX = cameraX * yawCos - cameraZ * yawSin
                           - (uPlayerX - 2.5) * 0.34;
        float perspectiveY = cameraX * yawSin + cameraZ * yawCos
                           - (uPlayerZ - 2.5) * 0.38;
        float gridX = 1.0 - smoothstep(0.018, 0.045, abs(fract(perspectiveX * 0.34) - 0.5));
        float gridY = 1.0 - smoothstep(0.018, 0.05, abs(fract(perspectiveY * 0.055) - 0.5));
        float grid = max(gridX, gridY);
        float fade = 1.0 - smoothstep(0.0, 0.65, depth);
        color = mix(vec3(0.012, 0.018, 0.034), vec3(0.035, 0.20, 0.24), grid * fade * 0.28);
    }

    float horizonGlow = exp(-abs(uv.y - horizon) * 34.0);
    color += vec3(0.12, 0.025, 0.16) * horizonGlow;

    vec2 center = abs(uv - vec2(0.5));
    float crosshair = (1.0 - smoothstep(0.003, 0.006, center.x)) * step(center.y, 0.018)
                    + (1.0 - smoothstep(0.003, 0.006, center.y)) * step(center.x, 0.018);
    color += vec3(0.24, 0.76, 0.78) * crosshair * 0.72;

    // Native touch HUD: a left movement stick and a right sprint button.
    // Coordinates are measured in screen-height units to keep circular controls round.
    float aspect = max(uAspect, 1.0);
    vec2 hudPosition = vec2(uv.x * aspect, uv.y);
    vec2 stickCenter = vec2(aspect * 0.16, 0.21);
    float stickDistance = length(hudPosition - stickCenter);
    float stickDisk = 1.0 - smoothstep(0.112, 0.132, stickDistance);
    float stickRing = smoothstep(0.112, 0.118, stickDistance)
                    * (1.0 - smoothstep(0.128, 0.134, stickDistance));
    color = mix(color, vec3(0.012, 0.050, 0.070), stickDisk * uTouchVisible * 0.58);
    color += vec3(0.025, 0.40, 0.48) * stickRing * uTouchVisible * 0.75;

    vec2 stickKnobCenter = stickCenter + vec2(uMoveX, uMoveY) * 0.066;
    float stickKnob = 1.0 - smoothstep(0.038, 0.048, length(hudPosition - stickKnobCenter));
    color = mix(
        color,
        vec3(0.10, 0.67, 0.75),
        stickKnob * uTouchVisible * mix(0.38, 0.95, uMoveActive)
    );

    vec2 sprintCenter = vec2(aspect * 0.88, 0.21);
    float sprintDistance = length(hudPosition - sprintCenter);
    float sprintDisk = 1.0 - smoothstep(0.118, 0.136, sprintDistance);
    float sprintRing = smoothstep(0.132, 0.140, sprintDistance)
                     * (1.0 - smoothstep(0.146, 0.154, sprintDistance));
    vec3 sprintColor = mix(vec3(0.035, 0.105, 0.13), vec3(0.08, 0.53, 0.57), uSprintActive);
    color = mix(color, sprintColor, sprintDisk * uTouchVisible * 0.84);
    color += vec3(0.04, 0.57, 0.65) * sprintRing * uTouchVisible * 0.85;
    float sprintGlyph = 1.0 - smoothstep(0.006, 0.012, abs(hudPosition.x - sprintCenter.x));
    sprintGlyph *= 1.0 - smoothstep(0.027, 0.035, abs(hudPosition.y - sprintCenter.y));
    color += vec3(0.52, 0.94, 0.96) * sprintGlyph * uTouchVisible * 0.78;

    float scanline = 0.96 + 0.04 * sin(uv.y * 720.0);
    float vignette = 1.0 - smoothstep(0.38, 0.78, length((uv - 0.5) * vec2(1.0, 0.8)));
    float pulse = 0.985 + 0.015 * sin(uTime * 1.7 + uYaw + uPlayerX * 0.03 + uPlayerZ * 0.02);
    color *= scanline * vignette * pulse;
    gl_FragColor = vec4(color, 1.0);
}
)GLSL";
#endif

#if defined(PD_OPENGL_ES) && PD_OPENGL_ES
constexpr const char* kVectorVertexShaderSource = R"GLSL(
attribute vec3 aPosition;
attribute vec3 aColor;
varying vec3 vColor;
void main() {
    gl_Position = vec4(aPosition, 1.0);
    vColor = aColor;
}
)GLSL";

constexpr const char* kVectorFragmentShaderSource = R"GLSL(
precision mediump float;
varying vec3 vColor;
void main() {
    gl_FragColor = vec4(vColor, 1.0);
}
)GLSL";
#else
constexpr const char* kVectorVertexShaderSource = R"GLSL(
#version 120
attribute vec3 aPosition;
attribute vec3 aColor;
varying vec3 vColor;
void main() {
    gl_Position = vec4(aPosition, 1.0);
    vColor = aColor;
}
)GLSL";

constexpr const char* kVectorFragmentShaderSource = R"GLSL(
#version 120
varying vec3 vColor;
void main() {
    gl_FragColor = vec4(vColor, 1.0);
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
    api.BindAttribLocation(program, 1, "aColor");
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
    gl::GLuint vector_program = 0;
    gl::GLuint vector_buffer = 0;
    gl::GLint time_location = -1;
    gl::GLint player_x_location = -1;
    gl::GLint player_z_location = -1;
    gl::GLint yaw_location = -1;
    gl::GLint pitch_location = -1;
    gl::GLint aspect_location = -1;
    gl::GLint move_x_location = -1;
    gl::GLint move_y_location = -1;
    gl::GLint move_active_location = -1;
    gl::GLint sprint_active_location = -1;
    gl::GLint touch_visible_location = -1;
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

    const GLuint vector_vertex_shader = compile_shader(api, gl::kVertexShader, kVectorVertexShaderSource);
    if (vector_vertex_shader == 0) {
        api.DeleteProgram(impl_->program);
        impl_->program = 0;
        return false;
    }
    const GLuint vector_fragment_shader = compile_shader(api, gl::kFragmentShader, kVectorFragmentShaderSource);
    if (vector_fragment_shader == 0) {
        api.DeleteShader(vector_vertex_shader);
        api.DeleteProgram(impl_->program);
        impl_->program = 0;
        return false;
    }
    impl_->vector_program = link_program(api, vector_vertex_shader, vector_fragment_shader);
    api.DeleteShader(vector_vertex_shader);
    api.DeleteShader(vector_fragment_shader);
    if (impl_->vector_program == 0) {
        api.DeleteProgram(impl_->program);
        impl_->program = 0;
        return false;
    }

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

    api.GenBuffers(1, &impl_->vector_buffer);

    impl_->time_location = api.GetUniformLocation(impl_->program, "uTime");
    impl_->player_x_location = api.GetUniformLocation(impl_->program, "uPlayerX");
    impl_->player_z_location = api.GetUniformLocation(impl_->program, "uPlayerZ");
    impl_->yaw_location = api.GetUniformLocation(impl_->program, "uYaw");
    impl_->pitch_location = api.GetUniformLocation(impl_->program, "uPitch");
    impl_->aspect_location = api.GetUniformLocation(impl_->program, "uAspect");
    impl_->move_x_location = api.GetUniformLocation(impl_->program, "uMoveX");
    impl_->move_y_location = api.GetUniformLocation(impl_->program, "uMoveY");
    impl_->move_active_location = api.GetUniformLocation(impl_->program, "uMoveActive");
    impl_->sprint_active_location = api.GetUniformLocation(impl_->program, "uSprintActive");
    impl_->touch_visible_location = api.GetUniformLocation(impl_->program, "uTouchVisible");

    impl_->initialized = true;
    resize(impl_->width, impl_->height);
    SDL_Log("OpenGL renderer ready. Vector/ASCII hybrid test scene loaded.");
    return true;
}

void OpenGLRenderer::resize(int width, int height) noexcept {
    impl_->width = width > 0 ? width : 1;
    impl_->height = height > 0 ? height : 1;
    if (impl_->initialized) {
        impl_->api.Viewport(0, 0, impl_->width, impl_->height);
    }
}

void OpenGLRenderer::render(
    const pd::core::GameState& state,
    float time_seconds,
    const TouchHudState& touch) noexcept {
    if (!impl_->initialized) return;

    auto& api = impl_->api;
    api.Viewport(0, 0, impl_->width, impl_->height);
    api.ClearColor(0.012F, 0.015F, 0.03F, 1.0F);
    api.Clear(gl::kColorBufferBit | gl::kDepthBufferBit);

    api.UseProgram(impl_->program);
    if (impl_->time_location >= 0) api.Uniform1f(impl_->time_location, time_seconds);
    if (impl_->player_x_location >= 0) api.Uniform1f(impl_->player_x_location, state.player.x);
    if (impl_->player_z_location >= 0) api.Uniform1f(impl_->player_z_location, state.player.z);
    if (impl_->yaw_location >= 0) api.Uniform1f(impl_->yaw_location, state.player.yaw_radians);
    if (impl_->pitch_location >= 0) api.Uniform1f(impl_->pitch_location, state.player.pitch_radians);
    if (impl_->aspect_location >= 0) {
        api.Uniform1f(impl_->aspect_location, static_cast<float>(impl_->width) /
            static_cast<float>(impl_->height));
    }
    if (impl_->move_x_location >= 0) api.Uniform1f(impl_->move_x_location, touch.move_x);
    if (impl_->move_y_location >= 0) api.Uniform1f(impl_->move_y_location, touch.move_y);
    if (impl_->move_active_location >= 0) {
        api.Uniform1f(impl_->move_active_location, touch.move_active ? 1.0F : 0.0F);
    }
    if (impl_->sprint_active_location >= 0) {
        api.Uniform1f(impl_->sprint_active_location, touch.sprint_active ? 1.0F : 0.0F);
    }
    if (impl_->touch_visible_location >= 0) {
        api.Uniform1f(impl_->touch_visible_location, touch.visible ? 1.0F : 0.0F);
    }

    api.BindBuffer(gl::kArrayBuffer, impl_->vertex_buffer);
    api.EnableVertexAttribArray(0);
    api.VertexAttribPointer(0, 2, gl::kFloat, 0, 0, nullptr);
    api.DrawArrays(gl::kTriangles, 0, 3);
    api.DisableVertexAttribArray(0);

    // The vector pass uses Nano-punk-style camera-space projection, then draws
    // perspective-correct wire geometry and bitmap ASCII glyphs as native GL_LINES.
    const auto vector_vertices = detail::build_vector_scene(
        state, impl_->width, impl_->height, touch);
    if (!vector_vertices.empty() && impl_->vector_program != 0 && impl_->vector_buffer != 0) {
        api.Enable(gl::kDepthTest);
        api.DepthFunc(gl::kLequal);
        api.DepthMask(gl::kTrue);
        api.UseProgram(impl_->vector_program);
        api.BindBuffer(gl::kArrayBuffer, impl_->vector_buffer);
        api.BufferData(
            gl::kArrayBuffer,
            static_cast<gl::GLsizeiptr>(vector_vertices.size() * sizeof(detail::VectorVertex)),
            vector_vertices.data(),
            gl::kDynamicDraw
        );
        api.EnableVertexAttribArray(0);
        api.EnableVertexAttribArray(1);
        api.VertexAttribPointer(
            0, 3, gl::kFloat, gl::kFalse, static_cast<gl::GLsizei>(sizeof(detail::VectorVertex)),
            reinterpret_cast<const void*>(offsetof(detail::VectorVertex, x)));
        api.VertexAttribPointer(
            1, 3, gl::kFloat, gl::kFalse, static_cast<gl::GLsizei>(sizeof(detail::VectorVertex)),
            reinterpret_cast<const void*>(offsetof(detail::VectorVertex, r)));
        api.DrawArrays(gl::kLines, 0, static_cast<gl::GLsizei>(vector_vertices.size()));
        api.DisableVertexAttribArray(0);
        api.DisableVertexAttribArray(1);
        api.Disable(gl::kDepthTest);
    }
}

void OpenGLRenderer::shutdown() noexcept {
    if (!impl_ || !impl_->initialized) return;
    auto& api = impl_->api;
    if (impl_->vertex_buffer != 0) api.DeleteBuffers(1, &impl_->vertex_buffer);
    if (impl_->vector_buffer != 0) api.DeleteBuffers(1, &impl_->vector_buffer);
    if (impl_->program != 0) api.DeleteProgram(impl_->program);
    if (impl_->vector_program != 0) api.DeleteProgram(impl_->vector_program);
    impl_->vertex_buffer = 0;
    impl_->vector_buffer = 0;
    impl_->program = 0;
    impl_->vector_program = 0;
    impl_->initialized = false;
}

bool OpenGLRenderer::is_initialized() const noexcept {
    return impl_ && impl_->initialized;
}
} // namespace pd::renderer
