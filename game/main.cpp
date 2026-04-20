#include <glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <thread>
#include <chrono>

#include "Start.h"

std::shared_ptr<Start> gStart;
int main() {

    gStart = std::make_unique<Start>();
    gStart->Init();
    gStart->Update();
    return 0;
}
