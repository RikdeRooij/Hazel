#pragma once

#include "PhysicsManager.h"
#include "GameObject.h"
#include "ParticleSystem.h"

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
        void Jump(float x, float power) const;
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

        float time;
        float lastJumpTime;

        void PlayJumpSound() {} //PlayJumpSound(RandomInt(_countof(jumpSound)));

        void PlayJumpSound(int i)
        {

        }

        ParticleProps m_Particle;

    };
}
