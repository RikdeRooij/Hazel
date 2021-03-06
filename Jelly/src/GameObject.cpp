#include "GameObject.h"
#include "Hazel/Renderer/Renderer2D.h"
#include "ObjectManager.h"

using namespace Jelly;

unsigned long GameObject::instanceCount = 0;

GameObject::GameObject()
{
    instanceID = instanceCount++;
    //DBG_OUTPUT("GameObject CREATE %d", instanceID);
    dontDestroy = false;
    dontDraw = false;
    m_body = nullptr;
    prev_posx = posx = 0;
    prev_posy = posy = 0;
    prev_angle = angle = 0;
    width = 0;
    height = 0;
}

GameObject::GameObject(b2Body* bd, float w, float h, glm::vec4 color) : GameObject()
{
    m_type = Objects::Object;

    m_body = bd;
    b2BodyUserData data;
    data.pointer = reinterpret_cast<uintptr_t>(this);
    m_body->SetUserData(data);

    prev_posx = posx = m_body->GetPosition().x * RATIO;
    prev_posy = posy = m_body->GetPosition().y * RATIO;
    prev_angle = angle = toDegrees(m_body->GetAngle());

    width = w;
    height = h;

    this->m_origin = { 0.5f, 0.5f };
    this->m_color = color;
}

GameObject::GameObject(TextureRef tex, glm::vec2 pos, glm::vec2 size, glm::vec2 origin) : GameObject()
{
    m_body = nullptr;
    prev_posx = posx = pos.x;
    prev_posy = posy = pos.y;
    prev_angle = angle = 0;
    width = size.x;
    height = size.y;
    this->m_texture = tex;
    this->m_origin = origin;
    this->m_color = { 1, 1, 1, 1 };
}

GameObject::GameObject(b2Body* bd, TextureRef tex, glm::vec2 size, glm::vec2 origin) : GameObject()
{
    m_body = bd;
    b2BodyUserData data;
    data.pointer = reinterpret_cast<uintptr_t>(this);
    m_body->SetUserData(data);

    prev_posx = posx = m_body->GetPosition().x * RATIO;
    prev_posy = posy = m_body->GetPosition().y * RATIO;
    prev_angle = angle = toDegrees(m_body->GetAngle());

    width = size.x;
    height = size.y;
    this->m_texture = tex;
    this->m_origin = origin;
    this->m_color = { 1, 1, 1, 1 };
}

GameObject::~GameObject()
{
    //DBG_OUTPUT("GameObject DELETE %d", instanceID);
    if (destroyed)
    {
        ObjectManager::Remove(this);
        return;
    }
    destroyed = true;
    ObjectManager::Remove(this);
    m_body = nullptr;
}

void GameObject::Init(Objects::Type type, int layer)
{
    this->m_type = type;
    this->m_draw_layer = layer;
#if DEBUG
    //if (m_body)
    //    m_body->_debug = (Objects::EnumStrings[this->m_type]);
#endif
}

glm::vec2 GameObject::GetPosition() const
{
    //if(m_body) return glm::vec2(m_body->GetPosition().x * RATIO, m_body->GetPosition().y * RATIO);
    //return glm::vec2(posx, posy);
    float px = posx + abs(width) * (0.5f - m_origin.x);
    float py = posy + height * (0.5f - m_origin.y);
    return glm::vec2(px, py);
}

glm::vec2 GameObject::GetPosition(const glm::vec2 origin) const
{
    //float px = posx + abs(width) * (0.5f - m_origin.x);
    //float py = posy + height * (0.5f - m_origin.y);
    float px = posx + abs(width) * (0.5f - m_origin.x) + abs(width) * (0.5f - origin.x);
    float py = posy - height * (-1 + m_origin.y + origin.y);
    return glm::vec2(px, py);
}

void GameObject::Update(float dt)
{
    if (m_body)
    {
        if ((m_body->GetType() == b2_dynamicBody || m_body->GetType() == b2_kinematicBody))
        {
            prev_posx = posx;
            prev_posy = posy;
            prev_angle = angle;

            posx = m_body->GetPosition().x * RATIO;
            posy = m_body->GetPosition().y * RATIO;
            angle = toDegrees(m_body->GetAngle());

            //b2Vec2 pos = getBody()->GetPosition();
            prev_vel = vel;
            vel = GetBody()->GetLinearVelocity();
        }
    }

}

void GameObject::LateUpdate(float dt)
{
    if (!destroyed && destroy)
    {
        ObjectManager::Remove(this);
        return;
    }
}

// Draw the object on the given render target
void GameObject::Draw(int layer) const
{
    if (dontDraw)
        return;
    if (this->m_draw_layer != layer)
        return;

    //float px = posx - abs(width) * 0.5f + m_origin.x;
    //float py = posy - height * 0.5f + m_origin.y;

    float px = posx + abs(width) * (0.5f - m_origin.x);
    float py = posy + height * (0.5f - m_origin.y);

    Draw({ px, py }, { width, height }, angle);
}

void Jelly::GameObject::Draw(glm::vec2 pos, glm::vec2 size, float angle) const
{
    auto z = -0.99f + (static_cast<float>(m_type) / static_cast<float>(Objects::MAX_COUNT) * 0.5f);
    z += (instanceID * 0.000001f);
    if (m_texture.Has())
        Hazel::Renderer2D::DrawRotatedQuad({ pos.x, pos.y, z }, { size.x, size.y }, angle, m_texture.Get(), m_texture_tiling, m_texture_offset, m_color);
    else
        Hazel::Renderer2D::DrawRotatedQuad({ pos.x, pos.y, z }, { size.x, size.y }, angle, m_color);
    //Hazel::Renderer2D::DrawRotatedQuad({ posx, posy, z +0.001f }, { 0.2f, 0.2f }, angle, { 1,1,1,1 });
}

void Jelly::GameObject::Delete()
{
    if (destroy)
        return;
    if (!destroyed && m_body && m_body->GetWorld()->IsLocked())
    {
        destroy = true;
        return;
    }
    ObjectManager::Remove(this);
}
