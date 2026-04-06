#pragma once
#include <string>

#include "glad.h"
#include "core/Globals.h"

namespace Wheel
{
    namespace Renderer
    {
        enum class RepeatType
        {
            Repeat = GL_REPEAT,
            ClampToEdge = GL_CLAMP_TO_EDGE,
            MirroredRepeat = GL_MIRRORED_REPEAT,
        };

        enum class FilterType
        {
            Nearest = GL_NEAREST,
            Linear = GL_LINEAR,
        };

        /**
         * @brief Pass this to the renderer to register a texture. Note: Creating the object will not load the texture. Pass it to the renderer via Renderer->LoadTexture()
         */
        class Texture
        {
        public:
            /**
             *
             * @param a_Path relative to the "assets" folder
             * @param a_FilterType how the texture should behave when sizing up or down
             * @param a_RepeatType what should happen when the texture is too small for the given mesh.
             */
            explicit Texture(const std::string& a_Path, FilterType a_FilterType = FilterType::Linear, RepeatType a_RepeatType = RepeatType::Repeat)
             : m_FilterType(a_FilterType), m_RepeatType(a_RepeatType)
            {
                m_Path = ASSETS_LOCATION + a_Path;
                m_ID = -1;
            }
            ~Texture() = default;
            std::string GetPath() const { return m_Path; }
            RepeatType GetRepeatType() const { return m_RepeatType; }
            void SetRepeatType(RepeatType a_Type) {
                m_RepeatType = a_Type;
            }
            FilterType GetFilterType() const { return m_FilterType; }
            void SetFilterType(FilterType a_Type)
            {
                m_FilterType = a_Type;
            }

            std::pair<std::string, unsigned int> LoadTexture();

            friend class Renderer;

        private:
            std::string m_Path;
            FilterType m_FilterType = FilterType::Linear;
            RepeatType m_RepeatType = RepeatType::Repeat;
            unsigned int m_ID;
        };
    }
}
