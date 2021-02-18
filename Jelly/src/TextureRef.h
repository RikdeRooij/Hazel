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
        operator bool() const { return val != nullptr; }

        const std::shared_ptr<Hazel::Texture2D>& Get() const { return val; }
        const bool Has() const { return val != nullptr; }

    private:
        std::shared_ptr<Hazel::Texture2D> val = nullptr;
    };
}
