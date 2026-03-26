#include <iostream>

#include "game/Start.h"

int main()
{
    Start* start = new Start();
    start->Init();

    while (true)
    {
        std::cout << "i" << std::endl;
    }

    delete start;
    return 0;
}
