#pragma once

#include "box2d/b2_draw.h"
#include "box2d/b2_world.h"

#include "glm/vec2.hpp"
#include "glm/vec4.hpp"

#include <functional>

class DebugDraw : public b2Draw
{
public:

    DebugDraw(b2World* world) : m_World(world)
    {
        auto flags = b2Draw::e_shapeBit | b2Draw::e_jointBit | b2Draw::e_aabbBit | b2Draw::e_pairBit | b2Draw::e_centerOfMassBit;
        this->SetFlags(flags);
    }

    virtual ~DebugDraw() {}

    virtual void DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color) override;

    virtual void DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color) override;

    virtual void DrawTransform(const b2Transform& xf) override;

    virtual void DrawCircle(const b2Vec2& center, float radius, const b2Color& color) override;

    virtual void DrawSolidCircle(const b2Vec2& center, float radius, const b2Vec2& axis, const b2Color& color) override;

    virtual void DrawPoint(const b2Vec2& p, float size, const b2Color& color) override;

    virtual void DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color) override;


    void Draw() const { this->m_World->DebugDraw(); }

    void SetBeginDraw(const std::function<void()>& beginDraw) { m_BeginDraw = beginDraw; }
    void SetEndDraw(const std::function<void()>& endDraw) { m_EndDraw = endDraw; }
    bool HasBeginDraw() const { return m_BeginDraw != nullptr; }


    static void DrawRay(const glm::vec2 & pos, const glm::vec2 & dir, const glm::vec4 & color, float lineWidth = 0.025f);

    static void DrawLine(const glm::vec2& a, const glm::vec2& b, const glm::vec4& color, float lineWidth = 0.02f);

    static void DrawLineRect(const glm::vec2 & a, const glm::vec2 & b, const glm::vec4 & color, float lineWidth = 0.02f);

private:

    b2World* m_World = nullptr;

    //void(*m_BeginDraw)() = nullptr;
    //void(*m_EndDraw)() = nullptr;
    std::function<void()> m_BeginDraw = nullptr;
    std::function<void()> m_EndDraw = nullptr;

    static glm::vec4 RayQuadVertexPositions[4];
};
