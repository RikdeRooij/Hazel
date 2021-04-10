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
        // in render order
        enum Type : int8
        {
            Unknown = 0,

            Background,

            Player,
            Enemy,
            Object,

            Ground,
            Platform,

            SawBlade,

            Projectile,
            Wall,

            Spike,

            Lava,

            MAX_COUNT
        };

        static const char * EnumStrings[] = {
            STRY(Unknown), STRY(Background),
            STRY(Player),
            STRY(Enemy),
            STRY(Object),
            STRY(Ground), STRY(Platform),
            STRY(SawBlade),
            STRY(Projectile),
            STRY(Wall),
            STRY(Spike), 
            STRY(Lava), 
            STRY(MAX_COUNT) };
    }

    namespace Category
    {
        enum Type : int8
        {
            Unknown = 0,

            Player,
            Enemy,

            World,
            Platform,
            Interactive,
            Projectile,

            MAX_COUNT = 8
        };

        static const char * EnumStrings[] = {
            STRY(Unknown), STRY(Player), STRY(Enemy),
            STRY(World), STRY(Platform), STRY(Interactive), STRY(Projectile), STRY(MAX_COUNT) };
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

        virtual void LateUpdate(float dt);

        virtual void Draw(int layer) const;

        virtual bool OnBeginContact(GameObject* other, glm::vec2 normal) { return true; }
        virtual void OnEndContact(GameObject* other) {}
        virtual void Delete();

#if DEBUG
        virtual void DebugDraw() const {}
#endif

        bool IsCharacter() const { return m_type == Objects::Player || m_type == Objects::Enemy; }

        Objects::Type m_type = Objects::Unknown;

        bool dontDestroy;
        bool dontDraw;;

        bool debugMode = false;

        b2Vec2 vel = b2Vec2(0.f, 0.f);
        b2Vec2 prev_vel = b2Vec2(0.f, 0.f);
        float posx;
        float posy;
        float angle;
        float prev_posx;
        float prev_posy;
        float prev_angle;

    protected:
        static unsigned long instanceCount;
        unsigned long instanceID;

        // Member variables
        b2Body* m_body;
        float width;
        float height;
        glm::vec2 m_origin;

        glm::vec4 m_color = { 1.0f, 1.0f, 1.0f, 1.0f };
        TextureRef m_texture;
        glm::vec2 m_texture_offset = { 0.0f, 0.0f };
        glm::vec2 m_texture_tiling = { 1.0f, 1.0f };
        int m_draw_layer = 0;

        friend class PhysicsManager;

    public:
        bool destroyed = false;
        bool destroy = false;

    private:
        friend class ObjectManager;
    };
}
