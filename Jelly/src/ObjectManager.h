#pragma once

#include "PhysicsManager.h"

#include "TextureRef.h"
#include "GameObject.h"
#include "Globals.h"

#include <map>
#include "Player.h"
#include "Enemy.h"

#define LVL_SCALE 0.01f
//#define LVL_FIXTURE &FixtureData::DEFAULT // BOX2D
#define LVL_FIXTURE &FixtureData::LEVEL

namespace Jelly
{
    namespace Textures
    {
        enum Type
        {
            Player,
            PlayerParticle,
            Lava,
            LavaParticle,

            Lvl_Background,
            Lvl_Ground,
            Lvl_Platform_0, // small
            Lvl_Platform_1, // large

            Lvl_Wall_Left_Big_0,
            Lvl_Wall_Left_Big_1,
            Lvl_Wall_Left_Small_0,
            Lvl_Wall_Left_Small_1,

            Lvl_Wall_Right_Big_0,
            Lvl_Wall_Right_Big_1,
            Lvl_Wall_Right_Small_0,
            Lvl_Wall_Right_Small_1,

            SawBlade,
            Spike,
        };
    }

    //using Ref = std::shared_ptr<T>;
    class TextureRef;
    //typedef TextureType Texture2D;

    class ObjectManager
    {
    public:
        ObjectManager();
        ~ObjectManager();

        PhysicsManager* GetPhysicsMgr();
        auto GetObjectCount() const { return objectList.size(); }
        auto GetObjectList() const { return objectList; }

        Player* CreatePlayer(float x, float y, float size);
        Enemy* CreateEnemy(float x, float y, float size);

        GameObject* CreateLava(float x, float y, float w, float h);

        static GameObject* CreateBox(float x, float y, float w, float h, glm::vec4 color,
                                     const BodyType bodyType, const FixtureData* fixtureData = &FixtureData::DEFAULT);

        GameObject* AddBackground(float &y);
        GameObject* AddLeftWall(float offX, float &y);
        GameObject* AddRightWall(float offX, float &y);

        GameObject* AddPlatform(float x, float y, glm::vec2 org, float angle, Textures::Type type);
        void AddPlatforms(float wallOffX);

        GameObject* AddSawblade(float x, float y);
        GameObject* AddSpike(float x, float y, float angle = 0);
        void AddSpikes(float x, float y, float angle = 0);

        void GenerateLevel(float y);
        void UpdateLevel(float y);

        int RemoveObjectsBelow(float y);
        void UpdateStep(float dt);
        void UpdateObjects(float dt);

        void DrawObjects(int layer);

    protected:

        GameObject* CreateBoxPhysicsObject(glm::vec2 pos, glm::vec2 size, glm::vec2 origin, float angle, TextureRef tex,
                                           const BodyType bodyType = staticBody, const FixtureData * fixtureData = LVL_FIXTURE)
        {
            b2Body* physBody = physicsMgr->AddBox((pos.x) * LVL_SCALE, (pos.y) * LVL_SCALE,
                (size.x - 1) * LVL_SCALE, (size.y - 1) * LVL_SCALE,
                                                  angle, bodyType, fixtureData,
                                                  origin.x, origin.y);

            GameObject* object = new GameObject(physBody, tex, { (size.x) * LVL_SCALE, (size.y) * LVL_SCALE }, origin);
            objectList.push_back(object);
            return object;
        }

        // PhysicsManager instance
        static PhysicsManager *physicsMgr;

        // list of pointers to the objects
        static std::list<GameObject*> objectList;

        float lvl_l_y = 0;
        float lvl_r_y = 0;
        float lvl_c_y = 0;
        float lvl_y = 0;
        float lvl_prev_x = 0; // previous step platform.x
        bool lvl_prev_double = false; // placed 2 platforms previous step

        std::map<Textures::Type, TextureRef> textures;

    private:

        ObjectManager(const ObjectManager&) = delete;
        ObjectManager& operator=(const ObjectManager&) = delete;

        //float(*randfunc)(float, float) = &glm::linearRand<float>;
        float(*randfunc)(float, float) = &Random;

    };

}
