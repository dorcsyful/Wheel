#pragma once
#include <unordered_map>

#include "glad.h"

namespace Wheel
{
    namespace Renderer
    {
        struct TypeInfo {
            TypeInfo(GLint a_Components, GLenum a_BaseType, void (*a_SetUniform)(GLint, const void*))
                : components(a_Components), baseType(a_BaseType), setUniform(a_SetUniform)
            {
            }
            GLint   components;   // 2 for vec2, 4 for mat2, etc.
            GLenum  baseType;     // GL_FLOAT for attribs
            void  (*setUniform)(GLint loc, const void* data);
        };
    }
    static std::unordered_map<GLenum, Renderer::TypeInfo> TypeInfoMap = {
        { GL_FLOAT, { 1, GL_FLOAT, [](GLint loc, const void* data) { glUniform1fv(loc, 1, static_cast<const GLfloat*>(data)); } } },
        { GL_FLOAT_VEC2, { 2, GL_FLOAT, [](GLint loc, const void* data) { glUniform2fv(loc, 1, static_cast<const GLfloat*>(data)); } } },
        { GL_FLOAT_VEC3, { 3, GL_FLOAT, [](GLint loc, const void* data) { glUniform3fv(loc, 1, static_cast<const GLfloat*>(data)); } } },
        { GL_FLOAT_VEC4, { 4, GL_FLOAT, [](GLint loc, const void* data) { glUniform4fv(loc, 1, static_cast<const GLfloat*>(data)); } } },
        { GL_FLOAT_MAT2, { 4, GL_FLOAT, [](GLint loc, const void* data) { glUniformMatrix2fv(loc, 1, GL_FALSE, static_cast<const GLfloat*>(data)); } } },
        { GL_FLOAT_MAT3, { 9, GL_FLOAT, [](GLint loc, const void* data) { glUniformMatrix3fv(loc, 1, GL_FALSE, static_cast<const GLfloat*>(data)); } } },
        { GL_FLOAT_MAT4, { 16, GL_FLOAT, [](GLint loc, const void* data) { glUniformMatrix4fv(loc, 1, GL_FALSE, static_cast<const GLfloat*>(data)); } } },
        // Add more types as needed
    };
}
