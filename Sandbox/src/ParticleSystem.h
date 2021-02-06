#pragma once

#include "glm/vec2.hpp"
#include "glm/vec4.hpp"
#include <vector>
#include "TextureRef.h"

struct ParticleProps
{
    glm::vec2 Position;
    glm::vec2 Velocity, VelocityVariation;
    glm::vec4 ColorBegin, ColorEnd;
    TextureRef Texture;
    bool HasTexture = false;
    float SizeBegin, SizeEnd, SizeVariation;
    float LifeTime = 1.0f;
};

class ParticleSystem
{
public:
    ParticleSystem();

    void OnUpdate(float ts);
    void OnRender();

    void Emit(const ParticleProps& particleProps);
private:
    struct Particle
    {
        glm::vec2 Position;
        glm::vec2 Velocity;
        glm::vec4 ColorBegin, ColorEnd;
        TextureRef Texture;
        bool HasTexture = false;
        float Rotation = 0.0f;
        float SizeBegin, SizeEnd;

        float LifeTime = 1.0f;
        float LifeRemaining = 0.0f;

        bool Active = false;
    };
    std::vector<Particle> m_ParticlePool;
    uint32_t m_PoolIndex = 999;
    uint32_t m_Count = 0;
};
