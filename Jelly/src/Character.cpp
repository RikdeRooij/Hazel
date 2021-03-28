#include "Character.h"
#include "Hazel/Renderer/Renderer2D.h"
#include "Hazel/Core/Input.h"
#include "../vendor/tinyxml2/tinyxml2.h"
#include "TextureAtlas.h"
#include "DebugDraw.h"
#include "ObjectManager.h"
#include "AudioManager.h"

using namespace Jelly;

//#define TEST 1
//#define PM_SCALE 0.5f

#define MOVE_SIDE (3.0f * PM_SCALE) // Horizontal (left-right) movement power
#define MOVE_SIDE_LIMIT (4.0f * PM_SCALE) // Horizontal (left-right) movement limit

#define MOVE_JUMP_UP (7.0f * PM_SCALE * PM_SCALEY) // jump power
#define MOVE_JUMP_TIME 200 // min time in milliseconds between jumps

#define MOVE_WALLJUMP_UP (4.0f * PM_SCALE * PM_SCALEY) // wall-jump up power
#define MOVE_WALLJUMP_SIDE (4.0f * PM_SCALE) // wall-jump side (away from wall) power

#define JUMP_NOXMOVE_TIME 50.0f // miliseconds x movement is limited after jump

#define MOVE_FALLOFF 1.0f // speed at which move-power starts reducing
#define MOVE_FALLOFF_POWER 1.0f // scales reduction power on movement at high speed


#define MOVE_AIR 0.25f // amount of movement when not grounded
#define MOVE_AIR_MOMENTUM 3.0f // momentum (speed) needed to move when fully airborne (reducing move power when less)
#define MOVE_AIR_COLLDING 0.5f // amount of movement when not grounded but still touching a wall or ceiling


#define MOVE_DAMPING 1.0f // damping when grounded

#define GROUND_NORMAL 0.7f // max absolute normal.x to be considered ground. (acos(0.7) * (180/PI) = 45.6 degrees)


Character::Character(b2Body* bd, TextureAtlas textureRef, float w, float h, float scale)
    : GameObject(bd, w, h, { 1,1,1,1 })
{
    this->m_textureAtlas = textureRef;
    this->m_texture = textureRef.GetTextureRef();

    dontDestroy = false;
    isCharacter = true;

    b2BodyUserData data;
    data.pointer = reinterpret_cast<uintptr_t>(this);
    bd->SetUserData(data);
    bd->SetFixedRotation(true);
    //physBody->SetLinearDamping(MOVE_DAMPING);
    bd->SetLinearDamping(0);

    time = 0;
    lastJumpTime = 0;
    lastInsideTime = 0;

    m_body = bd;

    //dontDestroy = false;
    dontDraw = false;

    glm::vec2 tsize = { m_texture.Get()->GetWidth(), m_texture.Get()->GetHeight() };
    if (m_textureAtlas.Valid())
    {
        auto rec = m_textureAtlas.GetRect(0);
        tsize.x = rec.z;
        tsize.y = rec.w;
    }
    float tdx = (w + 0.08f) / tsize.x;
    float tdy = (h + 0.08f) / tsize.y;
    width = tsize.x * tdx;// +0.05f;
    height = tsize.y * tdy;// +0.05f;

    posx = m_body->GetPosition().x * RATIO;
    posy = m_body->GetPosition().y * RATIO;
    angle = toDegrees(m_body->GetAngle());
    m_origin = { .5f, .5f };
    m_color = { 1, 1, 1, 1 };
    GetBody()->SetLinearVelocity(b2Vec2(0, 0));


    // Init here
    m_Particle.ColorBegin = { 0.0f, 0.9f, 0.0f, 1.0f };
    m_Particle.ColorEnd = { 0.0f, 0.8f, 0.0f, 0.1f };
    m_Particle.Texture = Hazel::Texture2D::Create("assets/particle.png");
    m_Particle.SizeBegin = 0.25f, m_Particle.SizeVariation = 0.2f, m_Particle.SizeEnd = 0.0f;
    m_Particle.LifeTime = 0.60f;
    m_Particle.Velocity = { 0.1f, 0.1f };
    m_Particle.VelocityVariation = { 2.5f, 2.5f };
    m_Particle.Position = { 0.0f, 0.0f };
}

Character::~Character()
{

}

void Character::Update(float dt)
{
    time += dt * 1000;
    animtime += dt;


    //b2Vec2 pos = getBody()->GetPosition();
    prev_vel = vel;
    vel = GetBody()->GetLinearVelocity();

    bool was_grounded = grounded;
    bool was_wallLeft = wallLeft;
    bool was_wallRight = wallRight;

    UpdateCollisions(vel);

    if (inside)
    {
        lastInsideTime = time;
        lastInsideY = posy;
    }

    //getBody()->SetLinearVelocity(vel);
    speed = vel.Length();


    if (grounded && !was_grounded)
    {
        if (prev_vel.y < -4.f)
        {
            //DBG_OUTPUT("land:   %.3f   %.3f", vel.y, prev_vel.y);
            lastLandTime = time;
        }
    }

    bool klr = (key_left || key_right);
    bool wall_squish = klr && (wallLeft || wallRight || was_wallLeft || was_wallRight);
    anim_squish = Interpolate::Linear(anim_squish, wall_squish, dt * 10);


    float damping = !inside && grounded ? MOVE_DAMPING : 0.01f;
    GetBody()->SetLinearDamping(damping);

    Input input = UpdateInput();

    if (input.update_move)
    {
        UpdateMove(input, vel);

        if (dead && GetBody())
        {
            this->height = -std::abs(height);
            GetBody()->SetLinearVelocity(b2Vec2(0, 0));
            GetBody()->SetAwake(false);
            if (GetBody()->IsEnabled())
            {
                if (!GetBody()->GetWorld()->IsLocked())
                    GetBody()->SetEnabled(false);
            }
        }
    }

    GameObject::Update(dt);
}

Character::Input Character::UpdateInput()
{
    return Input(false, false, false, false);
}

void Character::UpdateCollisions(b2Vec2& vel)
{
    if (dead)
        return;
    auto go_pos = GetPosition();

    grounded = false;
    wallLeft = false;
    wallRight = false;
    ceiling = false;
    inside = false;

#if DEBUG
    contacts.clear();
#endif

    psc_pos = { 0,0 };
    psc_normal = { 0,0 };
    int ceCount = 0;
    auto body = GetBody();
    for (const b2ContactEdge* ce = body->GetContactList(); ce; ce = ce->next)
    {
        if (!ce)
            continue;
        b2Contact* c = ce->contact;

        b2Fixture* fixtureA = c->GetFixtureA();
        b2Fixture* fixtureB = c->GetFixtureB();

        GameObject* goA = PhysicsManager::GetUserData<GameObject*>(fixtureA);
        GameObject* goB = PhysicsManager::GetUserData<GameObject*>(fixtureB);

        GameObject* other = goA == this ? goB : goB == this ? goA : nullptr;

        if (!other)
            throw;
        //DBG_WRITE("## %s  ##  %s", (typeid(*goA).name()), (typeid(*goB).name()));

        b2Manifold* manifold = c->GetManifold();
        if (manifold->pointCount <= 0)
            continue;

        b2Vec2 localNormal = manifold->localNormal;
        if (goA != this)
            localNormal = -localNormal;

        if (!OnCollision(localNormal, other))
            continue;

        b2WorldManifold worldManifold;
        c->GetWorldManifold(&worldManifold);

        ceCount++;
        glm::vec2 avgPos(0, 0);
        glm::vec2 ctrDirVec(0, 0);

        if (manifold->pointCount > 0)
        {
            for (int i = 0; i < manifold->pointCount; i++)
            {
                glm::vec2 sfPos = { worldManifold.points[i].x * RATIO, worldManifold.points[i].y * RATIO };
                avgPos += sfPos;
            }
            avgPos = glm::vec2(avgPos.x / (float)manifold->pointCount, avgPos.y / (float)manifold->pointCount);
            ctrDirVec = avgPos - go_pos;
        }
        else
        {
            avgPos = go_pos;
            ctrDirVec = glm::vec2(0, 0);
        }

        b2Vec2 wmNormal = worldManifold.normal;
        auto normal = b2Vec2(abs(wmNormal.x) * ctrDirVec.x,
                             abs(wmNormal.y) * ctrDirVec.y);
        normal.Normalize();

        bool isGround = (normal.y < 0 && abs(normal.x) < GROUND_NORMAL);
        bool isLeft = normal.x <= -GROUND_NORMAL;
        bool isRight = normal.x >= GROUND_NORMAL;
        bool isUp = (normal.y > 0 && abs(normal.x) < GROUND_NORMAL);

        bool isInside = manifold->pointCount > 0 &&
            (glm::length(ctrDirVec) < 0.17f * height ||
            (abs(ctrDirVec.x) < 0.34f * height && abs(ctrDirVec.y) < 0.32f * height));

        if (isInside)
            inside = true;

        {
            if (isGround && !isInside)
                grounded = true;
            else
            {
                if (isLeft)
                    wallLeft = true;
                if (isRight)
                    wallRight = true;
                if (isUp && !isInside)
                    ceiling = true;
            }
        }

#if DEBUG
        ContactData cd = ContactData(avgPos, { normal.x, normal.y }, isGround, isUp);
        contacts.push_back(cd);
#endif
        psc_pos.x += avgPos.x;
        psc_pos.y += avgPos.y;
        psc_normal.x += ctrDirVec.x;
        psc_normal.y += ctrDirVec.y;

        vel = clipVector(vel, normal, 0.33f);
    }

    if (ceCount > 0)
    {
        psc_pos /= (ceCount);
        psc_normal /= ceCount;
        psc_normal = glm::normalize(psc_normal);
    }
    else
    {
        psc_pos = go_pos;
    }
}

void Character::UpdateMove(Input input, b2Vec2& vel)
{
    float mass = GetBody()->GetMass();

    float jumpDeltaTime = abs(time - lastJumpTime);

    bool airborne = inside ||
        (!grounded && !wallLeft && !wallRight && !ceiling);

    float move_pwr_scale = grounded ? 1.0f :
        fmax(airborne ? 0 : MOVE_AIR_COLLDING,
             fmin(pow3(fmin(1.0f, speed / MOVE_AIR_MOMENTUM)) * MOVE_AIR, MOVE_AIR));

    float move_falloff = 1.0f + fmax(speed - MOVE_FALLOFF, 0.0f) * MOVE_FALLOFF_POWER;
    //move_falloff = 1.0f;

    float power = (mass / move_falloff) * move_pwr_scale;

    // _JUMP vel.y:  0.0 | 3.4 | 4.2
    bool allowJump = //!inside &&  
        vel.y < 4.5f &&
        (grounded || wallLeft || wallRight); // || !ceiling)

    bool allowUpJump = allowJump && !inside;
    if (!allowUpJump && inside && grounded && !wallLeft && !wallRight && abs(vel.y) < 0.01f)
        allowUpJump = true;

    if (allowJump && (jumpDeltaTime > MOVE_JUMP_TIME))
    {
        if (input.up)
        {
            //if(grounded) DBG_OUTPUT("_JUMP %.1f", vel.y);

            float velup = clamp01(vel.y); // 1 when going up
            float veldn = clamp01(-vel.y); // 1 when going down

            float opposite = (1.0f - veldn * 0.8f); // smaller when going down

            if (allowUpJump && grounded && vel.y < MOVE_JUMP_UP * 0.85f)
            {
                float extend = (1.0f + velup * 0.2f); // larger when going up
                float limitjump = (lastInsideY <= posy ? clamp01(abs(time - lastInsideTime) / 200.0f) : 1.0f);
                // *clamp01(jumpDeltaTime / (MOVE_JUMP_TIME * 2));
                float jumpvel = max(vel.y, MOVE_JUMP_UP * opposite * extend * limitjump);
                if (jumpvel - vel.y > MOVE_JUMP_UP * 0.1f)
                {
                    Jump(vel.x, jumpvel);
                    lastJumpTime = time;
                    if (velup < 0.5f) PlayJumpSound(0);
                    else PlayJumpSound(1);
                }
            }
            else
            {
                float reduce = (1.0f - velup * 0.4f); // smaller when going up

                if (wallLeft)
                {
                    auto force = GetBody()->GetForce();
                    force.x *= 0.3f;
                    GetBody()->SetForce(force);

                    Jump(MOVE_WALLJUMP_SIDE * reduce, MOVE_WALLJUMP_UP * opposite);
                    lastJumpTime = time;
                    if (veldn < 0.5f) PlayJumpSound(0);
                    else PlayJumpSound(2);
                }
                if (wallRight)
                {
                    auto force = GetBody()->GetForce();
                    force.x *= 0.3f;
                    GetBody()->SetForce(force);

                    Jump(-MOVE_WALLJUMP_SIDE * reduce, MOVE_WALLJUMP_UP * opposite);
                    lastJumpTime = time;
                    if (veldn < 0.5f) PlayJumpSound(0);
                    else PlayJumpSound(2);
                }
            }
        }
    }

    bool keyleft = input.left;
    if (keyleft)
    {
#if TEST
        float limit = clamp01(MOVE_SIDE_LIMIT + vel.x) * clamp01(jumpDeltaTime / JUMP_NOXMOVE_TIME);
        moveX(-power * limit * MOVE_SIDE);
#else
        float opposite = !grounded ? 1.0f : 0.05f + pow2(clamp01(-(vel.x / MOVE_SIDE_LIMIT)));
        float limit = clamp01(MOVE_SIDE + vel.x) * clamp01(jumpDeltaTime / JUMP_NOXMOVE_TIME) * opposite;
        MoveX(-fmin(power * limit * MOVE_SIDE, MOVE_SIDE_LIMIT));
#endif
        this->width = std::abs(width);
    }

    bool keyright = input.right;
    if (keyright)
    {
#if TEST
        float limit = clamp01(MOVE_SIDE_LIMIT - vel.x) * clamp01(jumpDeltaTime / JUMP_NOXMOVE_TIME);
        moveX(power * limit * MOVE_SIDE);
#else
        float opposite = !grounded ? 1.0f : 0.05f + pow2(clamp01((vel.x / MOVE_SIDE_LIMIT)));
        float limit = clamp01(MOVE_SIDE - vel.x) * clamp01(jumpDeltaTime / JUMP_NOXMOVE_TIME) * opposite;
        MoveX(fmin(power * limit * MOVE_SIDE, MOVE_SIDE_LIMIT));
#endif
        this->width = -std::abs(width);
    }

    if (input.down)
    {
        Move(0.f, -1.f);
    }

    if (jumpanim)
    {
        const float animspeed = 7.0f;// 4.5f;
        const uint acnt = 3;
        //if (animtime * animspeed >= ((acnt - 1) * 2 - 0.1f))
        if (animtime * animspeed >= (acnt - 0.1f))
            jumpanim = false;
    }
    //if (!jumpanim)
    //{
    //    if (key_left != keyleft) animtime = 0;
    //    if (key_right != keyright) animtime = 0;
    //}
    key_left = keyleft;
    key_right = keyright;
}

void Character::MoveX(float power)
{
    if (dead)
        return;

#if TEST
    b2Vec2 impulse = b2Vec2(power, 0);
    b2Vec2 impulsePoint = GetBody()->GetPosition();
    GetBody()->ApplyLinearImpulse(impulse, impulsePoint, true);
#else
    //DBG_OUTPUT("moveX:: %.3f", (power));
    GetBody()->ApplyForceToCenter({ power * 100, 0 }, true);
#endif
}

void Character::Jump(float x, float power)
{
    if (dead)
        return;

    //if (!jumpanim)
    {
        jumpanim = true;
        animtime = 0;
    }

    //PlayJumpSound();

    b2Vec2 impulse = b2Vec2(x, power);
    GetBody()->SetLinearVelocity(impulse);
}

void Character::Move(float dx, float dy)
{
    if (dead)
        return;

    b2Vec2 vel = GetBody()->GetLinearVelocity();
    float power = GetBody()->GetMass() * (1.0f / fmax(vel.Length(), 1.0f));

    b2Vec2 impulse = b2Vec2(dx * power, dy * power);
    b2Vec2 impulsePoint = GetBody()->GetPosition();
    GetBody()->ApplyLinearImpulse(impulse, impulsePoint, true);
}

void Character::Explode()
{
    m_Particle.Position = { posx , posy };
    ParticleSystem::S_Emit(100, m_Particle);
}

void Character::Die()
{
    if (dead)
        return;

    dead = true;

    GetBody()->SetAwake(false);

    SetColor({ 1,1,1,0.1f });

    Explode();
}

void Character::PlayJumpSound(int i) const
{
    //AudioManager::PlayFile(format("assets/Sounds/jump%d.wav", (i + 1)).c_str());
    AudioManager::PlaySoundType((Sounds::Type)(Sounds::Jump1 + i));
}

void Character::Draw(int layer) const
{
    if (dontDraw)
        return;
    //GameObject::Draw(layer);

    if (this->m_draw_layer != layer)
        return;

    float px = posx + abs(width) * (0.5f - m_origin.x);
    float py = posy + height * (0.5f - m_origin.y);


    auto z = -0.99f + (static_cast<float>(m_type) / static_cast<float>(Objects::MAX_COUNT) * 0.5f);
    z += (instanceID * 0.000001f);

    const float anim_speed_breathe = 3;

    const float anim_idle_speed = 5;
    const float anim_move_speed = anim_idle_speed;


    float anim_land = clamp01(abs(lastLandTime - time) * 0.005f);
    float hm = anim_land * 0.3f + 0.7f;
    py -= height * (1 - hm) * .25f;
    float wm = 1 + ((1 - anim_land) * 0.15f);

    float s = Interpolate::Hermite(0, 1, TextureAtlas::AnimTime((animtime + 1.5f) * anim_speed_breathe, 3.0f, true) / 3.0f);
    const float breathe = 0.04f;
    float sw = (s * breathe);// *1.5f;
    float sh = ((0.5f + (0.5f - s)) * breathe);
    py += sh * .5f;
    px += sign(width) * 0.04f; // perspective image

    if (m_textureAtlas.Valid() && jumpanim)
    {
        const float animspeed = 7.0f;// 4.5f;
        const uint acnt = 3;

        ////if (animtime * animspeed >= ((acnt - 1) * 2 - 0.1f))
        //if (animtime * animspeed >= (acnt - 0.1f))
        //    jumpanim = false;
        auto  texRect = m_textureAtlas.AnimationRect(animtime * animspeed + 1, 6, acnt, false);

        Hazel::Renderer2D::DrawRotatedQuad({ px, py, z }, { width * wm + sign(width) * sw, height * hm + sh }, angle,
                                           m_textureAtlas.GetTextureRef().Get(),
                                           { texRect.z, texRect.w }, { texRect.x, texRect.y }, m_color);
    }
    else if (m_textureAtlas.Valid())
    {
        const float minxspd = 0.9f;
        bool klr = (key_left || key_right) && (speed > minxspd && abs(vel.x) > minxspd && abs(prev_vel.x) > minxspd && (!wallLeft && !wallRight));
        float animspeed = klr ? anim_move_speed : anim_idle_speed;

        auto texRect = m_textureAtlas.AnimationRect(animtime * animspeed, klr ? 5 : 0, klr ? -3 : 3, true);
        //texRect = textureAtlas.GetRect(6);

        float fangle = angle;
        //fangle = angle - width * anim_squish * 4;

        const float amd = 0.1f;
        wm = wm * (clamp01(1 - anim_squish) * amd + (1 - amd));
        hm = hm * (clamp01(anim_squish) * amd + 1.0f);
        px -= width * clamp01(anim_squish) * amd * 0.25f;


        Hazel::Renderer2D::DrawRotatedQuad({ px, py, z }, { width * wm + sign(width) * sw, height * hm + sh }, fangle,
                                           m_textureAtlas.GetTextureRef().Get(),
                                           { texRect.z, texRect.w }, { texRect.x, texRect.y }, m_color);
    }
    else
    {
        if (m_texture.Has())
            Hazel::Renderer2D::DrawRotatedQuad({ px, py, z }, { width * wm + sign(width) * sw, height * hm + sh }, angle,
                                               m_texture.Get(), m_texture_tiling, m_texture_offset, m_color);
        else
            Hazel::Renderer2D::DrawRotatedQuad({ px, py, z }, { width * wm + sign(width) * sw, height * hm + sh }, angle, m_color);
    }
}

#if DEBUG
void Character::DebugDraw() const
{
    auto cds = this->contacts;
    for (std::vector<struct ContactData>::iterator it = cds.begin(); it != cds.end(); ++it)
    {
        glm::vec4 dbgcolor2 = (*it).down ? glm::vec4(0, 1, 0, 1) : ((*it).up ? glm::vec4(1, 0, 0, 1) : glm::vec4(1, 1, 0, 1));
        DebugDraw::DrawRay((*it).pos, (*it).normal, dbgcolor2);
    }

    DebugDraw::DrawRay(this->GetPosition(), this->psc_normal, { 0,0,0,1 });
    DebugDraw::DrawLine(this->GetPosition({ 0.5f, 1 }), this->psc_pos, { 1,1,1,1 });
}
#endif
