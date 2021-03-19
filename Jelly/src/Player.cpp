#include "Player.h"
#include "Hazel/Core/Input.h"

using namespace Jelly;

Player::Player(float x, float y, float size, float scale, PhysicsManager* physicsMgr)
    : Character(x,y,size,scale,physicsMgr)
{
    PM_SCALE = 1.0f;
}

Player::~Player()
{
}

bool IsKeyPressed(Hazel::KeyCode key, Hazel::KeyCode alt)
{
    return Hazel::Input::IsKeyPressed(key)
        || Hazel::Input::IsKeyPressed(alt);
}

Character::Input Player::UpdateInput()
{
    if (Hazel::Input::BeginKeyPress(Hazel::Key::X))
        Explode();

    if (Hazel::Input::BeginKeyPress(Hazel::Key::K))
        Die();

    if (Hazel::Input::BeginKeyPress(Hazel::Key::Slash))
    {
        debugMode = !debugMode;
        //GetBody()->SetType(debugMode ? b2_kinematicBody : b2_dynamicBody);
        GetBody()->SetGravityScale(debugMode ? 0.f : 1.f);
    }

    if (debugMode)
    {
        b2Vec2 impulse = b2Vec2(0, 0);

        if (Hazel::Input::IsKeyPressed(Hazel::Key::Left))
            impulse.x += -1;
        if (Hazel::Input::IsKeyPressed(Hazel::Key::Right))
            impulse.x += 1;
        if (Hazel::Input::IsKeyPressed(Hazel::Key::Up))
            impulse.y += 1;
        if (Hazel::Input::IsKeyPressed(Hazel::Key::Down))
            impulse.y += -1;

        GetBody()->SetLinearVelocity(impulse);

        Input ninput = Input(false, false, false, false);
        ninput.update_move = false;
        return ninput;
    }

    Input input = Input(IsKeyPressed(Hazel::Key::Left, Hazel::Key::A),
                        IsKeyPressed(Hazel::Key::Right, Hazel::Key::D),
                        IsKeyPressed(Hazel::Key::Up, Hazel::Key::W),
                        IsKeyPressed(Hazel::Key::Down, Hazel::Key::S));
    input.update_move = true;
    return input;
}
