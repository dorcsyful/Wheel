#pragma once
#include <string>

namespace Wheel
{
    /**
     * @brief Absolute path to the directory containing the running executable.
     * Resolved once from the OS, so it never depends on the process working
     * directory (which differs between IDEs, CMake generators, and launches).
     */
    const std::string& GetExecutableDir();

    /**
     * @brief Resolve an asset path relative to the executable, e.g.
     * GetAssetPath("shaders/base.vert") -> "<exeDir>/assets/shaders/base.vert".
     * Assets are copied next to the executable at build time (see CMakeLists.txt).
     */
    std::string GetAssetPath(const std::string& a_Relative);
}