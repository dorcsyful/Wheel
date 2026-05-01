#pragma once
#define MAX_ENTITIES 1000
#define MAX_COMPONENT_TYPES 16
#define MAX_SYSTEMS 16

/**
 * @brief The location of all assets relative to the EXECUTABLE. You can change this if you want to put your assets in a different location
 */
#define ASSETS_LOCATION "../assets/"
/**
 * @brief Wheel Units to actual pixels. Relevant in 2D
 */
#define PIXELS_PER_UNIT 100
#include <random>

template<typename T>
T random(T range_from, T range_to) {
    std::random_device                  rand_dev;
    std::mt19937                        generator(rand_dev());
    std::uniform_int_distribution<T>    distr(range_from, range_to);
    return distr(generator);
}
