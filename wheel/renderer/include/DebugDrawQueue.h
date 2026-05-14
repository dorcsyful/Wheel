#pragma once
#ifdef DEBUG_BUILD
#include <variant>
#include <vector>
#include "math/Vector2.h"

namespace Wheel::Engine::Debug {

enum class DrawType { Line, Point, Arrow };
struct Drawer { };
struct DrawLine : public Drawer {  DrawLine(const Math::Vector2& a_From, const Math::Vector2& a_To, DebugColor a_Color) : from(a_From), to(a_To), color(a_Color) {} Math::Vector2 from, to;  DebugColor color; float thickness = 1.5f; };
struct DrawPoint : public Drawer { DrawPoint(const Math::Vector2& a_Position, DebugColor a_Color) : position(a_Position), color(a_Color) {} Math::Vector2 position;  DebugColor color; float radius = 4.0f; };
struct DrawArrow : public Drawer { DrawArrow(const Math::Vector2& a_From, const Math::Vector2& a_To, DebugColor a_Color) : from(a_From), to(a_To), color(a_Color) {} Math::Vector2 from, to;  DebugColor color; float thickness = 1.5f; };


class DebugDrawQueue
{
public:
    DebugDrawQueue(const DebugDrawQueue&) = delete;
    void operator=(const DebugDrawQueue&) = delete;
    static DebugDrawQueue& Get() { static DebugDrawQueue instance; return instance; }

    void Submit(Drawer dt) { m_Drawer.push_back(std::move(dt)); }
    void Clear() { return m_Drawer.clear(); }
    std::vector<Drawer> GetDrawers() { return m_Drawer; }

private:
    DebugDrawQueue() = default;
    std::vector<Drawer> m_Drawer;
};

} // namespace Wheel::Engine::Debug
#endif // DEBUG_BUILD