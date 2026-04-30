#pragma once

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
        struct AnyMouseButtonEvent
    }
}

