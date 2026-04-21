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
    }
}

