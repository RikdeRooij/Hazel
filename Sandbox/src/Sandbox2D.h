#pragma once

#include "Hazel.h"
#include "GameObject.h"
#include "DebugDraw.h"
#include "ObjectManager.h"
#include "ParticleSystem.h"

class Sandbox2D;

class Sandbox2D : public Hazel::Layer
{
public:
    static Sandbox2D* sandbox2D;
    Sandbox2D();
    virtual ~Sandbox2D() = default;

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

    Hazel::OrthographicCameraController* GetCameraController() { return &m_CameraController; }

    void EmitParticles(float x, float y, int count) { EmitParticles(x, y, count, m_Particle); }
    void EmitParticles(float x, float y, int count, ParticleProps& particleProps)
    {
        particleProps.Position = { x , y };
        for (int i = 0; i < count; i++)
            m_ParticleSystem.Emit(particleProps);
    }

private:
    void beginDraw() { Hazel::Renderer2D::BeginScene(m_CameraController.GetCamera()); }
    void endDraw() const { Hazel::Renderer2D::EndScene(); }

    Hazel::OrthographicCameraController m_CameraController;
    float m_ScreenWidth = 1;
    float m_ScreenHeight = 1;
    float m_AspectRatio = 1;

    static ObjectManager* objectManager;
    DebugDraw* debugDraw;

    Player* player;
    GameObject* lava;

    long playerMaxY = 0;
    //long playerBestMaxY = 0;

    float deltaTime = 0;
    float dt_smooth = 0;

    ParticleProps m_Particle;
    ParticleSystem m_ParticleSystem;
};
