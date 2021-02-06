#pragma once

#include <glm/glm.hpp>

#include "Hazel/Core/KeyCodes.h"
#include "Hazel/Core/MouseCodes.h"

namespace Hazel {

    class Input
    {
    public:
        enum KeyStateType
        {
            None = 0,
            BeginPress,
            Hold,
            Released
        };
        static KeyStateType GetKeyStateType(const KeyCode key);

        static bool IsKeyPressed(KeyCode key);
        static bool BeginKeyPress(KeyCode key);
        static bool ReleasedKeyPress(KeyCode key);

        static bool IsMouseButtonPressed(MouseCode button);
        static bool BeginMouseButtonPress(MouseCode button);

        static glm::vec2 GetMousePosition();
        static float GetMouseX();
        static float GetMouseY();

    };
}
