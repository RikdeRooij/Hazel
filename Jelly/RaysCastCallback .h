#pragma once

#include "box2d/b2_world_callbacks.h"
#include "box2d/b2_math.h"

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
        m_hit = fraction > 0 && fixture != nullptr;
        return fraction;
    }

    b2Fixture* m_fixture;
    b2Vec2 m_point;
    b2Vec2 m_normal;
    float m_fraction;
    bool m_hit;

    explicit operator bool() const { return m_hit; }

    static RaysCastCallback RayCast(glm::vec2 from, glm::vec2 to)
    {
        RaysCastCallback callback;
        Jelly::PhysicsManager::GetPhysicsWorld()->RayCast(&callback, { from.x * UNRATIO, from.y * UNRATIO }, { to.x * UNRATIO,  to.y * UNRATIO });
        return callback;
    }
};
