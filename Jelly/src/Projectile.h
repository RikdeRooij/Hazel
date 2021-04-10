#pragma once

#include "GameObject.h"

namespace Jelly
{
    class Projectile :
        public GameObject
    {
    
    public:
        Projectile(b2Body* bd, TextureRef tex, float w, float h, glm::vec4 color);

        virtual bool OnBeginContact(GameObject* other, glm::vec2 normal) override;

        void Start(GameObject* owner, float xvel, float yvel);

        virtual void LateUpdate(float dt) override;

    private:
        GameObject* m_owner;
    };
}
