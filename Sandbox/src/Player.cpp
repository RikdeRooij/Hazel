#include "Player.h"
#include "Hazel/Renderer/Renderer2D.h"
#include "Hazel/Core/Input.h"
#include "DebugDraw.h"
#include "Sandbox2D.h"

//#define TEST 1

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


#define MOVE_AIR 0.25f // amount of movement when not grounded
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

    b2FixtureDef fixtureDef = FixtureData(1.0f, 0.2f, 0.45f, "Player");
    fixtureDef.shape = &shape;

    b2Body* physBody = physicsMgr->GetPhysicsWorld()->CreateBody(&bodyDef);
    b2BodyUserData data;
    data.pointer = reinterpret_cast<uintptr_t>(this);
    physBody->SetUserData(data);
    physBody->CreateFixture(&fixtureDef);
    physicsMgr->AddPhysicsObject(physBody);

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


    // Init here
    m_Particle.ColorBegin = { 0.0f, 0.9f, 0.0f, 1.0f };
    m_Particle.ColorEnd = { 0.0f, 0.8f, 0.0f, 0.1f };
    m_Particle.Texture = Hazel::Texture2D::Create("assets/particle.png");
    m_Particle.HasTexture = true;
    m_Particle.SizeBegin = 0.25f, m_Particle.SizeVariation = 0.2f, m_Particle.SizeEnd = 0.0f;
    m_Particle.LifeTime = 0.60f;
    m_Particle.Velocity = { 0.1f, 0.1f };
    m_Particle.VelocityVariation = { 2.5f, 2.5f };
    m_Particle.Position = { 0.0f, 0.0f };
}

Player::~Player()
{

}

void Player::Draw(int layer)
{
    if (dontDraw)
        return;
    GameObject::Draw(layer);
}

void Player::Update(float dt)
{
    time += dt * 1000;


    //b2Vec2 pos = getBody()->GetPosition();
    b2Vec2 vel = GetBody()->GetLinearVelocity();

    UpdateCollisions(vel);

    //getBody()->SetLinearVelocity(vel);
    speed = vel.Length();


    if (Hazel::Input::BeginKeyPress(Hazel::Key::X))
        Explode();

    if (Hazel::Input::BeginKeyPress(Hazel::Key::K))
        Die();


    float damping = !inside && grounded ? MOVE_DAMPING : 0;
    GetBody()->SetLinearDamping(damping);



    if (Hazel::Input::BeginKeyPress(Hazel::Key::Slash))
    {
        debugMode = !debugMode;
        //GetBody()->SetType(debugMode ? b2_kinematicBody : b2_dynamicBody);
        GetBody()->SetGravityScale(debugMode ? 0 : 1);
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
    }
    else
    {
        UpdateMove(vel);

        if (dead && GetBody())
        {
            this->height = -std::abs(height);
            GetBody()->SetLinearVelocity(b2Vec2(0, 0));
            GetBody()->SetAwake(false);
        }
    }

    GameObject::Update(dt);
}

void Player::UpdateMove(b2Vec2& vel)
{
    float mass = GetBody()->GetMass();

    bool airborne = inside ||
        (!grounded && !wallLeft && !wallRight && !ceiling);

    float move_pwr_scale = grounded ? 1.0f :
        fmax(airborne ? 0 : MOVE_AIR_COLLDING,
             fmin(pow3(fmin(1.0f, speed / MOVE_AIR_MOMENTUM)) * MOVE_AIR, MOVE_AIR));

    float move_falloff = 1.0f + fmax(speed - MOVE_FALLOFF, 0.0f) * MOVE_FALLOFF_POWER;
    //move_falloff = 1.0f;

    float power = (mass / move_falloff) * move_pwr_scale;

    // _JUMP vel.y:  0.0 | 3.4 | 4.2
    bool allowJump = !inside &&  vel.y < 4.5f &&
        (grounded || wallLeft || wallRight); // || !ceiling)

    float jumpDeltaTime = abs(time - lastJumpTime);
    if (allowJump && (jumpDeltaTime > MOVE_JUMP_TIME))
    {
        if (Hazel::Input::IsKeyPressed(Hazel::Key::Up)
            // || Hazel::Input::IsKeyPressed(Hazel::Key::W)
            )
        {
            //if(grounded) DBG_OUTPUT("_JUMP %.1f", vel.y);

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
#if TEST
        float limit = clamp01(MOVE_SIDE_LIMIT + vel.x) * clamp01(jumpDeltaTime / JUMP_NOXMOVE_TIME);
        moveX(-power * limit * MOVE_SIDE);
#else
        float opposite = !grounded ? 1.0f : 0.05f + pow2(clamp01(-(vel.x / MOVE_SIDE_LIMIT)));
        float limit = clamp01(MOVE_SIDE + vel.x) * clamp01(jumpDeltaTime / JUMP_NOXMOVE_TIME) * opposite;
        MoveX(-fmin(power * limit * MOVE_SIDE, MOVE_SIDE_LIMIT));
#endif
        this->width = std::abs(width);
    }

    if (Hazel::Input::IsKeyPressed(Hazel::Key::Right)
        // || Hazel::Input::IsKeyPressed(Hazel::Key::D)
        )
    {
#if TEST
        float limit = clamp01(MOVE_SIDE_LIMIT - vel.x) * clamp01(jumpDeltaTime / JUMP_NOXMOVE_TIME);
        moveX(power * limit * MOVE_SIDE);
#else
        float opposite = !grounded ? 1.0f : 0.05f + pow2(clamp01((vel.x / MOVE_SIDE_LIMIT)));
        float limit = clamp01(MOVE_SIDE - vel.x) * clamp01(jumpDeltaTime / JUMP_NOXMOVE_TIME) * opposite;
        MoveX(fmin(power * limit * MOVE_SIDE, MOVE_SIDE_LIMIT));
#endif
        this->width = -std::abs(width);
    }

    if (Hazel::Input::IsKeyPressed(Hazel::Key::Down)
        // || Hazel::Input::IsKeyPressed(Hazel::Key::S)
        )
    {
        Move( 0.f, -1.f );
    }
}

void Player::UpdateCollisions(b2Vec2& vel)
{
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
            ctrDirVec = avgPos - GetPosition();
        }
        else
        {
            avgPos = GetPosition();
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

        bool isInside = manifold->pointCount > 0 &&
            (glm::length(ctrDirVec) < 0.1f || 
            (abs(ctrDirVec.x) < 0.2f && abs(ctrDirVec.y) < 0.19f));

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
}

void Player::MoveX(float power) const
{
    if (dead)
        return;

#if TEST
    b2Vec2 impulse = b2Vec2(power, 0);
    b2Vec2 impulsePoint = GetBody()->GetPosition();
    GetBody()->ApplyLinearImpulse(impulse, impulsePoint, true);
#else
    //DBG_OUTPUT("moveX:: %.3f", (power));
    GetBody()->ApplyForceToCenter({ power * 100, 0 }, true);
#endif
}

void Player::Jump(float x, float power) const
{
    if (dead)
        return;

    //PlayJumpSound();

    b2Vec2 impulse = b2Vec2(x, power);
    GetBody()->SetLinearVelocity(impulse);
}

void Player::Move(float dx, float dy) const
{
    if (dead)
        return;

    b2Vec2 vel = GetBody()->GetLinearVelocity();
    float power = GetBody()->GetMass() * (1.0f / fmax(vel.Length(), 1.0f));

    b2Vec2 impulse = b2Vec2(dx * power, dy * power);
    b2Vec2 impulsePoint = GetBody()->GetPosition();
    GetBody()->ApplyLinearImpulse(impulse, impulsePoint, true);
}

void Player::Explode()
{
    Sandbox2D::sandbox2D->EmitParticles(posx, posy, 100, m_Particle);
}

void Player::Die()
{
    if (dead)
        return;

    GetBody()->SetAwake(false);

    SetColor({ 1,1,1,0 });

    Explode();

    dead = true;
}
