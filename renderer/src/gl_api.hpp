#pragma once
#include <cstddef>

#if defined(_WIN32)
#define PD_GL_APIENTRY __stdcall
#else
#define PD_GL_APIENTRY
#endif

namespace pd::renderer::gl {
using GLenum = unsigned int;
using GLuint = unsigned int;
using GLint = int;
using GLsizei = int;
using GLboolean = unsigned char;
using GLfloat = float;
using GLchar = char;
using GLsizeiptr = std::ptrdiff_t;

constexpr GLenum kColorBufferBit = 0x00004000;
constexpr GLenum kArrayBuffer = 0x8892;
constexpr GLenum kStaticDraw = 0x88E4;
constexpr GLenum kFloat = 0x1406;
constexpr GLenum kTriangles = 0x0004;
constexpr GLenum kVertexShader = 0x8B31;
constexpr GLenum kFragmentShader = 0x8B30;
constexpr GLenum kCompileStatus = 0x8B81;
constexpr GLenum kLinkStatus = 0x8B82;
constexpr GLenum kInfoLogLength = 0x8B84;

struct Api {
    void (PD_GL_APIENTRY *Viewport)(GLint, GLint, GLsizei, GLsizei) = nullptr;
    void (PD_GL_APIENTRY *ClearColor)(GLfloat, GLfloat, GLfloat, GLfloat) = nullptr;
    void (PD_GL_APIENTRY *Clear)(GLenum) = nullptr;
    void (PD_GL_APIENTRY *GenBuffers)(GLsizei, GLuint*) = nullptr;
    void (PD_GL_APIENTRY *BindBuffer)(GLenum, GLuint) = nullptr;
    void (PD_GL_APIENTRY *BufferData)(GLenum, GLsizeiptr, const void*, GLenum) = nullptr;
    void (PD_GL_APIENTRY *DeleteBuffers)(GLsizei, const GLuint*) = nullptr;
    GLuint (PD_GL_APIENTRY *CreateShader)(GLenum) = nullptr;
    void (PD_GL_APIENTRY *ShaderSource)(GLuint, GLsizei, const GLchar* const*, const GLint*) = nullptr;
    void (PD_GL_APIENTRY *CompileShader)(GLuint) = nullptr;
    void (PD_GL_APIENTRY *GetShaderiv)(GLuint, GLenum, GLint*) = nullptr;
    void (PD_GL_APIENTRY *GetShaderInfoLog)(GLuint, GLsizei, GLsizei*, GLchar*) = nullptr;
    void (PD_GL_APIENTRY *DeleteShader)(GLuint) = nullptr;
    GLuint (PD_GL_APIENTRY *CreateProgram)() = nullptr;
    void (PD_GL_APIENTRY *AttachShader)(GLuint, GLuint) = nullptr;
    void (PD_GL_APIENTRY *BindAttribLocation)(GLuint, GLuint, const GLchar*) = nullptr;
    void (PD_GL_APIENTRY *LinkProgram)(GLuint) = nullptr;
    void (PD_GL_APIENTRY *GetProgramiv)(GLuint, GLenum, GLint*) = nullptr;
    void (PD_GL_APIENTRY *GetProgramInfoLog)(GLuint, GLsizei, GLsizei*, GLchar*) = nullptr;
    void (PD_GL_APIENTRY *DeleteProgram)(GLuint) = nullptr;
    void (PD_GL_APIENTRY *UseProgram)(GLuint) = nullptr;
    GLint (PD_GL_APIENTRY *GetUniformLocation)(GLuint, const GLchar*) = nullptr;
    void (PD_GL_APIENTRY *Uniform1f)(GLint, GLfloat) = nullptr;
    void (PD_GL_APIENTRY *EnableVertexAttribArray)(GLuint) = nullptr;
    void (PD_GL_APIENTRY *DisableVertexAttribArray)(GLuint) = nullptr;
    void (PD_GL_APIENTRY *VertexAttribPointer)(GLuint, GLint, GLenum, GLboolean, GLsizei, const void*) = nullptr;
    void (PD_GL_APIENTRY *DrawArrays)(GLenum, GLint, GLsizei) = nullptr;
    [[nodiscard]] bool load();
};
} // namespace pd::renderer::gl

#undef PD_GL_APIENTRY
