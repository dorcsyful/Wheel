#include <glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <thread>
#include <chrono>

#include "Start.h"

int main() {

    std::unique_ptr<Start> gStart = std::make_unique<Start>();
    gStart->Init();
    gStart->Update();
    return 0;
}
