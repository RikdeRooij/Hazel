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
        Player(float x, float y, float size, float scale, PhysicsManager* physicsMgr);
        ~Player();

        Input UpdateInput() override;
    };
}
