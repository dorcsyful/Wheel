#include "core/AssetPath.h"

#ifdef _WIN32
  #define WIN32_LEAN_AND_MEAN
  #define NOMINMAX
  #include <windows.h>
#else
  #include <unistd.h>
  #include <limits.h>
#endif

namespace Wheel
{
    const std::string& GetExecutableDir()
    {
        // Computed once from the running executable's own path so asset lookups
        // are independent of the current working directory.
        static const std::string dir = []() -> std::string
        {
            std::string fullPath;
#ifdef _WIN32
            char buffer[MAX_PATH];
            DWORD length = GetModuleFileNameA(nullptr, buffer, MAX_PATH);
            fullPath.assign(buffer, length);
#else
            char buffer[PATH_MAX];
            ssize_t length = readlink("/proc/self/exe", buffer, sizeof(buffer));
            if (length > 0)
                fullPath.assign(buffer, static_cast<size_t>(length));
#endif
            const size_t slash = fullPath.find_last_of("/\\");
            return slash == std::string::npos ? std::string(".") : fullPath.substr(0, slash);
        }();
        return dir;
    }

    std::string GetAssetPath(const std::string& a_Relative)
    {
        return GetExecutableDir() + "/assets/" + a_Relative;
    }
}