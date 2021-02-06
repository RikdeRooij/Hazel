#pragma once

#include "Hazel/Renderer/OrthographicCamera.h"
#include "Hazel/Renderer/OrthographicCameraController.h"
#include "Hazel/Core/Input.h"

namespace Jelly
{
    class OrthographicCameraController : public Hazel::OrthographicCameraController
    {
    public:
        OrthographicCameraController(float aspectRatio, bool rotation = false)
            : Hazel::OrthographicCameraController(aspectRatio, rotation)
        {
            m_CameraRotationSpeed = 45.0f;
        }

        void OnUpdate(Hazel::Timestep ts) override
        {
            HZ_PROFILE_FUNCTION();

            if (Hazel::Input::IsKeyPressed(Hazel::Key::KP4))
            {
                m_CameraPosition.x -= cos(glm::radians(m_CameraRotation)) * m_CameraTranslationSpeed * ts;
                m_CameraPosition.y -= sin(glm::radians(m_CameraRotation)) * m_CameraTranslationSpeed * ts;
            }
            else if (Hazel::Input::IsKeyPressed(Hazel::Key::KP6))
            {
                m_CameraPosition.x += cos(glm::radians(m_CameraRotation)) * m_CameraTranslationSpeed * ts;
                m_CameraPosition.y += sin(glm::radians(m_CameraRotation)) * m_CameraTranslationSpeed * ts;
            }

            if (Hazel::Input::IsKeyPressed(Hazel::Key::KP8))
            {
                m_CameraPosition.x += -sin(glm::radians(m_CameraRotation)) * m_CameraTranslationSpeed * ts;
                m_CameraPosition.y += cos(glm::radians(m_CameraRotation)) * m_CameraTranslationSpeed * ts;
            }
            else if (Hazel::Input::IsKeyPressed(Hazel::Key::KP2))
            {
                m_CameraPosition.x -= -sin(glm::radians(m_CameraRotation)) * m_CameraTranslationSpeed * ts;
                m_CameraPosition.y -= cos(glm::radians(m_CameraRotation)) * m_CameraTranslationSpeed * ts;
            }

            if (m_Rotation)
            {
                if (Hazel::Input::IsKeyPressed(Hazel::Key::KP7))
                    m_CameraRotation += m_CameraRotationSpeed * ts;
                if (Hazel::Input::IsKeyPressed(Hazel::Key::KP9))
                    m_CameraRotation -= m_CameraRotationSpeed * ts;

                if (m_CameraRotation > 180.0f)
                    m_CameraRotation -= 360.0f;
                else if (m_CameraRotation <= -180.0f)
                    m_CameraRotation += 360.0f;

                m_Camera.SetRotation(m_CameraRotation);
            }

            m_Camera.SetPosition(m_CameraPosition);

            m_CameraTranslationSpeed = m_ZoomLevel;
        }
    };

}
