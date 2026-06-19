#pragma once
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string>
#include <unordered_map>

#include "DebugDescriptor.h"
#include "Shader.h"
#include "math/Vector2.h"
#include "math/Vector3.h"
#include "math/Vector4.h"
#include "math/Matrix2x2.h"
#include "math/Matrix4x4.h"

namespace Wheel
{
    namespace Renderer
    {
        /**
         * @brief The uniform's value type
         */
        enum class UniformType { Float, Int, Vec2, Vec3, Vec4, Mat2, Mat4 };

        /**
         * @brief Maps a C++ type to its UniformType
         */
        template<typename T> struct UniformTypeOf;
        template<> struct UniformTypeOf<float>           { static constexpr UniformType value = UniformType::Float; };
        template<> struct UniformTypeOf<int>             { static constexpr UniformType value = UniformType::Int;   };
        template<> struct UniformTypeOf<Math::Vector2>   { static constexpr UniformType value = UniformType::Vec2;  };
        template<> struct UniformTypeOf<Math::Vector3>   { static constexpr UniformType value = UniformType::Vec3;  };
        template<> struct UniformTypeOf<Math::Vector4>   { static constexpr UniformType value = UniformType::Vec4;  };
        template<> struct UniformTypeOf<Math::Matrix2x2> { static constexpr UniformType value = UniformType::Mat2;  };
        template<> struct UniformTypeOf<Math::Matrix4x4> { static constexpr UniformType value = UniformType::Mat4;  };

        /**
         * @brief One named uniform value held by a Material.
         */
        struct MaterialProperty
        {
            UniformType type;
            alignas(16) std::byte bytes[64];

            const void* Data() const { return bytes; }
        };

        /**
         * @brief Holds the shader and texture to draw an object. Referenced by RenderObject components.
         */
        class Material
        {
        public:
            std::string ShaderName = "base";
            std::string TextureName = "square.png";

            //id is this material's slot in the renderer shaderId/textureId are the resolved lookups cached
            // here so the render loop never hashes a name per object.
            uint32_t id        = 0;
            uint32_t shaderId  = 0;
            uint32_t textureId = 0;

            /**
             * @brief Sets (or overwrites) a uniform value by name.
             * @tparam T UniformTypeOf type 
             */
            template<typename T>
            void Set(const std::string& a_Name, const T& a_Value)
            {
                static_assert(sizeof(T) <= sizeof(MaterialProperty::bytes),
                              "Uniform value is larger than MaterialProperty storage.");
                MaterialProperty& prop = m_Properties[a_Name];
                prop.type = UniformTypeOf<T>::value;
                std::memcpy(prop.bytes, &a_Value, sizeof(T));
            }

            /**
             * @brief Reads a uniform value back by name.
             * @return The stored value
             */
            template<typename T>
            T Get(const std::string& a_Name) const
            {
                T out{};
                auto it = m_Properties.find(a_Name);
                if (it != m_Properties.end())
                    std::memcpy(&out, it->second.bytes, sizeof(T));
                return out;
            }

            bool Has(const std::string& a_Name) const
            {
                return m_Properties.find(a_Name) != m_Properties.end();
            }


            const std::unordered_map<std::string, MaterialProperty>& GetProperties() const
            {
                return m_Properties;
            }

        private:


            std::unordered_map<std::string, MaterialProperty> m_Properties;
        };
    }
}