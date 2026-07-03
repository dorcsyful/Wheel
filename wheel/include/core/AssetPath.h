#pragma once
#include <string>
#include <vector>
#include <filesystem>
namespace Wheel
{
    /**
     * @brief Absolute path to the directory containing the running executable
     */
    const std::filesystem::path& GetExecutableDirFS();
    /** @brief Same as GetExecutableDirFS(), as a forward-slash std::string. */
    const std::string& GetExecutableDir();

    /**
     * @brief Absolute path to the assets folder. All assets are copied next to
     * the executable at build time, so this is GetExecutableDir()/assets.
     */
    const std::filesystem::path& GetAssetPathFS();
    /** @brief Same as GetAssetPathFS(), as a forward-slash std::string with a trailing '/'. */
    const std::string& GetAssetPath();

    /** @brief Absolute path to the shaders folder (assets/shaders). */
    const std::filesystem::path& GetShaderPathFS();
    /** @brief Same as GetShaderPathFS(), as a forward-slash std::string with a trailing '/'. */
    const std::string& GetShaderPath();

    /** @brief Absolute path to the textures folder (assets/textures). */
    const std::filesystem::path& GetTexturePathFS();
    /** @brief Same as GetTexturePathFS(), as a forward-slash std::string with a trailing '/'. */
    const std::string& GetTexturePath();

    /**
     * @brief Recursively gets all files in a given directory
     * @param a_Root The starting directory, absolute path
     * @param a_Extension File extension eg. ".png"
     * @return
     */
    std::vector<std::filesystem::path> GetRecursivePathsByExtension(const std::string& a_Root, const std::string& a_Extension);

    /**
     * @brief Gets all files in a given directory
     * @param a_Directory The directory, absolute path
     * @param a_Extension File extension eg. ".png"
     * @return
     */
    std::vector<std::filesystem::path> GetPathsByExtension(const std::string& a_Directory, const std::string& a_Extension);

    /**
     * @brief Gets all files in a given directory
     * @param a_Root The starting folder
     * @param a_Extensions A vector of file extensions eg. ".png"
     * @return
     */
    std::vector<std::filesystem::path> GetPathsByExtensions(const std::string& a_Root, std::vector<std::string>& a_Extensions);

    /**
     * @brief Recursively gets all files in a given directory
     * @param a_Root The starting folder
     * @param a_Extensions A vector of file extensions eg. ".png"
     * @return
     */
    std::vector<std::filesystem::path> GetRecursivePathsByExtensions(const std::string& a_Root, const std::vector<std::string>& a_Extensions);

}