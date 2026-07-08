#pragma once

#ifdef DEBUG_BUILD

#include <functional>
#include <string>
#include <vector>
#include <cstdint>

#include "math/Vector2.h"
#include "math/Vector4.h"

namespace Wheel { namespace Rendering { enum class CameraMode : int; } }
namespace Wheel { namespace Physics { enum class Rigidbody2DType : int; } }

enum class FieldType { INT, UINT, FLOAT, BOOL, STRING, VEC2, VEC3, VEC4, SIZE_T, CAMERA_MODE, RIGIDBODY2D_TYPE };

struct FieldDescriptor {
    const char*             name;
    FieldType               type;
    size_t                  offset;   // offsetof() into the struct
};

// One of these per reflected type
struct TypeDescriptor {
    const char*                  name;
    const FieldDescriptor* fields;
    size_t                 fieldCount;

    TypeDescriptor(const char* n,const FieldDescriptor* f, size_t c)
    : name(n), fields(f), fieldCount(c) {}

};
template<typename C, typename T>
size_t offsetFromMemberPtr(T C::*mp) {
    C dummy{};
    return reinterpret_cast<size_t>(&(dummy.*mp)) -
           reinterpret_cast<size_t>(&dummy);
}

// One overload per supported type
template<typename C>
FieldDescriptor makeField(const char* name, float C::*mp) {
    return { name, FieldType::FLOAT, offsetFromMemberPtr(mp) };
}

template<typename C>
FieldDescriptor makeField(const char* name, int C::*mp) {
    return { name, FieldType::INT, offsetFromMemberPtr(mp) };
}

template<typename C>
FieldDescriptor makeField(const char* name, Wheel::Physics::Rigidbody2DType C::*mp) {
    return { name, FieldType::RIGIDBODY2D_TYPE, offsetFromMemberPtr(mp) };
}

template<typename C>
FieldDescriptor makeField(const char* name, bool C::*mp) {
    return { name, FieldType::BOOL, offsetFromMemberPtr(mp) };
}

template<typename C>
FieldDescriptor makeField(const char* name, uint32_t C::*mp) {
    return { name, FieldType::UINT, offsetFromMemberPtr(mp) };
}

template<typename C>
FieldDescriptor makeField(const char* name, std::string C::*mp) {
    return { name, FieldType::STRING, offsetFromMemberPtr(mp) };
}

template<typename C>
FieldDescriptor makeField(const char* name, Wheel::Math::Vector2 C::*mp) {
    return { name, FieldType::VEC2, offsetFromMemberPtr(mp) };
}
template<typename C>
FieldDescriptor makeField(const char* name, Wheel::Math::Vector4 C::*mp) {
    return { name, FieldType::VEC4, offsetFromMemberPtr(mp) };
}

template<typename C>
FieldDescriptor makeField(const char* name, size_t C::*mp) {
    return { name, FieldType::SIZE_T, offsetFromMemberPtr(mp) };
}

template<typename C>
FieldDescriptor makeField(const char* name, Wheel::Rendering::CameraMode C::*mp) {
    return { name, FieldType::CAMERA_MODE, offsetFromMemberPtr(mp) };
}
/**
 * @brief Use these macros to add reflection to your components. This is used by the Debugger to display component details, but you can also use it for serialization or whatever else you want.
 * @param ComponentType The name of the component class you're adding reflection to. This should be the same as the name of the struct after REFLECT_END
 * @param displayName The name that will show up in the Debugger. Can be different from ComponentType if you want, but usually it's just the same thing without namespaces
 *
 * Example usage:
 *
 * class Transform2D
 * {
 * public:
 *     std::string name;
 *     Wheel::Math::Vector2 position;
 *     Wheel::Math::Vector2 scale;
 *     float rotation = 0.0f;
 *
 *     REFLECT_BEGIN(Transform2D)
 *     FIELD(name)
 *     FIELD(position)
 *     FIELD(scale)
 *     FIELD(rotation)
 *     REFLECT_END(Transform2D, "Transform 2D")
 * };
 */
#define  REFLECT_BEGIN(ComponentType) \
static const TypeDescriptor& descriptor() { \
typedef ComponentType _C; \
static const FieldDescriptor fields[] = {

#define FIELD(memberName) \
makeField(#memberName, &_C::memberName),

#define REFLECT_END(ComponentType, displayName) \
}; \
static TypeDescriptor d = { \
displayName, fields, \
sizeof(fields)/sizeof(fields[0]) \
}; \
return d; \
}

#else // !DEBUG_BUILD — reflection compiles to nothing in release builds

#define REFLECT_BEGIN(ComponentType)
#define FIELD(memberName)
#define REFLECT_END(ComponentType, displayName)

#endif // DEBUG_BUILD
