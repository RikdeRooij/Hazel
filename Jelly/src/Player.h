#pragma once

//#define PM_SCALE 2.0f

#include "PhysicsManager.h"
#include "Character.h"

namespace Jelly
{
    class Player :
        public Character
    {
    public:
        static Player* instance;

        Player(b2Body* bd, TextureAtlas textureRef, float w, float h, float scale);
        ~Player();

        Input UpdateInput() override;
    };
}
