#include "Enemy.h"
#include "DebugDraw.h"
#include "RaysCastCallback.h"
#include "Player.h"
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

Enemy::Enemy(b2Body* bd, TextureAtlas textureRef, float w, float h, float scale)
    : Character(bd, textureRef, w, h, scale)
{
    PM_SCALE = 0.6f;
    PM_SCALEY = 1.6f;
    isCharacter = true;
    m_color = { 1.f, 0.3f, 0.3f, 1.f };
    dontDestroy = false;

    //b2CircleShape shape2;
    //shape2.m_radius = (size*0.5f * UNRATIO);
    //shape2.m_p = b2Vec2((-size)* UNRATIO, -size * 0.5f * UNRATIO);

    //CreateSensor(physBody, size, -1);
    //CreateSensor(physBody, size, 1);

    m_Particle.ColorBegin = { 0.9f, 0.0f, 0.0f, 1.0f };
    m_Particle.ColorEnd = { 0.8f, 0.0f, 0.0f, 0.1f };
}

Enemy::~Enemy()
{
}

void Enemy::Update(float dt)
{
    ai_time += dt;
    Character::Update(dt);
}

#define MIN_WALL_NORMAL 0.71f

#define MIN_TURN_NORMAL 0.15f

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
        ai_time = 0;
    }

    if (ai_time > 0.2f)
    {
        if (!ai_player_left && ai_move_left && (psc_normal.x > MIN_TURN_NORMAL || psc_normal.x < -0.6f))
        {
            ai_move_left = false;
            ai_move_right = true;
            ai_time = 0;
        }
        if (!ai_player_right && ai_move_right && (psc_normal.x < -MIN_TURN_NORMAL || psc_normal.x > 0.6f))
        {
            ai_move_left = true;
            ai_move_right = false;
            ai_time = 0;
        }
    }

    bool ai_jump = false;

    auto player = Player::instance;
    if (player)
    {
        auto d = player->GetPosition() - GetPosition();
        if (d.y > 0.9f && d.y < 1.6f && abs(d.x) < 1.3f)
            ai_jump = true;
    }

    Input ninput = Input(ai_move_left, ai_move_right, ai_jump, false);
    ninput.update_move = true;
    return ninput;
}

#define RAY_DX 0.8f
#define RAY_DY 0.5f

void Enemy::UpdateCollisions(b2Vec2& vel)
{
    auto go_pos = GetPosition();
    Character::UpdateCollisions(vel);

    ai_player_left = false;
    ai_player_right = false;

    if (ai_move_left)
    {
        // left cast
        RaysCastCallback callback = RaysCastCallback::RayCastVec({ go_pos.x, go_pos.y }, { -RAY_DX, -RAY_DY });
        if (!callback.m_hit)
            psc_normal.x = max(psc_normal.x, (MIN_TURN_NORMAL + 0.02f)); // move right
        else if (abs(callback.m_normal.x) > MIN_WALL_NORMAL)
        {
            psc_normal.x = max(psc_normal.x, (MIN_TURN_NORMAL + 0.02f)); // move right
            auto hitgo = callback.GetHitGameObject();
            if (hitgo && hitgo->m_type == Objects::Player)
            {
                ai_player_left = true;
            }
        }
    }

    if (ai_move_right)
    {
        // right cast
        RaysCastCallback callback = RaysCastCallback::RayCastVec({ go_pos.x, go_pos.y }, { RAY_DX, -RAY_DY });
        if (!callback.m_hit)
            psc_normal.x = min(psc_normal.x, -(MIN_TURN_NORMAL + 0.02f)); // move left
        else if (abs(callback.m_normal.x) > MIN_WALL_NORMAL)
        {
            psc_normal.x = min(psc_normal.x, -(MIN_TURN_NORMAL + 0.02f)); // move left
            auto hitgo = callback.GetHitGameObject();
            if (hitgo && hitgo->m_type == Objects::Player)
            {
                ai_player_right = true;
            }
        }
    }
}

void Jelly::Enemy::Jump(float x, float power)
{
    Character::Jump(x, min(6.5f, power));
}


#if DEBUG
void Enemy::DebugDraw()
{
    Character::DebugDraw();

    auto go_pos = GetPosition();
    //DebugDraw::DrawLine(go_pos, { go_pos.x - RAY_DX, go_pos.y - RAY_DY }, { 1, 0, 0, 1 });
    //DebugDraw::DrawLine(go_pos, { go_pos.x + RAY_DX, go_pos.y - RAY_DY }, { 1, 0, 0, 1 });
    DebugDraw::DrawRay({ go_pos.x, go_pos.y }, { -RAY_DX, -RAY_DY }, { 1, 0, 0, 1 });
    DebugDraw::DrawRay({ go_pos.x, go_pos.y }, { RAY_DX, -RAY_DY }, { 1, 0, 0, 1 });
}
#endif
