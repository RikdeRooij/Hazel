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

    TextureRef dummy = Hazel::Texture2D::Create("assets/pistol_03.png");
    weapon = Weapon(dummy, { posx, posy }, { 0.4f, 0.25f }, { 0.5f, 0.5f });
    weapon.SetColor({ 1.0f, 0.5f, 0.0f, 1.0f });
    weapon.Init(Objects::Object, 2);
    weapon.dontDraw = true;
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

void Jelly::Player::Update(float dt)
{
    Character::Update(dt);
    weapon.Update(dt);
    weapon.posx = posx - width * 0.4f;
    weapon.posy = posy;

    float ra = 0;
    if (Hazel::Input::IsKeyPressed(Hazel::Key::Q)) ra -= 1.0f;
    if (Hazel::Input::IsKeyPressed(Hazel::Key::E)) ra += 1.0f;
    aimangle += (ra * dt * 120 * sign(width));
    if (ra != 0)
        weapon.dontDraw = aimangle <= -45.f;
    aimangle = clamp(aimangle, -45.f, 90.f);
    
    weapon.height = -sign(width) * abs(weapon.height);
    weapon.angle = width < 0 ? aimangle : (180 - aimangle);

    float a = aimangle * DEG2RAD;
    if (width > 0)  a = PI - a;
    float r = abs(width) * 0.48f;
    weapon.posx = posx + r * (float)cos(a);
    weapon.posy = posy + r * (float)sin(a);

    bool fire = IsKeyPressed(Hazel::Key::Space, Hazel::Key::F);
    if (fire)
        weapon.Shoot(this, 12, 5.0f);

}

void Jelly::Player::Delete()
{
    weapon.Delete();
    Character::Delete();
}

void Jelly::Player::Draw(int layer) const
{
    if(layer == 0)
        Character::Draw(this->m_draw_layer);
    if (layer == 2)
        weapon.Draw(2);
}

Character::Input Player::UpdateInput()
{
    if (Hazel::Input::BeginKeyPress(Hazel::Key::L))
        Explode();

    if (Hazel::Input::BeginKeyPress(Hazel::Key::K))
        Die();

    if (Hazel::Input::BeginKeyPress(Hazel::Key::Slash))
    {
        debugMode = !debugMode;
        //GetBody()->SetType(debugMode ? b2_kinematicBody : b2_dynamicBody);
        GetBody()->SetGravityScale(debugMode ? 0.f : 1.f);
        GetBody()->SetLinearDamping(debugMode ? 8.f : 0.f);

        if (debugMode)
            this->SetColor({ 0.5f, 0.5f, 0.5f, 1.f });
        else
            this->SetColor({ 1.f, 1.f, 1.f, 1.f });
    }

    if (debugMode)
    {
        b2Vec2 impulse = b2Vec2(0, 0);
        float speed = 32.0f;

        if (Hazel::Input::IsKeyPressed(Hazel::Key::Left))
            impulse.x += -1;
        if (Hazel::Input::IsKeyPressed(Hazel::Key::Right))
            impulse.x += 1;
        if (Hazel::Input::IsKeyPressed(Hazel::Key::Up))
            impulse.y += 1;
        if (Hazel::Input::IsKeyPressed(Hazel::Key::Down))
            impulse.y += -1;

        if (Hazel::Input::IsKeyPressed(Hazel::Key::RightShift))
            speed *= 2.0;
        if (Hazel::Input::IsKeyPressed(Hazel::Key::LeftShift))
            speed *= 3.0;

        //GetBody()->SetLinearVelocity(8.f * impulse);
        GetBody()->SetLinearDamping(8.f);
        GetBody()->ApplyForceToCenter(speed * impulse, true);

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

void Jelly::Player::Die()
{
    if (debugMode)
        return;
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
