#pragma once
#include <string>

#include "glad.h"

namespace Wheel
{
    namespace Renderer
    {
        class Shader
        {
        public:
            Shader(const std::string& a_VertexPath, const std::string& a_FragmentPath);
            ~Shader() = default;

            void use() const  {  glUseProgram(m_ID);  }
            void setBool(const std::string &name, bool value) const { glUniform1i(glGetUniformLocation(m_ID, name.c_str()), (int)value); }
            void setInt(const std::string &name, int value) const { glUniform1i(glGetUniformLocation(m_ID, name.c_str()), value); }
            void setFloat(const std::string &name, float value) const { glUniform1f(glGetUniformLocation(m_ID, name.c_str()), value); }
            [[nodiscard]] unsigned int GetID() const
            {
                return m_ID;
            }
            [[nodiscard]] std::string GetName() const
            {
                return m_Name;
            }

        private:
            void checkCompileErrors(unsigned int shader, const std::string& type);
            std::string m_Name;
            unsigned int m_ID;
        };
    }
}
