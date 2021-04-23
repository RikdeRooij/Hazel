#pragma once

#include "GameObject.h"

using namespace Jelly;

namespace Jelly
{
    class Weapon :
        public GameObject
    {
    public:
        Weapon::Weapon()
        {}

        Weapon::Weapon(TextureRef tex, glm::vec2 pos, glm::vec2 size, glm::vec2 origin)
            : GameObject(tex, pos, size, origin)
        {

        }

        bool Shoot(GameObject* owner, float speed, float addangle);


        virtual void Update(float dt) override;


        virtual void Draw(glm::vec2 pos, glm::vec2 size, float angle) const override;


        virtual void Draw(int layer) const override;

    protected:

        float lastFireTime = 0;
    };

}
