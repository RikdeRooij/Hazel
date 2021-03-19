#include "Enemy.h"
#include "DebugDraw.h"
#include "RaysCastCallback.h"
#pragma comment(lib, "winmm.lib")

using namespace Jelly;

void CreateSensor(b2Body* physBody, float size, float xsign)
{
    b2PolygonShape shape2;
    auto s = b2Vec2(size * 0.5f * UNRATIO, size * 0.5f * UNRATIO);
    auto ss = b2Vec2(size * 0.25f * UNRATIO, size * 0.25f * UNRATIO);
    shape2.SetAsBox(ss.x, ss.y, { xsign * (s.x + ss.x + 0.1f * UNRATIO), -s.y }, 0);
    b2FixtureDef fixtureDef2 = (FixtureData::SENSOR);
    fixtureDef2.isSensor = true;
    fixtureDef2.shape = &shape2;
    physBody->CreateFixture(&fixtureDef2);
}

Enemy::Enemy(float x, float y, float size, float scale, PhysicsManager* physicsMgr)
    : Character(x, y, size, scale, physicsMgr)
{
    PM_SCALE = 0.6f;

    //b2CircleShape shape2;
    //shape2.m_radius = (size*0.5f * UNRATIO);
    //shape2.m_p = b2Vec2((-size)* UNRATIO, -size * 0.5f * UNRATIO);

    //CreateSensor(physBody, size, -1);
    //CreateSensor(physBody, size, 1);
}

Enemy::~Enemy()
{
}

Character::Input Enemy::UpdateInput()
{
    if (!grounded)
    {
        ai_move_left = ai_move_right = false;
    }
    else if (!ai_move_left && !ai_move_right)
    {
        ai_move_left = Random::Float() > 0.5f;
        ai_move_right = !ai_move_left;
    }

    if (ai_move_left && (psc_normal.x > 0.1f || psc_normal.x < -0.6f))
    {
        ai_move_left = false;
        ai_move_right = true;
    }
    if (ai_move_right && (psc_normal.x < -0.1f || psc_normal.x > 0.6f))
    {
        ai_move_left = true;
        ai_move_right = false;
    }

    Input ninput = Input(ai_move_left, ai_move_right, false, false);
    ninput.update_move = true;
    return ninput;
}

void Enemy::UpdateCollisions(b2Vec2& vel)
{
    auto go_pos = GetPosition();
    Character::UpdateCollisions(vel);

    RaysCastCallback callback;

    // left cast
    callback = RaysCastCallback::RayCast({ go_pos.x, go_pos.y }, { go_pos.x - .8f, go_pos.y - 0.5f });
    if (!callback.m_hit)
        psc_normal.x = max(psc_normal.x, 0.2f); // move right
    else if (abs(callback.m_normal.x) > 0.5f)
        psc_normal.x = max(psc_normal.x, 0.2f); // move right

    // right cast
    callback = RaysCastCallback::RayCast({ go_pos.x, go_pos.y }, { go_pos.x + .8f, go_pos.y - 0.5f });
    if (!callback.m_hit)
        psc_normal.x = min(psc_normal.x, -0.2f); // move left
    else if (abs(callback.m_normal.x) > 0.5f)
        psc_normal.x = min(psc_normal.x, -0.2f); // move left
}


#if DEBUG
void Enemy::DebugDraw()
{
    Character::DebugDraw();

    auto go_pos = GetPosition();
    glm::vec2 target = glm::vec2(go_pos.x - 0.8f, go_pos.y - 0.4f);
    DebugDraw::DrawLine(go_pos, target, { 1,0,1,1 });
}
#endif
