#pragma once
#include "helpers/Collision2DManifold.h"
//basic events
namespace Wheel
{
    namespace Events
    {
        struct WindowResizeEvent {
            WindowResizeEvent(int a_Width, int a_Height) : width(a_Width), height(a_Height) {}
            int width = -1;
            int height = -1;
        };
        //TODO: convert to own codes
        /**
         * Uses glfw keycodes. See https://www.glfw.org/docs/latest/group__keys.html for list
         */
        struct KeyPressEvent
        {
            KeyPressEvent(int a_Key) : key(a_Key) {}
            int key = -1;
        };
        struct KeyReleaseEvent
        {
            KeyReleaseEvent(int a_Key) : key(a_Key) {}
            int key = -1;
        };

        struct LeftMouseButtonPressEvent
        {
            LeftMouseButtonPressEvent(double a_X, double a_Y) : x(a_X), y(a_Y) {}
            double x;
            double y;
        };
        struct RightMouseButtonPressEvent
        {
            RightMouseButtonPressEvent(double a_X, double a_Y) : x(a_X), y(a_Y) {}
            double x;
            double y;
        };
        struct LeftMouseButtonReleaseEvent
        {
            LeftMouseButtonReleaseEvent(double a_X, double a_Y) : x(a_X), y(a_Y) {}
            double x;
            double y;
        };
        struct RightMouseButtonReleaseEvent
        {
            RightMouseButtonReleaseEvent(double a_X, double a_Y) : x(a_X), y(a_Y) {}
            double x;
            double y;
        };
        struct MouseMoveEvent
        {
            MouseMoveEvent(double a_X, double a_Y) : x(a_X), y(a_Y) {}
            double x;
            double y;
        };
        struct MouseScrollEvent
        {
            MouseScrollEvent(double a_X, double a_Y) : xoffset(a_X), yoffset(a_Y) {}
            double xoffset;
            double yoffset;
        };
        struct CursorEnterEvent
        {
            CursorEnterEvent(int a_X, int a_Y) : x(a_X), y(a_Y) {}
            int x;
            int y;
        };
        struct CursorLeaveEvent
        {
        };
        struct CameraSwitchEvent
        {

        };

        struct CollisionEnterEvent
        {
            CollisionEnterEvent(const Engine::Collision::Collision2DManifold& a_Manifold) : manifold(a_Manifold) {}
            const Engine::Collision::Collision2DManifold manifold;
        };
        struct CollisionExitEvent
        {
            CollisionExitEvent(uint32_t a_Collider1, uint32_t a_Collider2) : collider1(a_Collider1), collider2(a_Collider2) {}
            uint32_t collider1;
            uint32_t collider2;
        };
        //used exclusively by the collision dispatcher, carries ALL collision manifolds
        struct CollisionResultsFinished
        {
            CollisionResultsFinished(const std::vector<Engine::Collision::Collision2DManifold>* a_ManifoldPtr) : ManifoldPtr(a_ManifoldPtr) {}
            const std::vector<Engine::Collision::Collision2DManifold>* ManifoldPtr = nullptr;
        };

        struct RunSimulation
        {
          RunSimulation(bool a_Enable) : enable(a_Enable) {};
          bool enable;
        };
        //The collision dispatcher needs to know when to unsubscribe
        struct EntityDestroyed
        {
            uint32_t entityId;
            EntityDestroyed(uint32_t a_EntityId) : entityId(a_EntityId) {}
        };
    }
}

