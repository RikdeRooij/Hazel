#pragma once

#include "PhysicsManager.h"
#include "glm/vec2.hpp"
#include "glm/vec4.hpp"
#include "Hazel/Core/Base.h"

#include "Globals.h"
#include "TextureRef.h"

namespace Jelly
{
    namespace Objects
    {
        enum Type : int8
        {
            Unknown = 0,

            Background,

            Player,
            Enemy,
            Object,

            Ground,
            Platform,

            Spike,
            SawBlade,

            Wall,

            Lava,

            MAX_COUNT
        };

        static const char * EnumStrings[] = {
            STRY(Unknown), STRY(Background),
            STRY(Player), STRY(Object),
            STRY(Enemy),
            STRY(Ground), STRY(Platform),
            STRY(Spike), STRY(SawBlade),
            STRY(Wall), STRY(Lava), STRY(MAX_COUNT) };
    }

    namespace Category
    {
        enum Type : int8
        {
            Unknown = 0,

            Player,
            Enemy,

            World,
            Interactive,

            MAX_COUNT = 8
        };

        static const char * EnumStrings[] = {
            STRY(Unknown), STRY(Player), STRY(Enemy),
            STRY(World), STRY(Interactive), STRY(MAX_COUNT) };
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

        void Init(Objects::Type type, int layer);

        b2Body* GameObject::GetBody() const { return m_body; }
        glm::vec2 GameObject::GetPosition() const;
        glm::vec2 GetPosition(const glm::vec2 origin) const;

        float GameObject::GetHeight() const { return this->height; }
        float GameObject::GetWidth() const { return this->width; }
        glm::vec2 GameObject::GetSize() const { return  glm::vec2(this->width, this->height); }

        glm::vec4 GameObject::GetColor() const { return m_color; }
        void GameObject::SetColor(const glm::vec4 color) { this->m_color = color; }

        void GameObject::SetTilingFactor(const glm::vec2& tilingFactor) { this->m_texture_tiling = tilingFactor; }
        void GameObject::SetTilingOffset(const glm::vec2& offset) { this->m_texture_offset = offset; }
        glm::vec2 GameObject::GetTilingFactor() const { return this->m_texture_tiling; }
        glm::vec2 GameObject::GetTilingOffset() const { return this->m_texture_offset; }

    protected:
        // setters
        void SetBody(b2Body* bd)
        {
            m_body = bd;
            if (m_body)
            {
                b2BodyUserData data;
                data.pointer = reinterpret_cast<uintptr_t>(this);
                m_body->SetUserData(data);
                posx = m_body->GetPosition().x * RATIO;
                posy = m_body->GetPosition().y * RATIO;
                angle = toDegrees(m_body->GetAngle());
            }
        }

    public:

        virtual void Update(float dt);

        virtual void Draw(int layer) const;

#if DEBUG
        virtual void DebugDraw() const {}
#endif

        Objects::Type m_type = Objects::Unknown;

        bool dontDestroy;
        bool dontDraw;;

        bool debugMode = false;
        bool isCharacter = false;

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
        glm::vec2 m_origin;

        glm::vec4 m_color = { 1.0f, 1.0f, 1.0f, 1.0f };
        TextureRef m_texture;
        glm::vec2 m_texture_offset = { 0.0f, 0.0f };
        glm::vec2 m_texture_tiling = { 1.0f, 1.0f };
        int m_draw_layer = 0;

    public:
        bool destroyed = false;
    };
}
