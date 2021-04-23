#pragma once

#include "GameObject.h"

namespace Jelly
{
    class Projectile :
        public GameObject
    {
    
    public:
        Projectile(b2Body* bd, TextureRef tex, float w, float h, glm::vec4 color);

        virtual bool OnBeginContact(GameObject* other, b2Fixture* fixture, glm::vec2 pos, glm::vec2 normal) override;

        void Start(GameObject* owner, float xvel, float yvel);

        virtual void LateUpdate(float dt) override;

    private:
        GameObject* m_owner;
        float velSpeedLerp = 0.9f;
        int hitcount = 0;
        float lifetime = 16.0f;
    };
}
