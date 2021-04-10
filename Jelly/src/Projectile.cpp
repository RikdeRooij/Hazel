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

bool Jelly::Projectile::OnBeginContact(GameObject* other, glm::vec2 normal)
{
    if (destroyed)
        return false;

    if (other == m_owner)
        return false;

    ParticleProps m_Particle;
    m_Particle.ColorBegin = { m_color.r, m_color.g, m_color.b, 1.0f };
    m_Particle.ColorEnd = { m_color.r, m_color.g, m_color.b, 0.1f };
    m_Particle.Texture = Hazel::Texture2D::Create("assets/particle.png");
    m_Particle.SizeBegin = 0.25f, m_Particle.SizeVariation = 0.2f, m_Particle.SizeEnd = 0.0f;
    m_Particle.LifeTime = 0.60f;
    m_Particle.Velocity = { 0.0f, 0.0f };
    m_Particle.VelocityVariation = { 1.5f, 1.5f };
    m_Particle.Position = { posx , posy };
    ParticleSystem::S_Emit(100, m_Particle);

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
        glm::vec2 pos = (*iter)->GetPosition({ 0.5f, 0.5f });
        glm::vec2 dp = mpos - pos;
        float dst = glm::length(dp);
        if (dst < 2.f)
        {
            float dstd = 1.f - (dst / 2.f);
            float power = -8.f * dstd;
            this->m_body->ApplyForceToCenter(b2Vec2(dp.x * power, dp.y * power), true);
        }
    }

    const float maxSpeed = 6.0f;
    b2Vec2 vel = m_body->GetLinearVelocity();
    float speed = vel.Normalize();//normalizes vector and returns length
    //if (speed > maxSpeed)
        m_body->SetLinearVelocity(maxSpeed * vel);
}

void Projectile::Start(GameObject* owner, float xvel, float yvel)
{
    m_owner = owner;
    m_body->SetLinearVelocity(b2Vec2(xvel, yvel));
}
