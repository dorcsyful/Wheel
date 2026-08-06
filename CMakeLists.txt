cmake_minimum_required(VERSION 3.20)
project(Wheel)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Emit compile_commands.json so editors/clangd can resolve include paths.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

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

add_library(imgui STATIC
    dependencies/imgui/imgui.cpp
    dependencies/imgui/imgui_draw.cpp
    dependencies/imgui/imgui_tables.cpp
    dependencies/imgui/imgui_widgets.cpp
    dependencies/imgui/imgui_impl_glfw.cpp
    dependencies/imgui/imgui_impl_opengl3.cpp
)
target_include_directories(imgui PUBLIC dependencies/imgui)
target_link_libraries(imgui PUBLIC glfw)
target_compile_definitions(imgui PRIVATE
    $<$<NOT:$<BOOL:${WIN32}>>:IMGUI_IMPL_OPENGL_ES2>
)


# --- 2. THE ENGINE (Wheel) ---

file(GLOB_RECURSE ENGINE_SOURCES CONFIGURE_DEPENDS "wheel/*.cpp" "wheel/*.h")
add_library(wheel_engine STATIC ${ENGINE_SOURCES})

# Include directories for the engine (so it can find its own and dependency headers)
target_link_libraries(wheel_engine PUBLIC glad glfw imgui)
target_include_directories(wheel_engine PUBLIC
        wheel/)

target_compile_definitions(wheel_engine PRIVATE
        $<$<CONFIG:Debug>:DEBUG_BUILD>
)
# --- 3. PLATFORM SPECIFIC LINKING ---

if(WIN32)
    # Windows uses standard OpenGL
    target_link_libraries(wheel_engine PUBLIC opengl32)
    message(STATUS "Configuring for Windows (Desktop OpenGL)")
else()
    # Raspberry Pi 3 uses OpenGL ES 2.0/3.0
    # 'm' is math library, 'dl' is for loading drivers
    target_link_libraries(wheel_engine PUBLIC GLESv2 m dl pthread)
    message(STATUS "Configuring for Raspberry Pi (OpenGL ES)")
endif()

# QA harness/oracles and their tests are plain C++ over wheel_engine — no
# renderer, no window — so they build the same way on every target platform.
# That's also what the cross-target oracle (same scene on Windows vs RPi)
# needs: these binaries must exist on both.
add_subdirectory("qa")
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/qa/Google_tests/CMakeLists.txt")
    add_subdirectory("qa/Google_tests")
    # Must come after qa/Google_tests: reuses the gtest/gtest_main targets
    # defined by its lib subdirectory instead of re-adding googletest.
    add_subdirectory("qa/harness_tests")
    add_subdirectory("qa/oracle_tests")
    add_subdirectory("qa/agent_tests")
endif()

# --- 4. THE GAME (User Code) ---

file(GLOB_RECURSE USER_SOURCES CONFIGURE_DEPENDS "game/*.cpp" "game/*.h")
add_executable(WheelApp ${USER_SOURCES}
        game/demos/DemoSelector.cpp
        game/demos/DemoSelector.h
        game/demos/Demo.h
        game/demos/scenes/FallingColliders.cpp
        game/demos/scenes/FallingColliders.h
        game/demos/scenes/CollisionPhases.cpp
        game/demos/scenes/CollisionPhases.h
        game/demos/scenes/NewtonsCradle.cpp
        game/demos/scenes/NewtonsCradle.h) # Using USER_SOURCES which includes your main.cpp

# Link the Game to the Engine
target_link_libraries(WheelApp PRIVATE wheel_engine)
target_compile_definitions(WheelApp PRIVATE
        $<$<CONFIG:Debug>:DEBUG_BUILD>
)

# Copy assets next to the executable so the app finds them regardless of the
# working directory it is launched from (paths are resolved via GetExecutableDir()).
add_custom_command(TARGET WheelApp POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/assets" "$<TARGET_FILE_DIR:WheelApp>/assets")
