#include "Weapon.h"
#include "ObjectManager.h"

bool Weapon::Shoot(GameObject* owner, float speed, float addangle)
{
    if (dontDraw)
        return false;
    if (lastFireTime <= 0)
    {
        //float a = this->angle * DEG2RAD;
        float a2 = repeat(this->angle, 360.0f) - 180.0f;
        float dirf = abs(a2) < 90 ? -1.f : 1.f;
        float a = (this->angle + (dirf * addangle)) * DEG2RAD;
        auto ax = (float)cos(a);
        auto ay = (float)sin(a);

        float px = posx;
        float py = posy;

        // end of gun barrel
        px += cos((angle + dirf * 30) * DEG2RAD) * width * 0.4f;
        py += sin((angle + dirf * 30) * DEG2RAD) * width * 0.4f;

        lastFireTime = 0.3f;
        FixtureData fixtureDef = FixtureData::Category(FixtureData::PROJECTILE, (1 << Category::Projectile));
        fixtureDef.filter.maskBits = ~((1 << Category::Projectile) | (1 << Category::Player) | (1 << Category::Platform));
        auto fp = ObjectManager::CreateProjectile(px, py, 20, 20, angle, { 1, 0.2f, 0, 1 }, &fixtureDef);
        
        auto velx = speed * ax;
        auto vely = speed * ay;
        fp->Start(owner, velx, vely);
        return true;
    }
    return false;
}

void Weapon::Update(float dt)
{
    if (lastFireTime > 0)
        lastFireTime -= dt;
}

void Jelly::Weapon::Draw(glm::vec2 pos, glm::vec2 size, float angle) const
{
    const float power = 0.3f;
    const float addangle = 10.0f;
    float a2 = repeat(this->angle, 360.0f);
    float a = (this->angle + (a2 > 90 ? -addangle : addangle)) * DEG2RAD;
    auto ax = (float)cos(a);
    auto ay = (float)sin(a);
    pos.x -= ax * clamp01(lastFireTime) * power;
    pos.y -= ay * clamp01(lastFireTime) * power;
    GameObject::Draw(pos, size, angle);
}

void Jelly::Weapon::Draw(int layer) const
{
    if (dontDraw)
        return;
    GameObject::Draw(layer);
}
