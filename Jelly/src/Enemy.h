#pragma once

#include "PhysicsManager.h"
#include "Character.h"

namespace Jelly
{
    class Enemy :
        public Character
    {
    public:
        Enemy(b2Body* bd, TextureAtlas textureRef, float w, float h, float scale);
        ~Enemy();

        virtual void Update(float dt) override;
        virtual Input UpdateInput() override;
        virtual void UpdateCollisions(b2Vec2& vel) override;

        virtual bool OnCollision(b2Vec2 normal, GameObject* other) override;

        virtual void Die() override;

        virtual void Jump(float x, float power) override;

#if DEBUG
        virtual void DebugDraw() const override;
#endif

        float ai_time = 0;
        bool ai_jump = false;
        bool ai_move_left = false;
        bool ai_move_right = false;
        bool ai_player_left = false;
        bool ai_player_right = false;

    };
}
