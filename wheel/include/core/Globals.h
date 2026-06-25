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
