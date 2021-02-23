#include "DebugDraw.h"

#include "Hazel/Renderer/Renderer2D.h"
#include <stdlib.h>
#include "Globals.h"

#define PZ 0.5f
#define LW 0.02f
#define PNTSIZE 0.05f

#define ALPHA 0.5f
#define CLR(color) { color.r, color.g, color.b, color.a }
#define CLRA(color) { color.r, color.g, color.b, color.a * ALPHA }

void DebugDraw::DrawRay(const glm::vec2 & pos, const glm::vec2 & dir, const glm::vec4 & color)
{
    auto len = std::max(0.01f, glm::length(dir));

    auto angleInRadians = std::atan2(dir.y / len, dir.x / len);
    auto angleInDegrees = (angleInRadians / PI) * 180.0f;

    auto v = pos + (dir * 0.5f);
    glm::vec2 size = { len, LW };

    Hazel::Renderer2D::DrawRotatedQuad({ v.x, v.y, PZ + 0.42f }, { size.x, size.y }, angleInDegrees, color);
    Hazel::Renderer2D::DrawRotatedQuad({ pos.x, pos.y, PZ + 0.43f }, { LW * 2, LW * 2 }, 0, color);
}

void DebugDraw::DrawLine(const glm::vec2 & a, const glm::vec2 & b, const glm::vec4 & color)
{
    auto dir = (b - a);
    auto len = std::max(0.01f, glm::distance(a, b));

    float angleInRadians = std::atan2(dir.y / len, dir.x / len);
    float angleInDegrees = (angleInRadians / PI) * 180.0f;

    auto v = (a + b) * 0.5f;
    glm::vec2 size = { len, LW };

    Hazel::Renderer2D::DrawRotatedQuad({ v.x, v.y, PZ + 0.22f }, { size.x, size.y }, angleInDegrees, color);
}

void DebugDraw::DrawLineRect(const glm::vec2 & a, const glm::vec2 & b, const glm::vec4 & color)
{
    auto dir = (b - a);
    auto v = (a + b) * 0.5f;
    glm::vec2 size = { dir.x, dir.y };

    Hazel::Renderer2D::DrawRotatedQuad({ v.x, v.y + size.y * .5f, PZ + 0.22f }, { size.x, LW }, 0, color);
    Hazel::Renderer2D::DrawRotatedQuad({ v.x, v.y - size.y * .5f, PZ + 0.22f }, { size.x, LW }, 0, color);
    Hazel::Renderer2D::DrawRotatedQuad({ v.x - size.x * .5f, v.y, PZ + 0.22f }, { LW, size.y }, 0, color);
    Hazel::Renderer2D::DrawRotatedQuad({ v.x + size.x * .5f, v.y, PZ + 0.22f }, { LW, size.y }, 0, color);
}

// ################################################################

void DrawLine(const b2Vec2 & p1, const b2Vec2 & p2, const b2Color & color)
{
    auto a = glm::vec2(p1.x, p1.y);
    auto b = glm::vec2(p2.x, p2.y);
    //auto len = std::max(0.01f, glm::distance(a, b));
    //for (float i = 0; i <= len; i += (PNTSIZE * 1.5f))
    //{
    //    auto v = a + ((b - a) / len * float(i));
    //    Hazel::Renderer2D::DrawQuad({ v.x * RATIO, v.y * RATIO, PZ + 0.2f }, { PNTSIZE, PNTSIZE }, CLR(color));
    //}
    DebugDraw::DrawLine(a* RATIO, b* RATIO, CLR(color));
}

void DrawCircle(const b2Vec2& center, float radius, const b2Vec2& axis, const b2Color& color)
{
    const int points = 8;
    b2Vec2 pv = center + (radius * 0.5f) * axis;
    double slice = 2 * PI / points;
    for (int i = 0; i < points; i++)
    {
        double angle = slice * i;
        b2Vec2 v = { (center.x + radius * (float)cos(angle)),
                     (center.y + radius * (float)sin(angle)) };

        ::DrawLine(pv, v, CLR(color));
        pv = v;
    }
}
void DrawRect(const int32 &vertexCount, const b2Vec2 * vertices, const b2Color & color)
{
    if (vertexCount == 4)
    {
        auto bl = vertices[0];
        auto tl = vertices[3];
        auto tr = vertices[2];
        auto br = vertices[1];

        b2Vec2 dx = br - bl;
        b2Vec2 dy = tl - bl;

        auto diffs = tr - tl;
        auto angleInRadians = std::atan2(diffs.y, diffs.x);
        auto angleInDegrees = (angleInRadians / PI) * 180.0f;

        float w = dx.Length();
        float h = dy.Length();

        auto p = glm::vec2((bl.x + tr.x) * .5f, (bl.y + tr.y) * .5f);

        Hazel::Renderer2D::DrawRotatedQuad({ p.x*RATIO, p.y*RATIO, PZ }, { w*RATIO, h*RATIO }, angleInDegrees, CLR(color));
    }
    else
    {
        b2Vec2 pv = vertices[vertexCount - 1];
        b2Vec2 sum = b2Vec2(0, 0);
        float w = 0;
        float h = 0;
        for (int i = 0; i < vertexCount; i++)
        {
            b2Vec2 v = vertices[i];
            sum += v;
            if (i > 0)
            {
                auto d = vertices[i - 1] - v;
                w = std::max(w, abs(d.x));
                h = std::max(h, abs(d.y));
            }
            //Hazel::Renderer2D::DrawQuad({ v.x*RATIO, v.y*RATIO }, { PNTSIZE, PNTSIZE }, CLR(color));
            DrawLine(pv, v, CLR(color));
            pv = v;
        }

        auto p = glm::vec2(sum.x / vertexCount, sum.y / vertexCount);
        Hazel::Renderer2D::DrawQuad({ p.x*RATIO, p.y*RATIO, PZ }, { w*RATIO, h*RATIO }, CLR(color));
    }
}

// ################################################################

void DebugDraw::DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
{
    if (m_BeginDraw != nullptr) m_BeginDraw();
    ::DrawRect(vertexCount, vertices, CLRA(color));
    if (m_EndDraw != nullptr) m_EndDraw();
}

void DebugDraw::DrawCircle(const b2Vec2& center, float radius, const b2Color& color)
{
    if (m_BeginDraw != nullptr) m_BeginDraw();
    ::DrawCircle(center, radius, { 0, 0 }, CLRA(color));
    if (m_EndDraw != nullptr) m_EndDraw();
}

void DebugDraw::DrawSolidCircle(const b2Vec2& center, float radius, const b2Vec2& axis, const b2Color& color)
{
    if (m_BeginDraw != nullptr) m_BeginDraw();
    ::DrawCircle(center, radius, axis, CLRA(color));
    if (m_EndDraw != nullptr) m_EndDraw();
}

void DebugDraw::DrawPoint(const b2Vec2& p, float size, const b2Color& color)
{
    size *= 0.02f;
    const float maxPntSize = 0.5f;
    if (size > maxPntSize)
        size = maxPntSize;
    if (m_BeginDraw != nullptr) m_BeginDraw();
    Hazel::Renderer2D::DrawQuad({ p.x*RATIO, p.y*RATIO, PZ + 0.1f }, { size*RATIO, size *RATIO }, CLRA(color));
    if (m_EndDraw != nullptr) m_EndDraw();
}

void DebugDraw::DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
{
    if (m_BeginDraw != nullptr) m_BeginDraw();
    ::DrawRect(vertexCount, vertices, CLRA(color));
    if (m_EndDraw != nullptr) m_EndDraw();
}

void DebugDraw::DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color)
{
    if (m_BeginDraw != nullptr) m_BeginDraw();
    ::DrawLine(p1, p2, CLRA(color));
    if (m_EndDraw != nullptr) m_EndDraw();
}

void DebugDraw::DrawTransform(const b2Transform& xf)
{
    b2Vec2 p1 = xf.p;
    const glm::float32 k_axisScale = 0.4f;

    if (m_BeginDraw != nullptr) m_BeginDraw();

    b2Vec2 p2 = p1 + k_axisScale * xf.q.GetXAxis();
    ::DrawLine(p1, p2, b2Color(1, 0, 0, ALPHA));

    p2 = p1 + k_axisScale * xf.q.GetYAxis();
    ::DrawLine(p1, p2, b2Color(0, 1, 0, ALPHA));

    if (m_EndDraw != nullptr) m_EndDraw();
}
