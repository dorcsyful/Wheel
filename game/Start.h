//
//This is the starting file. Register any custom systems you'd like to use here
#pragma once

#include <iostream>

#include "../wheel/include/core/Scene.h"
Wheel::Engine::Scene* g_Scene;

class Start
{
    public:
    Start()
    {
        g_Scene = new Wheel::Engine::Scene();
    }
    ~Start()
    {
        delete g_Scene;
    }

    void Init()
    {
        //Register your systems here
    }
    void Update();
};


