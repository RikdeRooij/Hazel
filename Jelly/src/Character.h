#pragma once

#include "GameObject.h"
#include "PhysicsManager.h"
#include "ParticleSystem.h"
#include "TextureAtlas.h"

namespace Jelly
{
    class Character :
        public GameObject
    {
    public:
        float PM_SCALE = 0.5f;

        Character(float x, float y, float size, float scale, PhysicsManager* physicsMgr);
        ~Character();

        typedef struct Input
        {
            Input(bool _left, bool _right, bool _up, bool _down)
            {
                this->left = _left; this->right = _right; this->down = _down; this->up = _up;
                this->update_move = true;
            }
            bool left; bool right; bool down; bool up;
            bool update_move;
        } Input;

        virtual void Draw(int layer) override;
#if DEBUG
        virtual void DebugDraw() override;
#endif

        virtual void Update(float dt) override;

        virtual Input UpdateInput();
        virtual void UpdateCollisions(b2Vec2& vel);
        virtual void UpdateMove(Input input, b2Vec2& vel);


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

        glm::vec2 psc_pos;
        glm::vec2 psc_normal;
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
