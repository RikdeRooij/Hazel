#ifndef OBJECTMANAGER_H_INCLUDED
#define OBJECTMANAGER_H_INCLUDED


#include "PhysicsManager.h"

#include "TextureRef.h"
#include "GameObject.h"
#include "Globals.h"

#include <map>
#include "Player.h"

#define LVL_SCALE 0.01f
//#define LVL_FIXTURE &FixtureData::DEFAULT // BOX2D
#define LVL_FIXTURE &FixtureData::TEST


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

    PhysicsManager* getPhysicsMgr();
    auto getObjectCount() const { return objectList.size(); }
    auto getObjectList() const { return objectList; }

    Player* createPlayer(float x, float y, float size);

    GameObject* createLava(float x, float y, float w, float h);

    static GameObject* createBox(float x, float y, float w, float h, glm::vec4 color,
                          const BodyType bodyType, const FixtureData* fixtureData = &FixtureData::DEFAULT);

    GameObject* addBackground(float &y);
    GameObject* addLeftWall(float offX, float &y);
    GameObject* addRightWall(float offX, float &y);

    GameObject* addPlatform(float x, float y, glm::vec2 org, float angle, Textures::Type type);
    void addPlatforms(float wallOffX);

    GameObject* addSawblade(float x, float y);
    GameObject* addSpike(float x, float y, float angle = 0);
    void addSpikes(float x, float y, float angle = 0);

    void generateLevel(float y);
    void updateLevel(float y);

    int removeObjectsBelow(float y);
    void UpdateStep(float dt);
    void updateObjects(float dt);

    void drawObjects(int layer);

protected:

    GameObject* createBoxPhysicsObject(glm::vec2 pos, glm::vec2 size, glm::vec2 origin, float angle, TextureRef tex,
                                    const BodyType bodyType = staticBody, const FixtureData * fixtureData = LVL_FIXTURE)
    {
        b2Body* physBody = physicsMgr->addBox((pos.x) * LVL_SCALE, (pos.y) * LVL_SCALE, 
                                              (size.x) * LVL_SCALE, (size.y) * LVL_SCALE,
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

#endif // !OBJECTMANAGER_H_INCLUDED
