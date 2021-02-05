#pragma once

#include "PhysicsManager.h"
#include "GameObject.h"

class Player :
    public GameObject
{
public:
    Player(float x, float y, float size, float scale, PhysicsManager* physicsMgr);
    ~Player();


    void draw(int layer) override;

    void update(float dt) override;

    void moveX(float power) const;
    void Jump(float x, float power) const;
    void MoveDown();


    void Explode();

    void Die() override;

    bool dead = false;

    bool grounded = false;
    bool wallLeft = false;
    bool wallRight = false;
    bool ceiling = false;
    bool inside = false;


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

};
