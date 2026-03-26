#pragma once
#define  MAX_ENTITIES 1000
#define  MAX_COMPONENT_TYPES 16
#define  MAX_SYSTEMS 16
template<typename T>
T random(T range_from, T range_to) {
    std::random_device                  rand_dev;
    std::mt19937                        generator(rand_dev());
    std::uniform_int_distribution<T>    distr(range_from, range_to);
    return distr(generator);
}