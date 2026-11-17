#include "gl_api.hpp"
#include <SDL3/SDL.h>

namespace pd::renderer::gl {
namespace {
template <typename Function>
bool load_function(Function& function, const char* name) {
    function = reinterpret_cast<Function>(SDL_GL_GetProcAddress(name));
    if (function == nullptr) {
        SDL_LogError(SDL_LOG_CATEGORY_RENDER, "Missing OpenGL entry point: %s", name);
        return false;
    }
    return true;
}
}

bool Api::load() {
    bool ok = true;
#define PD_LOAD_GL(name) ok = load_function(name, "gl" #name) && ok
    PD_LOAD_GL(Viewport);
    PD_LOAD_GL(ClearColor);
    PD_LOAD_GL(Clear);
    PD_LOAD_GL(GenBuffers);
    PD_LOAD_GL(BindBuffer);
    PD_LOAD_GL(BufferData);
    PD_LOAD_GL(DeleteBuffers);
    PD_LOAD_GL(CreateShader);
    PD_LOAD_GL(ShaderSource);
    PD_LOAD_GL(CompileShader);
    PD_LOAD_GL(GetShaderiv);
    PD_LOAD_GL(GetShaderInfoLog);
    PD_LOAD_GL(DeleteShader);
    PD_LOAD_GL(CreateProgram);
    PD_LOAD_GL(AttachShader);
    PD_LOAD_GL(BindAttribLocation);
    PD_LOAD_GL(LinkProgram);
    PD_LOAD_GL(GetProgramiv);
    PD_LOAD_GL(GetProgramInfoLog);
    PD_LOAD_GL(DeleteProgram);
    PD_LOAD_GL(UseProgram);
    PD_LOAD_GL(GetUniformLocation);
    PD_LOAD_GL(Uniform1f);
    PD_LOAD_GL(EnableVertexAttribArray);
    PD_LOAD_GL(DisableVertexAttribArray);
    PD_LOAD_GL(VertexAttribPointer);
    PD_LOAD_GL(DrawArrays);
#undef PD_LOAD_GL
    return ok;
}
} // namespace pd::renderer::gl
