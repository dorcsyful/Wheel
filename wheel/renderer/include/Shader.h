#pragma once
#include <string>
#include <unordered_map>

#include "glad.h"

namespace Wheel
{
    namespace Renderer
    {
        struct VarInfo {
            VarInfo(const std::string& a_Name, GLint a_Location, int a_TypeInfo)
                : name(a_Name), location(a_Location), typeInfo(a_TypeInfo) {}
            std::string name;
            GLint location;
            int typeInfo;
        };

        class Shader
        {
        public:
            Shader(const std::string& a_VertexPath, const std::string& a_FragmentPath);
            ~Shader() {
                glDeleteProgram(m_ID);
            }

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
            void CacheLocations();
            void AddPerBind(const std::string& a_Name);
            void AddPerObject(const std::string& a_Name);
            std::vector<VarInfo>& GetPerBinds() { return m_PerBind; }
            std::vector<VarInfo>& GetPerObjects() { return m_PerObject; }
            std::vector<VarInfo>& GetAttributes() { return m_Attributes; }
        private:
            void checkCompileErrors(unsigned int shader, const std::string& type);
            std::vector<VarInfo> m_Attributes;
            std::vector<VarInfo> m_PerBind;
            std::vector<VarInfo> m_PerObject;

            std::string m_Name;
            unsigned int m_ID;
        };
    }
}
