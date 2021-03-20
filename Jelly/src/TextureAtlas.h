#pragma once

#include "TextureRef.h"
#include "glm/vec4.hpp"

namespace Jelly
{
    class TextureAtlas
    {
    public:
        TextureAtlas();
        TextureAtlas(std::string path);
        TextureAtlas(const std::shared_ptr<Hazel::Texture2D>& x);

        void Load(std::string path);

        static TextureRef LoadXML(std::string path, std::vector<glm::vec4>& rects);

        bool Valid() const { return texure.Has() && rects.size() > 0; }

        TextureRef GetTextureRef() const { return texure; }

        std::vector<glm::vec4>& GetRects() { return rects; }
        int RectCount() const { return (int)rects.size(); }
        glm::vec4& GetRect(int i) { return rects[i]; }


        static float AnimTime(const float& animtime, const float &len, const bool pingpong)
        {
            float f = fmod(abs(animtime), ((len) * (pingpong ? 2 : 1)));
            if (pingpong && f >= (len)) f = (len) * 2 - f;
            return f < 0 ? 0 : f > len ? len : f;
        }

        static unsigned int AnimTimeIndex(const float& animtime, const unsigned int &exmax, const bool pingpong)
        {
            auto i = pingpong
                ? static_cast<unsigned int>(round(AnimTime(animtime - .4999f, (float)(exmax - 1), pingpong)))
                : static_cast<unsigned int>(floor(AnimTime(animtime, (float)(exmax), pingpong)));
            return i < 0u ? 0u : i > exmax - 1 ? exmax - 1 : i;
        }

        glm::vec4& AnimationRect(const float& t, const int& frame_start, const int& frame_length, const bool& pingpong);

    private:

        std::vector<glm::vec4> rects = std::vector<glm::vec4>();
        TextureRef texure = TextureRef();
    };
}
