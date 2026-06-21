#pragma once
#define MAX_ENTITIES 1000
#define MAX_COMPONENT_TYPES 16
#define MAX_SYSTEMS 16
#define MAX_CONSTRAINT_ITERATION 10
#define PI 3.14159265358979323846f
#define NO_VALUE UINT32_MAX
constexpr float RAD_TO_DEG = 180.0f / PI;
constexpr float DEG_TO_RAD = PI / 180.0f;

/**
 * @brief Wheel Units to actual pixels. Relevant in 2D
 */
#define PIXELS_PER_UNIT 100
#include <random>
#include <type_traits>

namespace Wheel {
    template<typename T>
    T Random(T range_from, T range_to) {
        static std::random_device rand_dev;
        static std::mt19937 generator{rand_dev()};
        if constexpr (std::is_floating_point_v<T>) {
            std::uniform_real_distribution<T> distr(range_from, range_to);
            return distr(generator);
        } else {
            static_assert(std::is_integral_v<T>, "Wheel::Random<T> requires an arithmetic type.");
            using Wide = std::conditional_t<std::is_signed_v<T>, long long, unsigned long long>;
            std::uniform_int_distribution<Wide> distr(static_cast<Wide>(range_from),
                                                      static_cast<Wide>(range_to));
            return static_cast<T>(distr(generator));
        }
    }
}
