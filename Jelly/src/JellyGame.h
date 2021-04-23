#pragma once

#include "Hazel.h"
#include "GameObject.h"
#include "DebugDraw.h"
#include "ObjectManager.h"
#include "ParticleSystem.h"
#include "OrthographicCameraController.h"
#include "AudioManager.h"

namespace Jelly
{
    class JellyGame : public Hazel::Layer, private instance_holder<JellyGame*>
    {
    public:
        static JellyGame* GetInstance() { return instance; }

        JellyGame();
        virtual ~JellyGame();

        virtual void OnAttach() override;
        virtual void OnDetach() override;

        void StartGame();
        void DestroyGame() const;

        void OnUpdate(Hazel::Timestep ts) override;

        void UpdateGame(Hazel::Timestep& ts);
        void DrawGame(Hazel::Timestep &ts);

        virtual void OnImGuiRender() override;
        void OnEvent(Hazel::Event& e) override;
        bool OnWindowResized(Hazel::WindowResizeEvent& e);
        void OnResize(float width, float height);

        Player* GetPlayer() const { return player; }
        Hazel::OrthographicCameraController* GetCameraController() { return &m_CameraController; }
        /*
        void EmitParticles(float x, float y, int count) { EmitParticles(x, y, count, m_Particle); }
        void EmitParticles(float x, float y, int count, ParticleProps& particleProps)
        {
            particleProps.Position = { x , y };
            for (int i = 0; i < count; i++)
                m_ParticleSystem.Emit(particleProps);
        }
        */

        static void ShakeScreen()
        {
            auto x = GetInstance();
            if(x) x->screenShake = 0.15f;
        }

    private:
        void beginDraw() { Hazel::Renderer2D::BeginScene(m_CameraController.GetCamera()); }
        void endDraw() const { Hazel::Renderer2D::EndScene(); }

        ::Jelly::OrthographicCameraController m_CameraController;
        float m_ScreenWidth = 1;
        float m_ScreenHeight = 1;
        float m_AspectRatio = 1;

        static ObjectManager* objectManager;
        static AudioManager* audioManager;
        DebugDraw* debugDraw;

        Player* player;
        GameObject* lava;

        float center_x = 0;
        float camcenter_x = 0;
        float camcenter_y = 0;

        ulong playerScoreY = 0;
        ulong playerScoreMaxY = 0;
        ulong playerScoreBestMaxY = 0;
        ulong playerScoreBestMaxY_prev = 0;

        bool startedMove = false;
        bool newBest = false;
        clock_t clockStart;
        float dt_next = 0;
        unsigned int avg_counter = 0;
        float avg_fps = 0;
        float screenShake = 0;

        float blackFadeAlpha = 1.0f;

        ParticleProps lavaParticle;
        ParticleSystem m_ParticleSystem;
    };
}
