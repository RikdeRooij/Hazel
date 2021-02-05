#include "Player.h"
#include "Hazel/Renderer/Renderer2D.h"
#include "Hazel/Core/Input.h"
#include "DebugDraw.h"

#define PM_SCALE 1.0f

#define MOVE_SIDE (3.0f * PM_SCALE) // Horizontal (left-right) movement power
#define MOVE_SIDE_LIMIT (4.0f * PM_SCALE) // Horizontal (left-right) movement limit

#define MOVE_JUMP_UP (8.0f * PM_SCALE) // jump power
#define MOVE_JUMP_TIME 200 // min time in milliseconds between jumps

#define MOVE_WALLJUMP_UP (5.0f * PM_SCALE) // wall-jump up power
#define MOVE_WALLJUMP_SIDE (6.0f * PM_SCALE) // wall-jump side (away from wall) power

#define JUMP_NOXMOVE_TIME 50.0f // miliseconds x movement is limited after jump

#define MOVE_FALLOFF 1.0f // speed at which move-power starts reducing
#define MOVE_FALLOFF_POWER 1.0f // scales reduction power on movement at high speed


#define MOVE_AIR 0.5f // amount of movement when not grounded
#define MOVE_AIR_MOMENTUM 3.0f // momentum (speed) needed to move when fully airborne (reducing move power when less)
#define MOVE_AIR_COLLDING 0.5f // amount of movement when not grounded but still touching a wall or ceiling


#define MOVE_DAMPING 3.0f // damping when grounded

#define GROUND_NORMAL 0.7f // max absolute normal.x to be considered ground. (acos(0.7) * (180/PI) = 45.6 degrees)


Player::Player(float x, float y, float size, float scale, PhysicsManager* physicsMgr)
{
    dontDestroy = true;

    b2BodyDef bodyDef;
    bodyDef.type = b2BodyType::b2_dynamicBody;
    bodyDef.position.Set(x*UNRATIO, y*UNRATIO);

    b2PolygonShape shape;
    shape.SetAsBox(size*0.5f*UNRATIO, size*0.5f*UNRATIO);

    b2FixtureDef fixtureDef = FixtureData(1.5f, 0.5f, 0.2f, "Player");
    fixtureDef.shape = &shape;

    b2Body* physBody = physicsMgr->getPhysicsWorld()->CreateBody(&bodyDef);
    b2BodyUserData data;
    data.pointer = reinterpret_cast<uintptr_t>(this);
    physBody->SetUserData(data);
    physBody->CreateFixture(&fixtureDef);
    physicsMgr->addPhysicsObject(physBody);

    physBody->SetFixedRotation(true);
    //physBody->SetLinearDamping(MOVE_DAMPING);
    physBody->SetLinearDamping(0);

    time = 0;


    m_body = physBody;


    //dontDestroy = false;
    dontDraw = false;

    this->tex = Hazel::Texture2D::Create("assets/Jelly.png");
    //glm::vec2 tsize = { tex.get()->GetWidth(), tex.get()->GetHeight() };
    width = size + 0.1f;// tsize.x * scale;
    height = size + 0.1f;//tsize.y * scale;

    posx = m_body->GetPosition().x * RATIO;
    posy = m_body->GetPosition().y * RATIO;
    angle = toDegrees(m_body->GetAngle());
    hastex = true;
    origin = { .5f, .5f };
    clr = { 1, 1, 1, 1 };
}

Player::~Player()
{

}

void Player::draw(int layer)
{
    if (dontDraw)
        return;
    GameObject::draw(layer);
}

void Player::update(float dt)
{
    time += dt * 1000;

    //b2Vec2 pos = getBody()->GetPosition();
    b2Vec2 vel = GetBody()->GetLinearVelocity();
    float speed = vel.Length();
    float mass = GetBody()->GetMass();

    grounded = false;
    wallLeft = false;
    wallRight = false;
    ceiling = false;
    inside = false;

#if DEBUG
    contacts.clear();
#endif

    for (b2ContactEdge* ce = GetBody()->GetContactList(); ce; ce = ce->next)
    {
        b2Contact* c = ce->contact;

        //DBG_WRITE("%s  ##  %s", ((char*)c->GetFixtureA()->GetUserData()), ((char*)c->GetFixtureA()->GetUserData()));
        //c->GetFixtureA()
        //if (c->GetFixtureA() == (*enemyBody)[i]->GetFixtureList())

        b2Manifold* manifold = c->GetManifold();
        if (manifold->pointCount < 0)
            continue;

        b2WorldManifold worldManifold;
        c->GetWorldManifold(&worldManifold);


        glm::vec2 avgPos(0, 0);
        glm::vec2 ctrDirVec(0, 0);

        if (manifold->pointCount > 0)
        {
            for (int i = 0; i < manifold->pointCount; i++)
            {
                glm::vec2 sfPos = { worldManifold.points[i].x * RATIO, worldManifold.points[i].y * RATIO };
                avgPos += sfPos;
            }
            avgPos = glm::vec2(avgPos.x / (float)manifold->pointCount, avgPos.y / (float)manifold->pointCount);
            ctrDirVec = avgPos - getPosition();
        }
        else
        {
            avgPos = getPosition();
            ctrDirVec = glm::vec2(0, 0);
        }

        b2Vec2 wmNormal = worldManifold.normal;
        auto normal = b2Vec2(abs(wmNormal.x) * ctrDirVec.x,
                               abs(wmNormal.y) * ctrDirVec.y);
        normal.Normalize();

        bool isGround = (normal.y < 0 && abs(normal.x) < GROUND_NORMAL);
        bool isLeft = normal.x <= -GROUND_NORMAL;
        bool isRight = normal.x >= GROUND_NORMAL;
        bool isUp = (normal.y > 0 && abs(normal.x) < GROUND_NORMAL);

        bool isInside = manifold->pointCount > 0 && glm::length(ctrDirVec) < 0.1f;

        if (isInside)
            inside = true;

        {
            if (isGround && !isInside)
                grounded = true;
            else
            {
                if (isLeft)
                    wallLeft = true;
                if (isRight)
                    wallRight = true;
                if (isUp && !isInside)
                    ceiling = true;
            }
        }

#if DEBUG
        ContactData cd = ContactData(avgPos, { normal.x, normal.y }, isGround, isUp);
        contacts.push_back(cd);
#endif

        //vel = clipVector(vel, normal, 0.5f);
    }



    if (Hazel::Input::BeginKeyPress(Hazel::Key::X))
        Explode();

    if (Hazel::Input::BeginKeyPress(Hazel::Key::K))
        Die();

    //getBody()->SetLinearVelocity(vel);
    //speed = vel.Length();


    bool airborne = inside ||
        (!grounded && !wallLeft && !wallRight && !ceiling);


    float damping = !inside && grounded ? MOVE_DAMPING : 0;
    GetBody()->SetLinearDamping(damping);



    float move_pwr_scale = grounded ? 1.0f :
        fmax(airborne ? 0 : MOVE_AIR_COLLDING,
             fmin(pow3(speed / MOVE_AIR_MOMENTUM) * MOVE_AIR, MOVE_AIR));

    float move_falloff = 1.0f + fmax(speed - MOVE_FALLOFF, 0.0f) * MOVE_FALLOFF_POWER;

    float power = (mass / move_falloff) * move_pwr_scale;

    bool allowJump =  !inside && vel.y < 3.1f && 
        (grounded || !ceiling || wallLeft || wallRight);
    float jumpDeltaTime = abs(time - lastJumpTime);
    if (allowJump && (jumpDeltaTime > MOVE_JUMP_TIME))
    {
        if (Hazel::Input::IsKeyPressed(Hazel::Key::Up)
            // || Hazel::Input::IsKeyPressed(Hazel::Key::W)
            )
        {
            float velup = clamp01(vel.y); // 1 when going up
            float veldn = clamp01(-vel.y); // 1 when going down


            float opposite = (1.0f - veldn * 0.8f); // smaller when going down


            if (grounded && vel.y < MOVE_JUMP_UP * 0.9f)
            {
                float extend = (1.0f + velup * 0.2f); // larger when going up

                Jump(vel.x, MOVE_JUMP_UP * opposite * extend);
                lastJumpTime = time;
                if (velup < 0.5f) PlayJumpSound(0);
                else PlayJumpSound(1);
            }
            else
            {
                float reduce = (1.0f - velup * 0.4f); // smaller when going up

                if (wallLeft)
                {
                    Jump(MOVE_WALLJUMP_SIDE * reduce, MOVE_WALLJUMP_UP * opposite);
                    lastJumpTime = time;
                    if (veldn < 0.5f) PlayJumpSound(0);
                    else PlayJumpSound(2);
                }
                if (wallRight)
                {
                    Jump(-MOVE_WALLJUMP_SIDE * reduce, MOVE_WALLJUMP_UP * opposite);
                    lastJumpTime = time;
                    if (veldn < 0.5f) PlayJumpSound(0);
                    else PlayJumpSound(2);
                }
            }
        }
    }


    if (Hazel::Input::IsKeyPressed(Hazel::Key::Left)
        // || Hazel::Input::IsKeyPressed(Hazel::Key::A)
        )
    {
        float limit = clamp01(MOVE_SIDE_LIMIT + vel.x) * clamp01(jumpDeltaTime / JUMP_NOXMOVE_TIME);
        moveX(-power * limit * MOVE_SIDE);
        this->width = std::abs(width);
    }

    if (Hazel::Input::IsKeyPressed(Hazel::Key::Right)
        // || Hazel::Input::IsKeyPressed(Hazel::Key::D)
        )
    {
        float limit = clamp01(MOVE_SIDE_LIMIT - vel.x) * clamp01(jumpDeltaTime / JUMP_NOXMOVE_TIME);
        moveX(power * limit * MOVE_SIDE);
        this->width = -std::abs(width);
    }


    if (Hazel::Input::IsKeyPressed(Hazel::Key::Down)
        // || Hazel::Input::IsKeyPressed(Hazel::Key::S)
        )
    {
        MoveDown();
    }

    if (dead && GetBody())
    {
        this->height = -std::abs(height);
        GetBody()->SetLinearVelocity(b2Vec2(0, 0));
        GetBody()->SetAwake(false);
    }


    GameObject::update(dt);
}


void Player::moveX(float power) const
{
    if (dead)
        return;

    b2Vec2 impulse = b2Vec2(power, 0);

    b2Vec2 impulsePoint = GetBody()->GetPosition();

    GetBody()->ApplyLinearImpulse(impulse, impulsePoint, true);
}


void Player::Jump(float x, float power) const
{
    if (dead)
        return;

    //PlayJumpSound();

    b2Vec2 impulse = b2Vec2(x, power);
    GetBody()->SetLinearVelocity(impulse);
}


void Player::MoveDown()
{
    if (dead)
        return;

    b2Vec2 vel = GetBody()->GetLinearVelocity();
    float power = 1.0f / fmax(vel.Length(), 1.0f);

    b2Vec2 impulse = b2Vec2(0, -GetBody()->GetMass() * MOVE_JUMP_UP * power * 0.1f);
    b2Vec2 impulsePoint = GetBody()->GetPosition();
    GetBody()->ApplyLinearImpulse(impulse, impulsePoint, true);
}



void Player::Explode()
{}

void Player::Die()
{
    if (dead)
        return;

    GetBody()->SetAwake(false);

    setColor({ 1,1,1,0 });

    Explode();

    dead = true;
}


