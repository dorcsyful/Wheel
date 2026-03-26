#pragma once
#include "Rendering.h"

#include <iostream>
#include <ostream>

void Wheel::Systems::Rendering::GetComponentPool(Engine::IComponentPool* a_Pool)
{

}

void Wheel::Systems::Rendering::Update(float deltaTime)
{
    std::cout << "Wheel::Systems::Rendering::Update" << std::endl;
}
