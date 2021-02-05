#pragma once

#include "PhysicsManager.h"
#include "glm/vec2.hpp"
#include "glm/vec4.hpp"
#include "Hazel/Core/Base.h"
#include "Hazel/Renderer/Texture.h"
//#include "Hazel.h"
#include "Globals.h"
#include "TextureRef.h"

#define STRINGIFY(s) STRY(s)
#define STRY(s) #s

namespace Objects
{
    enum Type
    {
        Unknown = 0,

        Background,

        Player,

        Ground,
        Platform,

        Spike,
        SawBlade,

        Wall,

        Lava,

        Object,

        MAX_COUNT
    };

    static const char * EnumStrings[] = { STRY(Unknown), STRY(Background), STRY(Player),
         STRY(Platform), STRY(Wall), STRY(SawBlade), STRY(Spike), STRY(Lava) };
}

class GameObject
{
public:

    // Constructor/Destructor
    GameObject();
    GameObject(b2Body* bd, float w, float h, glm::vec4 color);
    GameObject(TextureRef tex, glm::vec2 pos, glm::vec2 size, glm::vec2 origin);
    GameObject(b2Body* bd, TextureRef tex, glm::vec2 size, glm::vec2 origin);
    virtual ~GameObject();

    b2Body* GameObject::GetBody() const { return m_body; }
    glm::vec2 GameObject::getPosition() const;
    glm::vec2 getPosition(glm::vec2 origin) const;

    virtual void Die();

    glm::vec4 GameObject::getColor() const { return clr; }
    void GameObject::setColor(glm::vec4 color) { this->clr = color; }
    void GameObject::setDrawLayer(int layer) { this->draw_layer = layer; }
    void GameObject::setTilingFactor(const glm::vec2& tilingFactor) { this->tex_tiling = tilingFactor; }
    void GameObject::setTilingOffset(const glm::vec2& offset) { this->tex_offset = offset; }
    glm::vec2 GameObject::getTilingFactor() const { return this->tex_tiling; }
    glm::vec2 GameObject::getTilingOffset() const { return this->tex_offset; }
    float GameObject::getHeight() const { return this->height; }
    float GameObject::getWidth() const { return this->width; }

protected:
    // setters
    void setBody(b2Body* bd)
    {
        m_body = bd;
        b2BodyUserData data;
        data.pointer = reinterpret_cast<uintptr_t>(this);
        m_body->SetUserData(data);
        posx = m_body->GetPosition().x * RATIO;
        posy = m_body->GetPosition().y * RATIO;
        angle = toDegrees(m_body->GetAngle());
    }

public:

    virtual void update(float time);

    virtual void draw(int layer);

    Objects::Type type = Objects::Unknown;

    bool dontDestroy;
    bool dontDraw;;

protected:
    static unsigned long instanceCount;
    unsigned long instanceID;

    // Member variables
    b2Body* m_body;
    float posx;
    float posy;
    float angle;
    float width;
    float height;
    glm::vec2 origin;

    bool hastex;

    glm::vec4 clr = { 1.0f, 1.0f, 1.0f, 1.0f };
    TextureRef tex;
    glm::vec2 tex_offset = { 0.0f, 0.0f };
    glm::vec2 tex_tiling = { 1.0f, 1.0f };
    int draw_layer = 0;



};
