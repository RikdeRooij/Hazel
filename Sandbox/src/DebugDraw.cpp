#include "DebugDraw.h"

#include "Hazel/Renderer/Renderer2D.h"
#include "glm/gtx/matrix_factorisation.hpp"
#include <stdlib.h>
#include "Globals.h"

#define PZ 0.5f
#define LW 0.02f

const float df = 0.05f;
const float alp = 0.3f;

void DrawRect(const int32 &vertexCount, const b2Vec2 * vertices, const b2Color & color)
{
    float minlen = 10;
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
            minlen = std::min(minlen, d.Length());
            w = std::max(w, abs(d.x));
            h = std::max(h, abs(d.y));
        }
        //Hazel::Renderer2D::DrawQuad({ v.x*RATIO, v.y*RATIO }, { df, df }, { color.r, color.g, color.b, color.a * alp });
    }

    auto p = glm::vec2(sum.x / vertexCount, sum.y / vertexCount);
    //Hazel::Renderer2D::DrawQuad({ p.x*RATIO, p.y*RATIO, PZ }, { minlen*RATIO, minlen*RATIO }, { color.r, color.g, color.b, color.a* alp });

    Hazel::Renderer2D::DrawQuad({ p.x*RATIO, p.y*RATIO, PZ }, { w*RATIO, h*RATIO }, { color.r, color.g, color.b, color.a * alp });
}

void DrawLine(const b2Vec2 & p1, const b2Vec2 & p2, const b2Color & color)
{
    auto a = glm::vec2(p1.x, p1.y);
    auto b = glm::vec2(p2.x, p2.y);
    auto len = std::max(0.01f, glm::distance(a, b));

    for (float i = 0; i <= len; i += (df * 1.5f))
    {
        auto v = a + ((b - a) / len * float(i));
        Hazel::Renderer2D::DrawQuad({ v.x * RATIO, v.y * RATIO, PZ + 0.2f }, { df, df }, { color.r, color.g, color.b, color.a * alp });
    }
    DebugDraw::DrawLine(a* RATIO, b* RATIO, { color.r, color.g, color.b, color.a  * alp });
}

void DebugDraw::DrawRay(const glm::vec2 & pos, const glm::vec2 & dir, const glm::vec4 & color)
{
    auto len = std::max(0.01f, glm::length(dir));
    
    auto angleInRadians = std::atan2(dir.y / len, dir.x / len);
    auto angleInDegrees = (angleInRadians / PI) * 180.0f;

    auto v = pos + (dir * 0.5f);
    glm::vec2 size = { len, LW };

    Hazel::Renderer2D::DrawRotatedQuad({ v.x, v.y, PZ + 0.42f }, { size.x, size.y }, angleInDegrees, { color.r, color.g, color.b, color.a });
    Hazel::Renderer2D::DrawRotatedQuad({ pos.x, pos.y, PZ + 0.43f }, { LW * 2, LW * 2 }, 0, { color.r, color.g, color.b, color.a });
}

void DebugDraw::DrawLine(const glm::vec2 & a, const glm::vec2 & b, const glm::vec4 & color)
{
    auto dir = (b - a);
    auto len = std::max(0.01f, glm::distance(a, b));

    float angleInRadians = std::atan2(dir.y / len, dir.x / len);
    float angleInDegrees = (angleInRadians / PI) * 180.0f;

    auto v = (a + b) * 0.5f;
    glm::vec2 size = { len, LW };

    Hazel::Renderer2D::DrawRotatedQuad({ v.x, v.y, PZ + 0.22f }, { size.x, size.y }, angleInDegrees, { color.r, color.g, color.b, color.a  });
}


void DebugDraw::DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
{
    if (m_BeginDraw != nullptr) m_BeginDraw();
    ::DrawRect(vertexCount, vertices, color);
    if (m_EndDraw != nullptr) m_EndDraw();
}

void DebugDraw::DrawCircle(const b2Vec2& center, float radius, const b2Color& color)
{
    //throw std::logic_error("The method or operation is not implemented.");
    //if (m_BeginDraw != nullptr) m_BeginDraw();
    //Hazel::Renderer2D::DrawQuad({ center.x*RATIO, center.y*RATIO, PZ }, { radius * .5f*RATIO, radius* .5f *RATIO }, { color.r, color.g, color.b, color.a* alp });
    //if (m_EndDraw != nullptr) m_EndDraw();
}

void DebugDraw::DrawSolidCircle(const b2Vec2& center, float radius, const b2Vec2& axis, const b2Color& color)
{
    //throw std::logic_error("The method or operation is not implemented.");
    if (m_BeginDraw != nullptr) m_BeginDraw();
    Hazel::Renderer2D::DrawQuad({ center.x*RATIO, center.y*RATIO, PZ }, { radius* .5f*RATIO, radius* .5f *RATIO }, { color.r, color.g, color.b, color.a* alp });
    if (m_EndDraw != nullptr) m_EndDraw();
}

void DebugDraw::DrawPoint(const b2Vec2& p, float size, const b2Color& color)
{
    if (size > 1.f)
        size = 1.f;
    if (m_BeginDraw != nullptr) m_BeginDraw();
    Hazel::Renderer2D::DrawQuad({ p.x*RATIO, p.y*RATIO, PZ + 0.1f }, { size*RATIO, size *RATIO }, { color.r, color.g, color.b, color.a* alp });
    if (m_EndDraw != nullptr) m_EndDraw();
}

void DebugDraw::DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
{
    if (m_BeginDraw != nullptr) m_BeginDraw();
    ::DrawRect(vertexCount, vertices, color);
    if (m_EndDraw != nullptr) m_EndDraw();
}

void DebugDraw::DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color)
{
    if (m_BeginDraw != nullptr) m_BeginDraw();
    ::DrawLine(p1, p2, color);
    if (m_EndDraw != nullptr) m_EndDraw();
}

void DebugDraw::DrawTransform(const b2Transform& xf)
{
    b2Vec2 p1 = xf.p;
    const glm::float32 k_axisScale = 0.4f;

    if (m_BeginDraw != nullptr) m_BeginDraw();

    b2Vec2 p2 = p1 + k_axisScale * xf.q.GetXAxis();
    ::DrawLine(p1, p2, b2Color(1, 0, 0));

    p2 = p1 + k_axisScale * xf.q.GetYAxis();
    ::DrawLine(p1, p2, b2Color(0, 1, 0));

    if (m_EndDraw != nullptr) m_EndDraw();
}
