#pragma once

#include "PhysicsManager.h"
#include "Character.h"

namespace Jelly
{
    class Enemy :
        public Character
    {
    public:
        Enemy(float x, float y, float size, float scale, PhysicsManager* physicsMgr);
        ~Enemy();

        Input UpdateInput() override;
        void UpdateCollisions(b2Vec2& vel) override;
#if DEBUG
        void DebugDraw() override;
#endif

        bool ai_jump = false;
        bool ai_move_left = false;
        bool ai_move_right = false;

    };
}
