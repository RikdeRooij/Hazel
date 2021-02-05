#include "hzpch.h"
#include "Hazel/Core/Input.h"

#include "Hazel/Core/Application.h"
#include <GLFW/glfw3.h>

namespace Hazel {

    bool Input::IsKeyPressed(const KeyCode key)
    {
        auto* window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
        auto state = glfwGetKey(window, static_cast<int32_t>(key));
        return state == GLFW_PRESS || state == GLFW_REPEAT;
    }

    bool Input::BeginKeyPress(const KeyCode key)
    {
        auto* window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
        int32_t keyint = static_cast<int32_t>(key);
        int state = glfwGetKey(window, keyint) == GLFW_PRESS ? 1 : 0;

        static unsigned int keystates = 0x0;
        unsigned short ki = keyint & 0x1F;

        unsigned int b = ((keystates >> ki) & 0x01);
        keystates = keystates ^ ((state ^ b) << ki);

        return state == 1 && b == 0;
    }

    bool Input::IsMouseButtonPressed(const MouseCode button)
    {
        auto* window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
        auto state = glfwGetMouseButton(window, static_cast<int32_t>(button));
        return state == GLFW_PRESS;
    }

    bool Input::BeginMouseButtonPress(const MouseCode button)
    {
        auto* window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
        int32_t keyint = static_cast<int32_t>(button);
        int state = glfwGetMouseButton(window, keyint) == GLFW_PRESS ? 1 : 0;

        static unsigned int buttonstates = 0x0;
        unsigned short ki = keyint & 0x1F;

        unsigned int b = ((buttonstates >> ki) & 0x01);
        buttonstates = buttonstates ^ ((state ^ b) << ki);

        return state == 1 && b == 0;
    }

    glm::vec2 Input::GetMousePosition()
    {
        auto* window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetNativeWindow());
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);

        return { (float)xpos, (float)ypos };
    }

    float Input::GetMouseX()
    {
        return GetMousePosition().x;
    }

    float Input::GetMouseY()
    {
        return GetMousePosition().y;
    }

}
