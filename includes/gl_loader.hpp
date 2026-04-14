#pragma once

#include <GL/gl.h>
#include <GL/glext.h>

// バッファ系
extern PFNGLGENBUFFERSPROC              glGenBuffers;
extern PFNGLBINDBUFFERPROC              glBindBuffer;
extern PFNGLBUFFERDATAPROC              glBufferData;
extern PFNGLDELETEBUFFERSPROC           glDeleteBuffers;

// VAO系
extern PFNGLGENVERTEXARRAYSPROC         glGenVertexArrays;
extern PFNGLBINDVERTEXARRAYPROC         glBindVertexArray;
extern PFNGLDELETEVERTEXARRAYSPROC      glDeleteVertexArrays;

// 頂点属性
extern PFNGLVERTEXATTRIBPOINTERPROC     glVertexAttribPointer;
extern PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;

// シェーダー系
extern PFNGLCREATESHADERPROC            glCreateShader;
extern PFNGLSHADERSOURCEPROC            glShaderSource;
extern PFNGLCOMPILESHADERPROC           glCompileShader;
extern PFNGLGETSHADERIVPROC             glGetShaderiv;
extern PFNGLGETSHADERINFOLOGPROC        glGetShaderInfoLog;
extern PFNGLDELETESHADERPROC            glDeleteShader;

// プログラム系
extern PFNGLCREATEPROGRAMPROC           glCreateProgram;
extern PFNGLATTACHSHADERPROC            glAttachShader;
extern PFNGLLINKPROGRAMPROC             glLinkProgram;
extern PFNGLUSEPROGRAMPROC              glUseProgram;
extern PFNGLGETPROGRAMIVPROC            glGetProgramiv;
extern PFNGLGETPROGRAMINFOLOGPROC       glGetProgramInfoLog;
extern PFNGLDELETEPROGRAMPROC           glDeleteProgram;

// Uniform系
extern PFNGLGETUNIFORMLOCATIONPROC      glGetUniformLocation;
extern PFNGLUNIFORMMATRIX4FVPROC        glUniformMatrix4fv;
extern PFNGLUNIFORM3FPROC               glUniform3f;
extern PFNGLUNIFORM1IPROC               glUniform1i;
extern PFNGLUNIFORM1FPROC               glUniform1f;

// テクスチャ系（glActiveTextureはOpenGL 1.3で libGL に直接含まれるためロード不要）

bool loadGLFunctions();
