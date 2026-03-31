//
//This is the starting file. Register any custom systems you'd like to use here
#pragma once

#include <iostream>

#include <core/Scene.h>

#include "Renderer.h"

//This is the class that communicates with the engine. Override any part of this to fit your needs
class Start
{
    public:
    Start()
    {
    }
    ~Start()
    {
    }

    void RegisterComponents()
    {
        //Register your components here
    }

    void RegisterSystems()
    {
        //Register your systems here
    }

    void CreateEntities()
    {
        //Create your entities here
    }

    void Init()
    {
        m_Renderer = std::make_unique<Wheel::Renderer::Renderer>();
        m_Renderer->Init(1280, 720, "Wheel Engine");
    }
    void Update();

    private:
    std::unique_ptr<Wheel::Renderer::Renderer> m_Renderer;
};


