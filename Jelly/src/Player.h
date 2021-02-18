#pragma once

#include "PhysicsManager.h"
#include "GameObject.h"
#include "ParticleSystem.h"
#include "TextureAtlas.h"

namespace Jelly
{
    class Player :
        public GameObject
    {
    public:
        Player(float x, float y, float size, float scale, PhysicsManager* physicsMgr);
        ~Player();


        void Draw(int layer) override;

        void Update(float dt) override;

        void UpdateCollisions(b2Vec2& vel);
        void UpdateMove(b2Vec2& vel);


        void MoveX(float power) const;
        void Jump(float x, float power);
        void Move(float dx, float dy) const;


        void Explode();

        void Die() override;

        bool dead = false;

        bool grounded = false;
        bool wallLeft = false;
        bool wallRight = false;
        bool ceiling = false;
        bool inside = false;

        float speed = 0;

        bool key_left = false;
        bool key_right = false;

#if DEBUG
        typedef struct ContactData
        {
            ContactData(glm::vec2 _pos, glm::vec2 _normal, bool _down, bool _up)
            {
                this->pos = _pos;
                this->normal = _normal;
                this->down = _down;
                this->up = _up;
            }
            glm::vec2 pos;
            glm::vec2 normal;
            bool down;
            bool up;

        } ContactData;
        std::vector<ContactData> contacts;
#endif

    protected:

        b2Vec2 vel = b2Vec2(0.f, 0.f);
        b2Vec2 prev_vel = b2Vec2(0.f, 0.f);
        float animtime = 0;
        float time = 0;
        float lastJumpTime = 0;
        float lastLandTime = 0;
        float lastInsideTime = 0;
        float lastInsideY = 0;
        bool jumpanim = false;
        float anim_squish = 0;

        void PlayJumpSound() const { PlayJumpSound(RandomInt(3)); }

        void PlayJumpSound(int i) const;

        ParticleProps m_Particle;

        TextureAtlas textureAtlas;
    };
}
