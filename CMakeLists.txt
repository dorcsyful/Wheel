cmake_minimum_required(VERSION 3.20)
project(Wheel)

# 1. Collect Core Files
# Using your original "globbing" method to keep it hands-off
file(GLOB_RECURSE CORE_SOURCES "src/*.cpp" "src/*.h")
add_library(engine_core STATIC ${CORE_SOURCES})

# 2. Collect User Files (The "Magic" Part)
file(GLOB_RECURSE USER_SOURCES "game/*.cpp" "game/*.h")
add_library(user_code STATIC ${USER_SOURCES})
target_link_libraries(user_code engine_core)

# 3. Target: Raspberry Pi (The Main Executable)
# This stays exactly as you had it for the Pi build
add_executable(pi_engine main.cpp)
target_link_libraries(pi_engine user_code engine_core)

# 4. Target: Google Test (Windows Host Only)
# We swap BUILD_TESTING for WIN32 to ensure the Pi doesn't touch GTest
if(WIN32)
    message(STATUS "Windows detected: Setting up Google Test...")

    # Point to the folder you downloaded from GitHub
    # EXCLUDE_FROM_ALL prevents GTest's own internal tests from cluttering your build
    add_subdirectory("Google_tests")

else()
    message(STATUS "Non-Windows environment: Skipping Google Test configuration.")
endif()