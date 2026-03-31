cmake_minimum_required(VERSION 3.20)
project(Wheel)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- 1. DEPENDENCIES (GLFW & GLAD) ---

# Disable GLFW junk (we don't need their docs or tests)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)

# Add GLFW from your submodules folder
add_subdirectory(dependencies/glfw)

# Create a library for GLAD (since it's just raw source files)
add_library(glad STATIC dependencies/glad/glad.c)
target_include_directories(glad PUBLIC dependencies/glad)

# --- 2. THE ENGINE (Wheel) ---

file(GLOB_RECURSE ENGINE_SOURCES "wheel/*.cpp" "wheel/*.h")
add_library(wheel_engine STATIC ${ENGINE_SOURCES})

# Tell the engine where to find GLFW and GLAD headers
target_link_libraries(wheel_engine PUBLIC glad glfw)
target_include_directories(wheel_engine PUBLIC wheel/include)
target_include_directories(wheel_engine PUBLIC wheel/renderer/include)

# --- 3. PLATFORM SPECIFIC LINKING ---

if(WIN32)
    # Windows uses standard OpenGL
    target_link_libraries(wheel_engine PUBLIC opengl32)
    message(STATUS "Configuring for Windows (Desktop OpenGL)")

    # Setup Google Test for Windows
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Google_tests/CMakeLists.txt")
        add_subdirectory("Google_tests")
    endif()
else()
    # Raspberry Pi 3 uses OpenGL ES 2.0/3.0
    # 'm' is math library, 'dl' is for loading drivers
    target_link_libraries(wheel_engine PUBLIC GLESv2 m dl pthread)
    message(STATUS "Configuring for Raspberry Pi (OpenGL ES)")
endif()

# --- 4. THE GAME (User Code) ---

file(GLOB_RECURSE USER_SOURCES "game/*.cpp" "game/*.h")
add_executable(WheelApp ${USER_SOURCES}) # Using USER_SOURCES which includes your main.cpp

# Link the Game to the Engine
target_link_libraries(WheelApp PRIVATE wheel_engine)