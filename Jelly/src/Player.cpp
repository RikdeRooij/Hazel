#include "Player.h"
#include "Hazel/Core/Input.h"
#include "AudioManager.h"
#include "JellyGame.h"

using namespace Jelly;

Player* Player::instance;

Player::Player(b2Body* bd, TextureAtlas textureRef, float w, float h, float scale)
    : Character(bd, textureRef, w, h, scale)
{
    PM_SCALE = 1.0f;
    dontDestroy = true;
    Player::instance = this;
}

Player::~Player()
{
    Player::instance = nullptr;
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
        GetBody()->SetLinearDamping(debugMode ? 8.f : 0.f);
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

        //GetBody()->SetLinearVelocity(8.f * impulse);
        GetBody()->SetLinearDamping(8.f);
        GetBody()->ApplyForceToCenter(32.f * impulse, true);

        Input ninput = Input(false, false, false, false);
        ninput.update_move = false;
        ninput.fire = IsKeyPressed(Hazel::Key::Space, Hazel::Key::F);
        return ninput;
    }

    Input input = Input(IsKeyPressed(Hazel::Key::Left, Hazel::Key::A),
                        IsKeyPressed(Hazel::Key::Right, Hazel::Key::D),
                        IsKeyPressed(Hazel::Key::Up, Hazel::Key::W),
                        IsKeyPressed(Hazel::Key::Down, Hazel::Key::S));
    input.update_move = true;
    input.fire = IsKeyPressed(Hazel::Key::Space, Hazel::Key::F);
    return input;
}

void Jelly::Player::Die()
{
    if (dead)
        return;
    Character::Die();
    //AudioManager::PlayFile("assets/Sounds/laser6.wav");
    AudioManager::PlaySoundType(Sounds::PlayerDie);
}

void Jelly::Player::OnHit(GameObject* by)
{
    JellyGame::ShakeScreen();
    AudioManager::PlaySoundType(Sounds::Hit);
}

bool Jelly::Player::OnCollision(b2Vec2 normal, GameObject* other)
{
    if (dead)
        return false;
    if (other && other->m_type == Objects::Enemy)
    {
        //DBG_OUTPUT("CC %.2f", normal.y);
        if (normal.y < -0.8f)
        {
            static_cast<Character*>(other)->OnHit(this);
            grounded = true;
            inside = false;
        }
    }
    return true;
}
