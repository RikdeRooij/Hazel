#include "Projectile.h"
#include "Character.h"
#include "ObjectManager.h"
#include "glm/detail/func_geometric.inl"

using namespace Jelly;

Jelly::Projectile::Projectile(b2Body * bd, TextureRef tex, float w, float h, glm::vec4 color)
    : GameObject(bd, tex, {w, h}, {.5f, .5f}), m_owner(nullptr)
{
    this->m_color = color;
    m_body->SetGravityScale(0.2f);
    m_body->SetBullet(true);
    m_body->SetLinearDamping(0.1f);
}

void EmitParticles(float posx, float posy, glm::vec4 color, int count, float v)
{
    ParticleProps m_Particle;
    m_Particle.ColorBegin = { color.r, color.g, color.b, color.a * 1.0f };
    m_Particle.ColorEnd = { color.r, color.g, color.b, color.a * 0.1f };
    m_Particle.Texture = Hazel::Texture2D::Create("assets/particle.png");
    m_Particle.SizeBegin = 0.25f, m_Particle.SizeVariation = 0.2f, m_Particle.SizeEnd = 0.0f;
    m_Particle.LifeTime = 0.60f;
    m_Particle.Velocity = { 0.0f, 0.0f };
    m_Particle.VelocityVariation = { 1.5f * v, 1.5f * v };
    m_Particle.Position = { posx , posy };
    ParticleSystem::S_Emit(count, m_Particle);
}

bool Jelly::Projectile::OnBeginContact(GameObject* other, b2Fixture* fixture, glm::vec2 pos, glm::vec2 normal)
{
    if (destroyed)
        return false;

    if (other == m_owner)
        return false;

    if (hitcount == 0)
    {
        hitcount++;
        bool isEnemy = (fixture->GetFilterData().categoryBits & (1 << Category::Enemy)) != 0;
        if (!isEnemy)
        {
            b2Vec2 vel = m_body->GetLinearVelocity();
            b2Vec2 nrm = { normal.x, normal.y };
            float dt = b2Dot(nrm, vel);
            vel -= 2 * (dt * nrm);
            m_body->SetLinearVelocity({ vel.x, vel.y });
            //velSpeedLerp *= 0.3f;
            m_body->SetGravityScale(0.6f);

            EmitParticles(pos.x, pos.y, m_color, 10, .3f);

            return true;
        }
    }

    EmitParticles(posx, posy, m_color, 100, 1.0f);

    Character* charact = dynamic_cast<Character*>(other);
    if (charact != nullptr)
    {
        charact->OnHit(this);
    }

    this->Delete();

    return true;
}

void Jelly::Projectile::LateUpdate(float dt)
{
    GameObject::LateUpdate(dt);
    if (destroyed)
        return;

    lifetime -= dt;
    if (lifetime <= 0)
    {
        this->Delete();
        return;
    }

    auto mpos = GetPosition();

    auto objectList = ObjectManager::GetInstance()->GetObjectList();
    for (std::list<GameObject*>::iterator iter = objectList.begin(); iter != objectList.end(); ++iter)
    {
        if ((*iter) == this)
            continue;
        if ((*iter) == m_owner)
            continue;
        if (!(*iter)->IsCharacter())
            continue;
        if ((dynamic_cast<Character*>(*iter))->dead)
            continue;
        glm::vec2 pos = (*iter)->GetPosition({ 0.5f, 0.5f });
        glm::vec2 dp = mpos - pos;
        float dst = glm::length(dp);
        if (dst < 3.f)
        {
            float dstd = 1.f - (dst / 3.f);
            float power = -8.f * dstd;
            this->m_body->ApplyForceToCenter(b2Vec2(dp.x * power, dp.y * power), true);
        }
    }

    const float maxSpeed = 6.0f;
    b2Vec2 vel = m_body->GetLinearVelocity();
    float cvelx = vel.x;
    float cvely = vel.y;

    float speed = vel.Normalize();//normalizes vector and returns length
    //if (speed > maxSpeed)
    //    m_body->SetLinearVelocity(maxSpeed * vel);

    cvelx = Interpolate::Linearf(cvelx, maxSpeed * vel.x, velSpeedLerp);
    cvely = Interpolate::Linearf(cvely, maxSpeed * vel.y, velSpeedLerp);
    m_body->SetLinearVelocity({ cvelx, cvely });
}

void Projectile::Start(GameObject* owner, float xvel, float yvel)
{
    m_owner = owner;
    m_body->SetLinearVelocity(b2Vec2(xvel, yvel));
}
