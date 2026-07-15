#include "AssetPath.h"

#include <algorithm>
#include <cctype>

#ifdef _WIN32
  #define WIN32_LEAN_AND_MEAN
  #define NOMINMAX
  #include <windows.h>
#endif

namespace Wheel
{
    static std::string to_lower(std::string s) {
        std::transform(s.begin(), s.end(), s.begin(),
                       [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
        return s;
    }

    static std::filesystem::path ResolveExecutableDir()
    {
#ifdef _WIN32
        std::wstring buf(MAX_PATH, L'\0');
        for (;;) {
            DWORD len = GetModuleFileNameW(nullptr, buf.data(),
                                           static_cast<DWORD>(buf.size()));
            if (len == 0)                       // failed; GetLastError() has the reason
                return std::filesystem::current_path();

            if (len < buf.size()) {             // fit: len excludes the null terminator
                buf.resize(len);                // trim trailing nulls
                return std::filesystem::path(buf).parent_path();
            }
            buf.resize(buf.size() * 2);         // truncated, grow and retry
        }
#else
        std::error_code ec;
        std::filesystem::path exe = std::filesystem::read_symlink("/proc/self/exe", ec);
        if (ec)
            return std::filesystem::current_path();
        return exe.parent_path();
#endif
    }

    const std::filesystem::path& GetExecutableDirFS()
    {
        static const std::filesystem::path dir = ResolveExecutableDir();
        return dir;
    }

    const std::string& GetExecutableDir()
    {
        static const std::string dir = GetExecutableDirFS().generic_string();
        return dir;
    }

    const std::filesystem::path& GetAssetPathFS()
    {
        static const std::filesystem::path dir = GetExecutableDirFS() / "assets";
        return dir;
    }

    const std::string& GetAssetPath()
    {
        static const std::string dir = GetAssetPathFS().generic_string() + "/";
        return dir;
    }

    const std::filesystem::path& GetShaderPathFS()
    {
        static const std::filesystem::path dir = GetAssetPathFS() / "shaders";
        return dir;
    }

    const std::string& GetShaderPath()
    {
        static const std::string dir = GetShaderPathFS().generic_string() + "/";
        return dir;
    }

    const std::filesystem::path& GetTexturePathFS()
    {
        static const std::filesystem::path dir = GetAssetPathFS() / "textures";
        return dir;
    }

    const std::string& GetTexturePath()
    {
        static const std::string dir = GetTexturePathFS().generic_string() + "/";
        return dir;
    }

    std::vector<std::filesystem::path> GetRecursivePathsByExtension(const std::string& a_Root,
                                                                    const std::string& a_Extension)
    {
        std::vector<std::filesystem::path> result;

        std::error_code ec;
        std::filesystem::recursive_directory_iterator it(
            a_Root, std::filesystem::directory_options::skip_permission_denied, ec);
        if (ec) return result;

        const std::filesystem::recursive_directory_iterator end;
        for (; it != end; it.increment(ec)) {
            if (ec) break;

            const std::filesystem::directory_entry& entry = *it;

            std::error_code fec;
            if (!entry.is_regular_file(fec) || fec) continue;

            std::string ext = to_lower(entry.path().extension().string());
            if (ext == a_Extension) result.push_back(entry.path());
        }

        return result;
    }

    std::vector<std::filesystem::path> GetPathsByExtension(const std::string& a_Directory,
        const std::string& a_Extension)
    {
        std::vector<std::filesystem::path> result;

        std::error_code ec;
        std::filesystem::directory_iterator it(
            a_Directory, std::filesystem::directory_options::skip_permission_denied, ec);
        if (ec) return result;

        const std::filesystem::directory_iterator end;
        for (; it != end; it.increment(ec)) {
            if (ec) break;

            const std::filesystem::directory_entry& entry = *it;

            std::error_code fec;
            if (!entry.is_regular_file(fec) || fec) continue;  // skips subdirs too

            std::string ext = to_lower(entry.path().extension().string());
            if (a_Extension == ext) result.push_back(entry.path());
        }

        return result;
    }

    std::vector<std::filesystem::path> GetPathsByExtensions(const std::string& a_Root,
        std::vector<std::string>& a_Extensions)
    {
        std::vector<std::filesystem::path> result;

        std::error_code ec;
        std::filesystem::directory_iterator it(
            a_Root, std::filesystem::directory_options::skip_permission_denied, ec);
        if (ec) return result;

        const std::filesystem::directory_iterator end;
        for (; it != end; it.increment(ec)) {
            if (ec) break;

            const std::filesystem::directory_entry& entry = *it;

            std::error_code fec;
            if (!entry.is_regular_file(fec) || fec) continue;  // skips subdirs too

            std::string ext = to_lower(entry.path().extension().string());
            if (std::find(a_Extensions.begin(), a_Extensions.end(), ext) != a_Extensions.end())
                result.push_back(entry.path());        }

        return result;
    }

    std::vector<std::filesystem::path> GetRecursivePathsByExtensions(const std::string& a_Root,
        const std::vector<std::string>& a_Extensions)
    {
        std::vector<std::filesystem::path> result;

        std::error_code ec;
        std::filesystem::recursive_directory_iterator it(
            a_Root, std::filesystem::directory_options::skip_permission_denied, ec);
        if (ec) return result; // root missing or unreadable

        const std::filesystem::recursive_directory_iterator end;
        for (; it != end; it.increment(ec)) {
            if (ec) break; // unrecoverable iteration error

            const std::filesystem::directory_entry& entry = *it;

            std::error_code fec;
            if (!entry.is_regular_file(fec) || fec) continue;

            std::string ext = to_lower(entry.path().extension().string());
            if (std::find(a_Extensions.begin(), a_Extensions.end(), ext) != a_Extensions.end())
                result.push_back(entry.path());
        }

        return result;
    }
}
