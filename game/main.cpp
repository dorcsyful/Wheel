#include <glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <thread>
#include <chrono>

#include "Start.h"


int main() {

    std::unique_ptr<Start> start = std::make_unique<Start>();
    start->Init();
    start->Update();
    return 0;
}
