#include <Hazel.h>
#include <Hazel/Core/EntryPoint.h>
#include "JellyGame.h"

class Sandbox : public Hazel::Application
{
public:
    Sandbox()
    {
        PushLayer(new Jelly::JellyGame());
    }

    ~Sandbox()
    {
    }
};

Hazel::Application* Hazel::CreateApplication(ApplicationCommandLineArgs args)
{
    return new Sandbox();
}
