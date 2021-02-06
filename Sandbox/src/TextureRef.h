#pragma once

#include <memory>
#include "Hazel/Renderer/Texture.h"

namespace Jelly
{
    class TextureRef
    {
    public:
        TextureRef() : val(nullptr) {}
        //TextureRef(std::shared_ptr<Hazel::Texture2D> x) : val(x) {}
        TextureRef(const std::shared_ptr<Hazel::Texture2D>& x) : val(x) {}

        //operator std::shared_ptr<Hazel::Texture2D>() const { return val; }

        const std::shared_ptr<Hazel::Texture2D>& get() const { return val; }

    private:
        std::shared_ptr<Hazel::Texture2D> val;
    };
}
