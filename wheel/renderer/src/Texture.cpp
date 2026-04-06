#include "Texture.h"
#include <filesystem>
#include <iostream>
#define STB_IMAGE_IMPLEMENTATION
#include "../../dependencies/stb/stb_image.h"
std::pair<std::string, unsigned int> Wheel::Renderer::Texture::LoadTexture()
{
    assert(m_ID == -1 && "Texture already loaded.");
    std::pair<std::string, unsigned int> pair;
    glGenTextures(1, &m_ID);
    glBindTexture(GL_TEXTURE_2D, m_ID);
     // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, (GLint)m_RepeatType);	// set texture wrapping to GL_REPEAT (default wrapping method)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, (GLint)m_RepeatType);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (GLint)m_FilterType);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (GLint)m_FilterType);
    // load image, create texture and generate mipmaps
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
    // The FileSystem::getPath(...) is part of the GitHub repository so we can find files on any IDE/platform; replace it with your own image path.
    unsigned char *data = stbi_load((ASSETS_LOCATION + m_Path).c_str(), &width, &height, &nrChannels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);
    pair.first = m_Path;
    pair.second = m_ID;
    return pair;
}
