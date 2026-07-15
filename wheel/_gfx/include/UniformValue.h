#pragma once
#include <cstddef>

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
         * @brief One named uniform value. Storage is fixed and large enough for a mat4.
         */
        struct MaterialProperty
        {
            UniformType type;
            alignas(16) std::byte bytes[64];

            const void* Data() const { return bytes; }
        };
    }
}
