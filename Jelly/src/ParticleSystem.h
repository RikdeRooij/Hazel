#pragma once

#include "glm/vec2.hpp"
#include "glm/vec4.hpp"
#include <vector>
#include "TextureRef.h"

namespace Jelly
{
    struct ParticleProps
    {
        glm::vec2 Position;
        glm::vec2 Velocity, VelocityVariation;
        glm::vec4 ColorBegin, ColorEnd;
        Jelly::TextureRef Texture;
        float SizeBegin, SizeEnd, SizeVariation;
        float LifeTime = 1.0f;
    };

    class ParticleSystem
    {
    public:
        ParticleSystem(ParticleSystem const&) = delete;
        void operator=(ParticleSystem const&) = delete;

        //static ParticleSystem& GetInstance() { static ParticleSystem instance; return instance; }

        static ParticleSystem* instance;
        static ParticleSystem* GetInstance() { return instance; }

        ParticleSystem();
        ~ParticleSystem();

    public:
        void OnUpdate(float ts);
        void OnRender();

        void Emit(const ParticleProps& particleProps);

        void EmitParticles(int count, const ParticleProps& particleProps)
        {
            for (int i = 0; i < count; i++)
                Emit(particleProps);
        }

        static void S_Emit(const ParticleProps& particleProps) { if (instance) instance->Emit(particleProps); }
        static void S_Emit(int count, const ParticleProps& particleProps) { if (instance) instance->EmitParticles(count, particleProps); }

    private:
        struct Particle
        {
            glm::vec2 Position;
            glm::vec2 Velocity;
            glm::vec4 ColorBegin, ColorEnd;
            Jelly::TextureRef Texture;
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
}
