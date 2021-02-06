#pragma once

#include "Hazel/Renderer/OrthographicCamera.h"
#include "Hazel/Core/Timestep.h"

#include "Hazel/Events/ApplicationEvent.h"
#include "Hazel/Events/MouseEvent.h"

namespace Hazel
{

    class OrthographicCameraController
    {
    public:
        virtual ~OrthographicCameraController() = default;
        OrthographicCameraController(float aspectRatio, bool rotation = false);

        virtual void OnUpdate(Timestep ts);
        virtual void OnEvent(Event& e);

        void OnResize(float width, float height);

        OrthographicCamera& GetCamera() { return m_Camera; }
        const OrthographicCamera& GetCamera() const { return m_Camera; }

        float GetZoomLevel() const { return m_ZoomLevel; }
        void SetZoomLevel(float level) { m_ZoomLevel = level; }

        glm::vec3 GetCameraPosition() const { return m_CameraPosition; }
        void SetCameraPosition(const glm::vec3& pos) { m_CameraPosition = pos; }

        float GetCameraRotation() const { return m_CameraRotation; }
        void SetCameraRotation(float rot)
        {
            m_CameraRotation = rot;
            if (m_CameraRotation > 180.0f)
                m_CameraRotation -= 360.0f;
            else if (m_CameraRotation <= -180.0f)
                m_CameraRotation += 360.0f;
            m_Camera.SetRotation(m_CameraRotation);
        }

        float GetScreenWidth() const { return m_ScreenWidth; }
        float GetScreenHeight() const { return m_ScreenHeight; }
        float GetAspectRatio() const { return m_AspectRatio; }

    protected:
        glm::vec3 m_CameraPosition = { 0.0f, 0.0f, 0.0f };

        float m_CameraRotation = 0.0f; //In degrees, in the anti-clockwise direction
        float m_CameraTranslationSpeed = 5.0f, m_CameraRotationSpeed = 180.0f;

        bool m_Rotation;

        OrthographicCamera m_Camera;

        float m_AspectRatio;
        float m_ZoomLevel = 1.0f;

        float m_ScreenWidth = 1;
        float m_ScreenHeight = 1;
    private:
        bool OnMouseScrolled(MouseScrolledEvent& e);
        bool OnWindowResized(WindowResizeEvent& e);
    };

}
