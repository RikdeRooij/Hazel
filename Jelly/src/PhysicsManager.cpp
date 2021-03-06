#include "PhysicsManager.h"
#include "Globals.h"
#include "GameObject.h"
#include "Character.h"

//#include <sstream>     // std::basic_stringstream, std::basic_istringstream, std::basic_ostringstream class templates and several typedefs 

using namespace Jelly;

// ----------------

// Predefined fixtures type.
#define FD_TYPE FixtureData

// FIXTUREDEF(density, friction, restitution, name);
#define FIXTUREDEF(d, f, r, n) const FD_TYPE (FixtureData::##n) = FD_TYPE(d, f, r, #n)

// Predefined fixtures. (density, friction, restitution, name)
FIXTUREDEF(0.00f, 0.20f, 0.00f, BOX2D);
FIXTUREDEF(1.00f, 0.30f, 0.10f, DEFAULT);
FIXTUREDEF(7.85f, 0.20f, 0.20f, METAL);
FIXTUREDEF(2.40f, 0.50f, 0.10f, STONE);
FIXTUREDEF(0.53f, 0.40f, 0.15f, WOOD);
FIXTUREDEF(2.50f, 0.10f, 0.20f, GLASS);
FIXTUREDEF(1.50f, 0.80f, 0.40f, RUBBER);
FIXTUREDEF(0.92f, 0.01f, 0.10f, ICE);
FIXTUREDEF(0.25f, 0.60f, 0.00f, PUMICE);
FIXTUREDEF(0.10f, 0.60f, 0.05f, POLYSTYRENE);
FIXTUREDEF(0.03f, 0.60f, 0.10f, FABRIC);
FIXTUREDEF(0.018f, 0.90f, 0.05f, SPONGE);
FIXTUREDEF(0.001f, 0.90f, 0.00f, AIR);
FIXTUREDEF(0.0001f, 0.9f, 0.00f, HELIUM);

FIXTUREDEF(0.50f, 0.10f, 0.45f, TEST);

FIXTUREDEF(5.00f, 0.30f, 0.10f, PROJECTILE);
FIXTUREDEF(2.00f, 0.50f, 0.10f, LEVEL);

const FixtureData FixtureData::SENSOR = FixtureData(true, {}, "SENSOR");

// FIXTUREDEF(density, friction, restitution, name);
//#define FIXTUREDEF(d, f, r, n) const FD_TYPE (FixtureData::##n) = \
//	(const_cast<FD_TYPE&>(n) = FD_TYPE(), const_cast<FD_TYPE&>(n).userData = #n, \
//	const_cast<FD_TYPE&>(n).density = d, const_cast<FD_TYPE&>(n).friction = f, const_cast<FD_TYPE&>(n).restitution = r, n)

// --------------------------------

// re-declare statics
b2World* PhysicsManager::physicsWorld = nullptr;
//std::list<b2Body*> PhysicsManager::physicsObjectList;

// Constructor
PhysicsManager::PhysicsManager()
{
    // create and set physics world.
    if (physicsWorld == nullptr)
    {
        physicsWorld = new b2World(b2Vec2(0.0f, P_GRAVITY));
        physicsWorld->SetAllowSleeping(true);
        physicsWorld->SetAllowSleeping(false);
    }

    // set variables
    if (this->physicsObjectList.empty())
        this->physicsObjectList = std::list<b2Body*>();
    //	std::list<b2Body*> physicsObjectList();
}

// Destructor
PhysicsManager::~PhysicsManager()
{
    //std::for_each(PhysicsManager::physicsObjectList.begin(), PhysicsManager::physicsObjectList.end(), Delete());
    PhysicsManager::physicsObjectList.clear();
    delete physicsWorld;
    physicsWorld = nullptr;
}

void PhysicsManager::AddPhysicsObject(b2Body* body)
{
    this->physicsObjectList.push_back(body);
}

#define NUM_SEGMENTS 12
#define OUT_RADIUS 0.4f
#define IN_RADIUS 0.4f

#define SPRINGINESS 8.0f // Specifies the mass-spring damping frequency in Hz. A low value will make the joint extremely soft and cause it to contract with very low force.
#define DAMPING 0.5f //  This value can range from 0 (no damping) to 1 (critical damping). With critical damping, all oscillations should vanish.

float SqrCircle(float f) // (f from -1 to 1)
{
    if (f < 0) f = -f;
    float pf = f * f;
    float rf = (1.0f - (pf * 0.5f + pf * f * 0.5f));
    return rf <= 0 ? 0 : rf >= 1 ? 1 : rf;
}

b2Body* PhysicsManager::CreatePhysicsObject(float x, float y, float size)
{
    b2Vec2 center = b2Vec2(x * UNRATIO, y * UNRATIO);

    float sradius = size * UNRATIO;
    float deltaAngle = (2.f * PI) / NUM_SEGMENTS;


    b2CircleShape circleShape;
    circleShape.m_radius = deltaAngle * sradius * OUT_RADIUS;

    b2FixtureDef fixtureDef;
    fixtureDef.shape = &circleShape;
    fixtureDef.density = 1.0f;
    fixtureDef.restitution = 0.05f;
    fixtureDef.friction = 1.0f;


    b2Body* bodies[NUM_SEGMENTS];

    for (int i = 0; i < NUM_SEGMENTS; i++)
    {
        float theta = deltaAngle * i;

        float tmp_x = cosf(theta);
        float tmp_y = sinf(theta);
        float tmp_s = SqrCircle(tmp_x) + SqrCircle(tmp_y);

        b2Vec2 circlePosition = b2Vec2(tmp_x * tmp_s * sradius, tmp_y * tmp_s * sradius);

        b2BodyDef bodyDef;
        bodyDef.type = b2_dynamicBody;
        bodyDef.position = (center + circlePosition);

        b2Body* body = physicsWorld->CreateBody(&bodyDef);
        body->CreateFixture(&fixtureDef);
        body->SetFixedRotation(true);

        bodies[i] = body;
    }

    b2BodyDef innerCircleBodyDef;
    innerCircleBodyDef.type = b2_dynamicBody;
    innerCircleBodyDef.position = center;

    circleShape.m_radius = (sradius - circleShape.m_radius) * IN_RADIUS;
    fixtureDef.shape = &circleShape;

    b2Body* innerCircleBody = physicsWorld->CreateBody(&innerCircleBodyDef);
    innerCircleBody->CreateFixture(&fixtureDef);
    innerCircleBody->SetFixedRotation(true);

    b2DistanceJointDef jointDef;
    b2PrismaticJointDef prisJointDef;
    //b2Vec2 worldAxis(1.0f, 0.0f);

    for (int i = 0; i < NUM_SEGMENTS; i++)
    {
        int neighborIndex = (i + 1) % NUM_SEGMENTS;

        b2Body* currentBody = bodies[i];
        b2Body* neighborBody = bodies[neighborIndex];
        /*
        // to neighbor
        jointDef.Initialize(currentBody,
                            neighborBody,
                            currentBody->GetWorldCenter(),
                            neighborBody->GetWorldCenter());
        jointDef.collideConnected = true;
        jointDef.frequencyHz = SPRINGINESS * 8.0f;
        jointDef.dampingRatio = 0; // DAMPING;
        jointDef.type = e_distanceJoint;
        physicsWorld->CreateJoint(&jointDef);*/


        b2Vec2 prisPos((currentBody->GetWorldCenter().x + neighborBody->GetWorldCenter().x) * 0.5f,
            (currentBody->GetWorldCenter().y + neighborBody->GetWorldCenter().y) * 0.5f);
        b2Vec2 prisDir = prisPos - center;

        prisJointDef.Initialize(currentBody, neighborBody, currentBody->GetWorldCenter(), prisDir);
        prisJointDef.lowerTranslation = -0.1f;
        prisJointDef.upperTranslation = 0.1f;
        prisJointDef.enableLimit = true;
        prisJointDef.maxMotorForce = 0.0f;
        prisJointDef.motorSpeed = 0.0f;
        prisJointDef.enableMotor = false;
        physicsWorld->CreateJoint(&prisJointDef);


        // to center
        jointDef.Initialize(currentBody,
                            innerCircleBody,
                            currentBody->GetWorldCenter(),
                            center);
        jointDef.collideConnected = true;
        //jointDef.frequencyHz = SPRINGINESS;
        //jointDef.dampingRatio = DAMPING;
        jointDef.stiffness = SPRINGINESS;
        jointDef.damping = DAMPING;
        jointDef.type = e_distanceJoint;
        physicsWorld->CreateJoint(&jointDef);

    }

    return innerCircleBody;
}

b2Body* PhysicsManager::AddBox(float x, float y, float w, float h, float angle,
                               const BodyType bodyType, const FixtureData* fixtureData, float ctrx, float ctry)
{
    b2BodyDef bodyDef;
    bodyDef.type = static_cast<b2BodyType>(bodyType);
    bodyDef.position.Set(x * UNRATIO, y * UNRATIO);
    bodyDef.angle = toRadians(angle);

    //b2Vec2 center = b2Vec2(-w * 0.5f * UNRATIO, -h * 0.5f * UNRATIO);
    b2Vec2 center = b2Vec2(w * (0.5f - ctrx) * UNRATIO, h * (0.5f - ctry) * UNRATIO);
    b2PolygonShape shape;
    shape.SetAsBox(w * 0.5f * UNRATIO, h * 0.5f * UNRATIO, center, angle);

    b2FixtureDef fixtureDef = (*fixtureData);
    fixtureDef.shape = &shape;

    b2Body* bodyBox = PhysicsManager::physicsWorld->CreateBody(&bodyDef);
    bodyBox->CreateFixture(&fixtureDef);

    physicsObjectList.push_back(bodyBox);
    return bodyBox;
}

b2Body* PhysicsManager::AddCircle(float x, float y, float radius, const BodyType bodyType, const FixtureData* fixtureData)
{
    b2BodyDef bodyDef;
    bodyDef.type = static_cast<b2BodyType>(bodyType);
    bodyDef.position.Set(x * UNRATIO, y * UNRATIO);

    b2CircleShape shape;
    shape.m_radius = (radius * UNRATIO);

    b2FixtureDef fixtureDef = (*fixtureData);
    fixtureDef.shape = &shape;

    b2Body* bodyBox = PhysicsManager::physicsWorld->CreateBody(&bodyDef);
    bodyBox->CreateFixture(&fixtureDef);

    PhysicsManager::physicsObjectList.push_back(bodyBox);
    return bodyBox;
}

const float FIXED_TIMESTEP = 1.f / 60.f;

void PhysicsManager::Update(float dt)
{
    ////this->physicsWorld->Step(P_TIMESTEP, velocityIterations, positionIterations);
    //this->physicsWorld->Step(dt, velocityIterations, positionIterations);
    //this->physicsWorld->ClearForces();

    float progress = 0.0;
    while (progress < dt)
    {
        float step = min((dt - progress), FIXED_TIMESTEP);
        this->physicsWorld->Step(step, velocityIterations, positionIterations);
        progress += step;
    }
}

unsigned int PhysicsManager::RemoveBody(b2Body* body)
{
    if (!body)
        return 0;

    auto c = physicsObjectList.size();
    physicsObjectList.remove(body);
    if (c == physicsObjectList.size())
    {
        physicsWorld->DestroyBody(body);
        return 0;
    }

    std::list<b2Body*> links;

    for (b2JointEdge* f = body->GetJointList(); f; f = f->next)
    {
        if (f && f->other)
            links.push_back(f->other);
    }
    physicsWorld->DestroyBody(body);

    for (std::list<b2Body*>::iterator i = links.begin(); i != links.end();)
    {
        physicsObjectList.remove(*i);
        physicsWorld->DestroyBody(*i);
        ++i;
    }

    return 1 + static_cast<unsigned int>(links.size());
}

int PhysicsManager::RemovePhysicBodies(int leftLimit, int rightLimit, int upLimit, int downLimit)
{
    int count = 0;
    for (std::list<b2Body*>::iterator i = physicsObjectList.begin(); i != physicsObjectList.end();)
    {
        auto pos = (*i)->GetPosition();
        if ((pos.x*RATIO < leftLimit) || (pos.x*RATIO > rightLimit) ||
            (pos.y*RATIO < upLimit) || (pos.y*RATIO > downLimit))
        {
            physicsWorld->DestroyBody(*i);
            i = physicsObjectList.erase(i); //update iterator
            count++;
        }
        else
            ++i; //next body
    }
    if (count > 0)
        DBG_OUTPUT("Deleted bodies: %d", count);
    return count;
}

// --------------------------------

#pragma region COLLISIONS

void PhysicsManager::InitContact(b2Contact* contact)
{
    if (oneWayPlatforms <= 1 || oneWayPlatforms >= 4)
        contact->SetEnabled(true);
}

void PhysicsManager::BeginContact(b2Contact* contact)
{
    b2Fixture* fixtureA = contact->GetFixtureA();
    b2Fixture* fixtureB = contact->GetFixtureB();

    //GameObject* goA = static_cast<GameObject*>(reinterpret_cast<GameObject*>(fixtureA->GetBody()->GetUserData().pointer));
    //GameObject* goB = static_cast<GameObject*>(reinterpret_cast<GameObject*>(fixtureB->GetBody()->GetUserData().pointer));

    GameObject* goA = GetUserData<GameObject*>(fixtureA);
    GameObject* goB = GetUserData<GameObject*>(fixtureB);

    if (!goA || !goB)
        return;

    if (goA->debugMode || goB->debugMode)
    {
        contact->SetEnabled(false);
    }

    if (fixtureA->IsSensor() || fixtureB->IsSensor())
    {
        if (goA && goA->m_type == Objects::Player)
        {
            if (goB && (goB->m_type == Objects::Lava || goB->m_type == Objects::SawBlade || goB->m_type == Objects::Spike))
            {
                static_cast<Character*>(goA)->Die();
            }
        }
        if (goB && goB->m_type == Objects::Player)
        {
            if (goA && (goA->m_type == Objects::Lava || goA->m_type == Objects::SawBlade || goA->m_type == Objects::Spike))
            {
                static_cast<Character*>(goB)->Die();
            }
        }
        if (goA && goA->m_type == Objects::Enemy)
        {
            if (goB && (goB->m_type == Objects::Lava))
            {
                static_cast<Character*>(goA)->Die();
            }
        }
        if (goB && goB->m_type == Objects::Enemy)
        {
            if (goA && (goA->m_type == Objects::Lava))
            {
                static_cast<Character*>(goB)->Die();
            }
        }
    }
    else
    {
        int numPoints = contact->GetManifold()->pointCount;
        b2WorldManifold worldManifold;
        contact->GetWorldManifold(&worldManifold);
        glm::vec2 avgPos(0, 0);
        if (numPoints > 0)
        {
            for (int i = 0; i < numPoints; i++)
            {
                glm::vec2 sfPos = { worldManifold.points[i].x * RATIO, worldManifold.points[i].y * RATIO };
                avgPos += sfPos;
            }
            avgPos = glm::vec2(avgPos.x / (float)numPoints, avgPos.y / (float)numPoints);

            b2Vec2 cnormal = contact->GetManifold()->localNormal;
            bool bca = goA->OnBeginContact(goB, fixtureB, avgPos, { cnormal.x, cnormal.y });
            bool bcb = goB->OnBeginContact(goA, fixtureA, avgPos, { -cnormal.x, -cnormal.y });

            if (!bca || !bcb)
            {
                contact->SetEnabled(false);
            }
        }
    }

    if (oneWayPlatforms >= 2)
    {
        // ################################
        // https://www.iforce2d.net/src/iforce2d_OneWayWalls_demo.h

        //check if one of the fixtures is single-sided
        b2Fixture* platformFixture = nullptr;
        b2Fixture* otherFixture = nullptr;
        bool fixtureAIsPlatform = (goA->m_type == Objects::Platform);
        bool fixtureBIsPlatform = (goB->m_type == Objects::Platform);
        if (fixtureAIsPlatform && fixtureBIsPlatform)
        {
            contact->SetEnabled(false);//avoids problems with swinging wall
            return;
        }
        else if (fixtureAIsPlatform)
        {
            platformFixture = fixtureA;
            otherFixture = fixtureB;
        }
        else if (fixtureBIsPlatform)
        {
            platformFixture = fixtureB;
            otherFixture = fixtureA;
        }

        //if (!((goA && goA->m_type == Objects::Player) || (goB && goB->m_type == Objects::Player)))
        //    return;

        bool solid = true;

        if (platformFixture)
        {
            int numPoints = contact->GetManifold()->pointCount;
            b2WorldManifold worldManifold;
            contact->GetWorldManifold(&worldManifold);

            b2Body* platformBody = platformFixture->GetBody();
            b2Body* otherBody = otherFixture->GetBody();

            //check if contact points are moving into platform
            solid = false;
            for (int i = 0; i < numPoints; i++)
            {
                b2Vec2 pointVelPlatform =
                    platformBody->GetLinearVelocityFromWorldPoint(worldManifold.points[i]);
                b2Vec2 pointVelOther =
                    otherBody->GetLinearVelocityFromWorldPoint(worldManifold.points[i]);
                b2Vec2 relativeVel = platformBody->GetLocalVector(pointVelOther - pointVelPlatform);
                if (relativeVel.y < -1)
                    solid = true;//point is moving into platform, leave contact solid
                else if (relativeVel.y < 1)
                {
                    // borderline case, moving only slightly out of platform
                    b2Vec2 contactPointRelativeToPlatform =
                        platformBody->GetLocalPoint(worldManifold.points[i]);
                    auto platformAABB = platformFixture->GetAABB(0);
                    auto platformSize = platformAABB.GetExtents().y;
                    float platformFaceY = platformSize;
                    if (contactPointRelativeToPlatform.y > platformFaceY - 0.05)
                        solid = true;
                }
            }
        }

        if (!solid)
            //no points are moving into platform, contact should not be solid
            contact->SetEnabled(false);
    }
}

void PhysicsManager::EndContact(b2Contact* contact)
{
    if (contact->IsEnabled())
    {
        b2Fixture* fixtureA = contact->GetFixtureA();
        b2Fixture* fixtureB = contact->GetFixtureB();

        GameObject* goA = static_cast<GameObject*>(reinterpret_cast<GameObject*>(fixtureA->GetBody()->GetUserData().pointer));
        GameObject* goB = static_cast<GameObject*>(reinterpret_cast<GameObject*>(fixtureB->GetBody()->GetUserData().pointer));

        if (goA && goB)
        {
            //b2Vec2 cnormal = contact->GetManifold()->localNormal;
            goA->OnEndContact(goB);
            goB->OnEndContact(goA);
        }
    }
    if (oneWayPlatforms >= 2)
    {
        contact->SetEnabled(true);
    }
}

bool Intersects(const b2AABB& a, const b2AABB& b)
{
    bool result = true;
    result = result && b.lowerBound.x <= a.upperBound.x;
    result = result && b.lowerBound.y <= a.upperBound.y;
    result = result && b.upperBound.x >= a.lowerBound.x;
    result = result && b.upperBound.y >= a.lowerBound.y;
    return result;
}

void PreSolvePlayerCollision(b2Contact * contact, b2Vec2 cnormal, GameObject * goA, GameObject * goB, b2Fixture * fixtureA, b2Fixture * fixtureB)
{
    const float epsilon = 0.01f;

    auto plyr_body = fixtureA->GetBody();
    if (!plyr_body || !plyr_body->IsEnabled())
        return;
    if (!fixtureB->GetBody()->IsEnabled())
        return;

    b2Vec2 plyr_velo = plyr_body->GetLinearVelocity();

    auto plyr_aabb = fixtureA->GetAABB(0);
    auto plyr_mins = plyr_aabb.GetCenter() - plyr_aabb.GetExtents();

    auto obj_aabb = fixtureB->GetAABB(0);
    auto obj_maxs = obj_aabb.GetCenter() + obj_aabb.GetExtents();

    auto pposy = min(goA->posy, goA->prev_posy) * UNRATIO;

    auto below = (max(plyr_mins.y, pposy) < obj_maxs.y - epsilon);

    if (cnormal.y > -0.1f || max(plyr_velo.y, goA->prev_vel.y) > 0.01f || (below))
    {
        contact->SetEnabled(false);
    }
}

void PhysicsManager::PreSolve(b2Contact* contact, const b2Manifold* oldManifold)
{
    const b2Manifold* manifold = contact->GetManifold();
    if (manifold->pointCount == 0)
        return;

    //b2PointState state1[b2_maxManifoldPoints], state2[b2_maxManifoldPoints];
    //b2GetPointStates(state1, state2, oldManifold, manifold);

    //b2WorldManifold worldManifold;
    //contact->GetWorldManifold(&worldManifold);

    // make all platforms one-way
    if (oneWayPlatforms == 1 || oneWayPlatforms >= 3)
    {
        contact->SetEnabled(true);

        b2Fixture* fixtureA = contact->GetFixtureA();
        b2Fixture* fixtureB = contact->GetFixtureB();

        GameObject* goA = static_cast<GameObject*>(reinterpret_cast<GameObject*>(fixtureA->GetBody()->GetUserData().pointer));
        GameObject* goB = static_cast<GameObject*>(reinterpret_cast<GameObject*>(fixtureB->GetBody()->GetUserData().pointer));

        if (!goA || !goB)
            return;

        if (goA->debugMode || goB->debugMode)
        {
            contact->SetEnabled(false);
        }

        b2Vec2 cnormal = contact->GetManifold()->localNormal;

        if (goA->IsCharacter() && goB->m_type == Objects::Platform)
        {
            PreSolvePlayerCollision(contact, cnormal, goA, goB, fixtureA, fixtureB);
        }

        if (goB->IsCharacter() && goA->m_type == Objects::Platform)
        {
            PreSolvePlayerCollision(contact, -cnormal, goB, goA, fixtureB, fixtureA);
        }
    }
}

//void PhysicsManager::PreSolve(b2Contact* contact, const b2Manifold* oldManifold) { } 
void PhysicsManager::PostSolve(b2Contact* contact, const b2ContactImpulse* impulse) {}

#pragma endregion COLLISIONS
