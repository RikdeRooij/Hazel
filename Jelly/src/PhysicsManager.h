#pragma once


#include <box2d/box2d.h>

#include <list>
#include "GameObject.h"


#define P_GRAVITY -9.81f

#define P_TIMESTEP (1.0f / 60.0f)

#define velocityIterations 8
#define positionIterations 3

namespace Jelly
{

    enum BodyType { staticBody = 0, kinematicBody, dynamicBody };

    // ----------------

    inline static b2Vec2 translatedVector(const b2Vec2& vector, const b2Vec2& dir, float len)
    {
        return b2Vec2(vector.x + dir.x * len, vector.y + dir.y * len);
    }
    static b2Vec2 flattenVector(const b2Vec2& vector, const b2Vec2& axis, float damp = 1)
    {
        float len = b2Dot(axis, axis);
        float f = len > 1.401298E-45f ? (b2Dot(vector, axis) / len) : 0;
        return translatedVector(vector, axis, f * damp);
    }

    static b2Vec2 clipVector(const b2Vec2& vector, const b2Vec2& normal, float overbounce = 1)
    {
        float dot = b2Dot(vector, normal);
        if (dot >= 0)
            return vector;
        float len = b2Dot(normal, normal);
        float f = len > 1.401298E-45f ? (dot / len) : 0;
        return translatedVector(vector, normal, f * overbounce);
    }

    // ----------------

    // forward declaration
    typedef struct FixtureData FixtureData;

    class PhysicsManager : public b2ContactListener
    {
    public:
        PhysicsManager();
        virtual ~PhysicsManager();

        static b2World* GetPhysicsWorld() { return physicsWorld; }
        //std::list<b2Body*> getphysicsObjectList() const { return physicsObjectList; }
        auto GetObjectCount() const { return physicsObjectList.size(); }

        template <typename T>
        static T GetUserData(b2Fixture* fixture) { return fixture ? GetUserData<T>(fixture->GetBody()) : nullptr; }
        template <typename T>
        static T GetUserData(b2Body* body) { return body ? reinterpret_cast<T>(body->GetUserData().pointer) : nullptr; }

        // Add physics object
        void AddPhysicsObject(b2Body* body);
        b2Body* CreatePhysicsObject(float x, float y, float size);
        b2Body* AddBox(float x, float y, float w, float h, float angle, const BodyType bodyType, const FixtureData* fixtureData, float ctrx = 0.5f, float ctry = 0.5f);
        b2Body* AddCircle(float x, float y, float radius, const BodyType bodyType, const FixtureData* fixtureData);

        unsigned int RemoveBody(b2Body* body);

        void Update(float dt);
        int RemovePhysicBodies(int leftLimit, int rightLimit, int upLimit, int downLimit);

        // Physics calls
        virtual void InitContact(b2Contact* contact) override;
        virtual void BeginContact(b2Contact* contact) override;
        virtual void EndContact(b2Contact* contact) override;
        virtual void PreSolve(b2Contact* contact, const b2Manifold* oldManifold) override;
        virtual void PostSolve(b2Contact* contact, const b2ContactImpulse* impulse) override;

        // physics data
        int oneWayPlatforms = 1;

    private:
        static b2World* physicsWorld;
        std::list<b2Body*> physicsObjectList;

        float fixedTimestepAccumulator = 0;

    };

    /// Properties used to create a fixture.
    typedef struct FixtureData
    {
        /// The constructor sets the default fixture definition values.
        FixtureData()
        {
            //userData = NULL;
            userData = "";
            friction = 0.2f;
            restitution = 0.0f;
            density = 0.0f;
            isSensor = false;
            filter = b2Filter();
        }

        /// Use this to store application specific fixture data.
        std::string userData;
        /// The friction coefficient, usually in the range [0,1].
        float friction;
        /// The restitution (elasticity) usually in the range [0,1].
        float restitution;
        /// The density, usually in kg/m^2.
        float density;
        /// A sensor shape collects contact information but never generates a collision response.
        bool isSensor;
        /// Contact filtering data.
        b2Filter filter;

        // implicit conversion
        operator b2FixtureDef() const
        {
            b2FixtureDef fixtureDef;
            fixtureDef.shape = nullptr;

            //fixtureDef.userData = this->userData;
            fixtureDef.friction = this->friction;
            fixtureDef.restitution = this->restitution;
            fixtureDef.density = this->density;
            fixtureDef.isSensor = this->isSensor;
            fixtureDef.filter = this->filter;
            return fixtureDef;
        }

        // Constructor2
        FixtureData(float density, float friction = 0.2f, float restitution = 0.0f, char* userData = nullptr)
        {
            this->density = density; this->friction = friction; this->restitution = restitution;
            this->userData = userData;
            isSensor = false;
        }
        // Constructor3: sensor
        FixtureData(bool sensor, b2Filter filter = {}, char* userData = nullptr)
        {
            isSensor = sensor; this->filter = filter;
            this->userData = userData;
            this->density = 0.0f; this->friction = 0.2f; this->restitution = 0.0f;
        }

        static FixtureData Category(FixtureData fixtureData, uint16 categoryBits)
        {
            auto copy = FixtureData(fixtureData);
            copy.filter.categoryBits = categoryBits;
            return copy;
        }

        // Predefined fixtures.
        static const FixtureData BOX2D;
        static const FixtureData DEFAULT;
        static const FixtureData METAL;
        static const FixtureData STONE;
        static const FixtureData WOOD;
        static const FixtureData GLASS;
        static const FixtureData RUBBER;
        static const FixtureData ICE;
        static const FixtureData PUMICE;
        static const FixtureData POLYSTYRENE;
        static const FixtureData FABRIC;
        static const FixtureData SPONGE;
        static const FixtureData AIR;
        static const FixtureData HELIUM;

        static const FixtureData SENSOR;

        static const FixtureData TEST;
        static const FixtureData PROJECTILE;
        static const FixtureData LEVEL;

    } FixtureData;

}
