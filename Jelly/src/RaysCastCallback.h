#pragma once

#include "box2d/b2_world_callbacks.h"
#include "box2d/b2_math.h"
#include "GameObject.h"

class RaysCastCallback : public b2RayCastCallback
{
public:
    RaysCastCallback() : m_fixture(nullptr), m_fraction(0), m_hit(false)
    {
    }

    float ReportFixture(b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, float fraction) override
    {
        m_fixture = fixture;
        m_point = point;
        m_normal = normal;
        m_fraction = fraction;
        m_hit = fixture != nullptr && (fraction >= 0 && fraction < 1);
        if (m_ignore != Jelly::Objects::Unknown && m_hit)
        {
            auto go = GetHitGameObject();
            if (go && go->m_type == m_ignore)
            {
                m_hit = false;
                m_fraction = -1;
                return -1;
            }
        }
        return fraction;
    }

    b2Fixture* m_fixture;
    b2Vec2 m_point;
    b2Vec2 m_normal;
    float m_fraction;
    bool m_hit;
    Jelly::Objects::Type m_ignore = Jelly::Objects::Unknown;

    Jelly::GameObject* GetHitGameObject() const
    {
        if(m_hit && m_fixture && m_fixture->GetBody())
            return static_cast<Jelly::GameObject*>(reinterpret_cast<Jelly::GameObject*>(m_fixture->GetBody()->GetUserData().pointer));
        return nullptr;
    }

    explicit operator bool() const { return m_hit; }

    static RaysCastCallback RayCast(glm::vec2 from, glm::vec2 to, Jelly::Objects::Type ignore = Jelly::Objects::Unknown)
    {
        RaysCastCallback callback;
        callback.m_ignore = ignore;
        Jelly::PhysicsManager::GetPhysicsWorld()->RayCast(&callback, { from.x * UNRATIO, from.y * UNRATIO }, { to.x * UNRATIO,  to.y * UNRATIO });
        return callback;
    }

    static RaysCastCallback RayCastVec(glm::vec2 from, glm::vec2 vec, Jelly::Objects::Type ignore = Jelly::Objects::Unknown)
    {
        RaysCastCallback callback;
        callback.m_ignore = ignore;
        Jelly::PhysicsManager::GetPhysicsWorld()->RayCast(&callback, { from.x * UNRATIO, from.y * UNRATIO }, { (from.x + vec.x) * UNRATIO,  (from.y + vec.y) * UNRATIO });
        return callback;
    }
};
