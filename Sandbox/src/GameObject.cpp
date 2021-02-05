
#include "GameObject.h"
#include "Hazel/Renderer/Renderer2D.h"

unsigned long GameObject::instanceCount = 0;

GameObject::GameObject()
{
    instanceID = instanceCount++;
    dontDestroy = false;
    dontDraw = false;
    m_body = nullptr;
    posx = 0;
    posy = 0;
    angle = 0;
    width = 0;
    height = 0;
    hastex = false;
}

GameObject::GameObject(b2Body* bd, float w, float h, glm::vec4 color) : GameObject()
{
    type = Objects::Object;

    m_body = bd;
    b2BodyUserData data;
    data.pointer = reinterpret_cast<uintptr_t>(this);
    m_body->SetUserData(data);

    posx = m_body->GetPosition().x * RATIO;
    posy = m_body->GetPosition().y * RATIO;
    angle = toDegrees(m_body->GetAngle());

    width = w;
    height = h;

    this->origin = { 0.5f, 0.5f };
    hastex = false;
    this->clr = color;
}

GameObject::GameObject(TextureRef tex, glm::vec2 pos, glm::vec2 size, glm::vec2 origin) : GameObject()
{
    m_body = nullptr;
    posx = pos.x;
    posy = pos.y;
    angle = 0;
    width = size.x;
    height = size.y;
    this->tex = tex;
    this->origin = origin;
    hastex = true;
    this->clr = { 1, 1, 1, 1 };
}

GameObject::GameObject(b2Body* bd, TextureRef tex, glm::vec2 size, glm::vec2 origin) : GameObject()
{
    m_body = bd;
    b2BodyUserData data;
    data.pointer = reinterpret_cast<uintptr_t>(this);
    m_body->SetUserData(data);

    posx = m_body->GetPosition().x * RATIO;
    posy = m_body->GetPosition().y * RATIO;
    angle = toDegrees(m_body->GetAngle());

    width = size.x;
    height = size.y;
    this->tex = tex;
    this->origin = origin;
    hastex = true;
    this->clr = { 1, 1, 1, 1 };
}

GameObject::~GameObject()
{
    m_body = nullptr;
}


//void GameObject::setBody(b2Body* bd) { m_body = bd; m_body->SetUserData(this); }


glm::vec2 GameObject::getPosition() const
{
    //if(m_body) return glm::vec2(m_body->GetPosition().x * RATIO, m_body->GetPosition().y * RATIO);
    //return glm::vec2(posx, posy);
    float px = posx + abs(width) * (0.5f - origin.x);
    float py = posy + height * (0.5f - origin.y);
    return glm::vec2(px, py);
}

glm::vec2 GameObject::getPosition(glm::vec2 origin) const
{
    //if(m_body) return glm::vec2(m_body->GetPosition().x * RATIO, m_body->GetPosition().y * RATIO);
    //return glm::vec2(posx, posy);
    float px = posx + abs(width) * (0.5f - origin.x);
    float py = posy + height * (0.5f - origin.y);
    return glm::vec2(px, py);
}

void GameObject::Die()
{}


void GameObject::update(float time)
{
    if (m_body && (m_body->GetType() == b2_dynamicBody || m_body->GetType() == b2_kinematicBody))
    {
        posx = m_body->GetPosition().x * RATIO;
        posy = m_body->GetPosition().y * RATIO;
        angle = toDegrees(m_body->GetAngle());
    }
}


// Draw the object on the given render target
void GameObject::draw(int layer)
{
    if (dontDraw)
        return;
    if (this->draw_layer != layer)
        return;

    //float px = posx - abs(width) * 0.5f + origin.x;
    //float py = posy - height * 0.5f + origin.y;

    float px = posx + abs(width) * (0.5f - origin.x);
    float py = posy + height * (0.5f - origin.y);


    auto z = -0.99f + (static_cast<float>(type) / static_cast<float>(Objects::MAX_COUNT) * 0.5f);
    z += (instanceID * 0.000001f);

    //renderTarget.draw(*m_drawable);
    if (hastex)
        Hazel::Renderer2D::DrawRotatedQuad({ px, py, z }, { width, height }, angle, tex.get(), tex_tiling, tex_offset, clr);
    else
        Hazel::Renderer2D::DrawRotatedQuad({ px, py, z }, { width, height }, angle, clr);
}
