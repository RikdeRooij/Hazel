#include "ObjectManager.h"
#include "glm/gtc/random.inl"
#include <sstream>     // std::basic_stringstream, std::basic_istringstream, std::basic_ostringstream class templates and several typedefs

#include <Windows.h>   // OutputDebugString, FormatMessage, ModuleFilePath...
#include "Enemy.h"

using namespace Jelly;

// background   =  610 x 182
// big-walls    =  198 x 640
// small-walls  =  198 x 246

#define LVL_END_Y -60.0f // start level y end

#define LVL_OFFS_Y 30.0f // start platforms offset

#define LVL_OVERLAP 2.0f

#define LVL_WALLS_X ((198.0f-LVL_OVERLAP) / 3.0f) // overlay background (to center)

#define LVL_PLATFORM_Y 100.0f // y-dist between platforms (max)

#define LVL_PLATFORM_SCALE 4.0f


#define LVL_MIN_CHANCE 0.42f // min dist apart from not allowed values
#define LVL_MIN_DOUBLE 0.71f // add another platform when more then this away from center

#define LVL_SPIKE_CHANCE 0.2f
#define LVL_ENEMY_CHANCE 0.2f

#define SAW_ROT_SPEED 2.0f
#define SAW_OFFSET 32
#define SPIKE_OFFSET 32

// Debugging
const char* DBG_GM_GO(const GameObject* go) { return (typeid(*go).name() + 10); }
std::string DBG_PM_BODY(const b2Body* body, const std::string info = "")
{
    static const char* b2BodyTypeStr[] = { STRY(b2BodyType::b2_staticBody), STRY(b2BodyType::b2_kinematicBody), STRY(b2BodyType::b2_dynamicBody) };
    static const char* b2ShapeTypeStr[] = { STRY(b2Shape::e_circle), STRY(b2Shape::e_edge), STRY(b2Shape::e_polygon), STRY(b2Shape::e_chain), STRY(b2Shape::e_typeCount) };

    std::string bodyStr = b2BodyTypeStr[body->GetType()] + 15;
    std::string shapeStr = b2ShapeTypeStr[body->GetFixtureList()->GetShape()->GetType()] + 11;

    //std::string userData = (char*)body->GetFixtureList()->GetUserData();
    std::string infoStr = info.length() > 0 ? " *" + info + "*" : "";

    std::ostringstream os("");
    os << "<" << bodyStr.substr(0, bodyStr.length() - 4).c_str() << "," << shapeStr.c_str() << ">" << infoStr << std::ends;
    return os.str();
}

// re-declare statics
PhysicsManager* ObjectManager::physicsMgr;
std::list<GameObject*> ObjectManager::objectList;

// Constructor
ObjectManager::ObjectManager()
{
    instance = this;
    physicsMgr = new PhysicsManager();
    if (objectList.empty())
        objectList = std::list<GameObject*>();

    lvl_centerPool.resize(lvl_centerPoolIndex + 1, glm::vec2(0, 0));

    // Load resources, store them in resource pointers and react to loading errors
    try
    {
        textures[Textures::Lava] = Hazel::Texture2D::Create("assets/lava_f.png");
        textures[Textures::Lvl_Background] = Hazel::Texture2D::Create("assets/Level/achtergrond_muur.png");
        textures[Textures::Lvl_Ground] = Hazel::Texture2D::Create("assets/Level/begingrond.png");
        textures[Textures::Lvl_Wall_Left_Big_0] = Hazel::Texture2D::Create("assets/Level/muur_groot_links_01.png");
        textures[Textures::Lvl_Wall_Left_Big_1] = Hazel::Texture2D::Create("assets/Level/muur_groot_links_02.png");
        textures[Textures::Lvl_Wall_Left_Small_0] = Hazel::Texture2D::Create("assets/Level/muur_klein_links_01.png");
        textures[Textures::Lvl_Wall_Left_Small_1] = Hazel::Texture2D::Create("assets/Level/muur_klein_links_02.png");
        textures[Textures::Lvl_Wall_Right_Big_0] = Hazel::Texture2D::Create("assets/Level/muur_groot_rechts_01.png");
        textures[Textures::Lvl_Wall_Right_Big_1] = Hazel::Texture2D::Create("assets/Level/muur_groot_rechts_02.png");
        textures[Textures::Lvl_Wall_Right_Small_0] = Hazel::Texture2D::Create("assets/Level/muur_klein_rechts_01.png");
        textures[Textures::Lvl_Wall_Right_Small_1] = Hazel::Texture2D::Create("assets/Level/muur_klein_rechts_02.png");

        textures[Textures::Lvl_Platform_0] = Hazel::Texture2D::Create("assets/Level/platform_01.png");
        textures[Textures::Lvl_Platform_1] = Hazel::Texture2D::Create("assets/Level/platform_02.png");

        textures[Textures::SawBlade] = Hazel::Texture2D::Create("assets/Level/sawblade_01.png");
        textures[Textures::Spike] = Hazel::Texture2D::Create("assets/Level/spike.png");
    }
    catch (std::exception& e)
    {
        ERR_OUTPUT("%s", e.what());
    }

    DBG_OUTPUT("allocation ObjectManager");
}

Textures::Type RandomTexture(Textures::Type min, Textures::Type max)
{
    float rf = std::rand() / (float)RAND_MAX;
    int ri = static_cast<int>(std::round(rf * (max - min)));
    return static_cast<Textures::Type>(min + ri);
}

// Destructor
ObjectManager::~ObjectManager()
{
    for (std::list<GameObject*>::iterator i = objectList.begin(); i != objectList.end();)
    {
        GameObject* go = *i;
        i = objectList.erase(i); // update iterator
        delete go;
    }
    objectList.clear();
    textures.clear();

    delete physicsMgr;
    physicsMgr = nullptr;

    instance = nullptr;
}

PhysicsManager* ObjectManager::GetPhysicsMgr()
{
    return physicsMgr;
}

Player* ObjectManager::CreatePlayer(float x, float y, float size)
{
    auto fixtureDef = FixtureData(1.0f, 0.5f, 0.45f, "Player");
    fixtureDef.filter.categoryBits = (1 << Category::Player);
    auto bodyBox = physicsMgr->AddBox(x * LVL_SCALE, y * LVL_SCALE, size * LVL_SCALE, size * 0.9f * LVL_SCALE, 0,
                                      BodyType::dynamicBody, &fixtureDef, 0.5f, 0.55f);

    //TextureAtlas textureRef = TextureRef(Hazel::Texture2D::Create("assets/Jelly2.png"));
    TextureAtlas textureRef = TextureAtlas("assets/jelly_anim.xml");

    Player* player = new Player(bodyBox, textureRef, size * 1.4f * LVL_SCALE, size * 1.0f * LVL_SCALE, LVL_SCALE);
    player->Init(Objects::Player, 5);

    DBG_OUTPUT("Added: %s %s", DBG_GM_GO(player), DBG_PM_BODY(player->GetBody(), "Player").c_str());
    //objectList.push_back(player);
    return player;
}

Enemy* ObjectManager::CreateEnemy(float x, float y, float size)
{
    auto fixtureDef = FixtureData(1.0f, 0.5f, 0.45f, "Enemy");
    fixtureDef.filter.categoryBits = (1 << Category::Enemy);

    //fixtureDef.filter.maskBits = ~(1 << Category::Player);
    auto bodyBox = physicsMgr->AddBox(x * LVL_SCALE, y * LVL_SCALE, size * LVL_SCALE, size * 0.9f * LVL_SCALE, 0,
                                      BodyType::dynamicBody, &fixtureDef, 0.5f, 0.55f);

    //TextureAtlas textureRef = TextureRef(Hazel::Texture2D::Create("assets/Jelly2.png"));
    TextureAtlas textureRef = TextureAtlas("assets/enemy_anim.xml");

    Enemy* enemy = new Enemy(bodyBox, textureRef, size * 1.4f * LVL_SCALE, size * 1.0f * LVL_SCALE, LVL_SCALE);
    enemy->Init(Objects::Enemy, 1);
    //enemy->SetColor({ 1.f, 0.3f, 0.3f, 1.f });

    DBG_OUTPUT("Added: %s %s", DBG_GM_GO(enemy), DBG_PM_BODY(enemy->GetBody(), "Enemy").c_str());
    objectList.push_back(enemy);
    return enemy;
}

GameObject* ObjectManager::CreateLava(float x, float y, float w, float h)
{
    auto ctrx = .5f;
    auto ctry = 1.f;
    auto ctry_off = 0.06f;

    b2BodyDef bodyDef;
    bodyDef.type = b2BodyType::b2_kinematicBody;
    bodyDef.position.Set(x*UNRATIO * LVL_SCALE, y*UNRATIO * LVL_SCALE);

    b2PolygonShape shape;
    //shape.SetAsBox(w*0.5f*UNRATIO, h*0.5f*UNRATIO);
    //shape.SetAsBox(0.0f, h*-0.5f*UNRATIO * LVL_SCALE);
    shape.SetAsBox(w * 0.5f * UNRATIO * LVL_SCALE, h * 0.5f * UNRATIO * LVL_SCALE,
                   { w * (0.5f - ctrx) * UNRATIO * LVL_SCALE, h * (0.5f - ctry) * UNRATIO * LVL_SCALE }, 0.0f);

    b2FixtureDef fixtureDef = FixtureData::SENSOR;
    fixtureDef.shape = &shape;
    fixtureDef.filter.categoryBits = 1 << Category::Interactive;

    b2Body* physBody = physicsMgr->GetPhysicsWorld()->CreateBody(&bodyDef);
    physBody->CreateFixture(&fixtureDef);

    physicsMgr->AddPhysicsObject(physBody);

    auto texture = textures[Textures::Lava];
    //auto texture = Hazel::Texture2D::Create("assets/lava_f.png");

    //glm::vec2 texSize = { texture.get()->GetWidth(), texture.get()->GetHeight() };
    //GameObject* lava = new GameObject(physBody, texture, { texSize.x * LVL_SCALE, texSize.y * LVL_SCALE }, { .5f, 1.0f });

    GameObject* lava = new GameObject(physBody, texture, { w * LVL_SCALE, h * LVL_SCALE }, { ctrx, ctry - ctry_off });
    lava->SetTilingFactor({ w / h, 1.0f });
    lava->dontDestroy = true;
    lava->Init(Objects::Lava, 5);

    DBG_OUTPUT("Added: %s %s", DBG_GM_GO(lava), DBG_PM_BODY(lava->GetBody(), "Lava").c_str());
    //objectList.push_back(lava);
    return lava;
}

GameObject* ObjectManager::CreateBox(float x, float y, float w, float h, glm::vec4 color,
                                     const BodyType bodyType, const FixtureData* fixtureData)
{
    // We create a new object
    b2Body* physBody = physicsMgr->AddBox(x, y, w * LVL_SCALE, h * LVL_SCALE, 0, bodyType, fixtureData, 0.5f, 0.5f);

    GameObject* object = new GameObject(physBody, w * LVL_SCALE, h * LVL_SCALE, color);
    object->Init(Objects::Object, 2);

    b2BodyUserData data;
    data.pointer = reinterpret_cast<uintptr_t>(object);
    physBody->SetUserData(data);

    objectList.push_back(object);
    return object;
}

Projectile * Jelly::ObjectManager::CreateProjectile(float x, float y, float w, float h, float angle, glm::vec4 color, const FixtureData * fixtureData)
{
    // We create a new object
    b2Body* physBody = physicsMgr->AddBox(x, y, w * LVL_SCALE, h * LVL_SCALE, 0, BodyType::dynamicBody, fixtureData, 0.5f, 0.5f);

    static TextureRef ptex = Hazel::Texture2D::Create("assets/particle.png");
    Projectile* object = new Projectile(physBody, ptex, w * LVL_SCALE, h * LVL_SCALE, color);
    object->angle = angle;
    object->Init(Objects::Projectile, 3);

    b2BodyUserData data;
    data.pointer = reinterpret_cast<uintptr_t>(object);
    physBody->SetUserData(data);

    objectList.push_back(object);
    return object;
}

// y-in = @bottom, y-out = @top
GameObject* ObjectManager::AddBackground(float &y, float rndScale)
{
    auto bgTex = textures[Textures::Lvl_Background];
    glm::vec2 bgSize = { bgTex.Get()->GetWidth(), bgTex.Get()->GetHeight() };

    float bgHeight = (bgSize.y - LVL_OVERLAP);
    y += bgHeight;

    int i = (int)(y / bgHeight);
    bool flip = abs(i % 2) == 1;

    GameObject* object = new GameObject(bgTex, 
                                        { lvl_x * LVL_SCALE, (y - bgHeight) * LVL_SCALE },
                                        { (flip ? -bgSize.x : bgSize.x) * LVL_SCALE, bgSize.y * LVL_SCALE }, 
                                        { 0.5f, 0.f });
    float tx = Random::Float();
    tx = tx < 0.5f ? tx * tx : (1 - ((1 - tx) * (1 - tx)));
    //tx = tx < 0.5f ? tx * tx : (1 - ((1 - tx) * (1 - tx)));
    object->SetTilingOffset({ tx * rndScale, 0 });
    object->SetTilingFactor({ Random::Float() < 0.8f ? 1 : -1, 1 });
    object->Init(Objects::Background, 0);

    DBG_OUTPUT("Added: %s *LVL_BG*", DBG_GM_GO(object));
    objectList.push_back(object);
    return object;
}

// y-in = @bottom, y-out = @top
GameObject* ObjectManager::AddLeftWall(float offX, float &y)
{
    Textures::Type texType = RandomTexture(Textures::Lvl_Wall_Left_Big_0, Textures::Lvl_Wall_Left_Small_1);
    auto lwallTex = textures[texType];
    auto lwallSize = glm::vec2(lwallTex.Get()->GetWidth(), lwallTex.Get()->GetHeight());

    //auto go = AddLeftWall(offX, y, lwallSize.x, lwallSize.y, lwallTex);
    auto go = AddWall(-offX, 1, y, lwallSize.x, lwallSize.y, lwallTex);

    float h = (lwallSize.y - LVL_OVERLAP);

    // fill gabs
    if (texType == Textures::Lvl_Wall_Left_Big_0)
    {
        AddSawblade(-(offX - SAW_OFFSET), y - h + lwallSize.y * 0.75f);
    }
    else if (texType == Textures::Lvl_Wall_Left_Big_1)
    {
        AddSawblade(-(offX - SAW_OFFSET), y - h + lwallSize.y * 0.25f);
    }

    // spike wall
    if (randfunc(0.0f, 1.0f) < LVL_SPIKE_CHANCE)
    {
        AddSpikes(-(offX - SPIKE_OFFSET), y, -90);
    }

    return go;
}

// y-in = @bottom, y-out = @top
GameObject* ObjectManager::AddRightWall(float offX, float &y)
{
    Textures::Type texType = RandomTexture(Textures::Lvl_Wall_Right_Big_0, Textures::Lvl_Wall_Right_Small_1);
    auto rwallTex = textures[texType];
    auto rwallSize = glm::vec2(rwallTex.Get()->GetWidth(), rwallTex.Get()->GetHeight());

    //auto go = AddRightWall(offX, y, rwallSize.x, rwallSize.y, rwallTex);
    auto go = AddWall(offX, 0, y, rwallSize.x, rwallSize.y, rwallTex);

    float h = (rwallSize.y - LVL_OVERLAP);

    // fill gabs
    if (texType == Textures::Lvl_Wall_Right_Big_0)
    {
        AddSawblade(offX - SAW_OFFSET, y - h + rwallSize.y * 0.75f);
    }
    else if (texType == Textures::Lvl_Wall_Right_Big_1)
    {
        AddSawblade(offX - SAW_OFFSET, y - h + rwallSize.y * 0.25f);
    }

    // spike wall
    if (randfunc(0.0f, 1.0f) < LVL_SPIKE_CHANCE)
    {
        AddSpikes(offX - SPIKE_OFFSET, y, 90);
    }

    return go;
}

GameObject* ObjectManager::AddWall(float offX, float originX, float& y, float tw, float th, TextureRef texture)
{
    glm::vec2 wallSize = { tw, th };
    float h = (wallSize.y - LVL_OVERLAP);
    y += h;

    FixtureData fixtureDef = LVL_FIXTURE(1 << Category::World);
    GameObject* object = CreateBoxPhysicsObject({ lvl_x + offX, (y - h) }, wallSize, { originX, 0 }, 0,
                                                texture, staticBody, &fixtureDef);
    object->Init(Objects::Wall, 4);

    DBG_OUTPUT("Added: %s %s", DBG_GM_GO(object), DBG_PM_BODY(object->GetBody(), "LVL_WALL").c_str());
    //objectList.push_back(object);
    return object;
}

GameObject* ObjectManager::AddPlatform(float x, float y, glm::vec2 org, float angle, Textures::Type type)
{
    auto tex = textures[type];
    auto texSize = glm::vec2(tex.Get()->GetWidth(), tex.Get()->GetHeight());

    float h = (texSize.y - LVL_OVERLAP);
    y += h;

    auto cat = type == Textures::Lvl_Ground ? Category::World : Category::Platform;
    FixtureData fixtureDef = LVL_FIXTURE(1 << cat);
    GameObject* object = CreateBoxPhysicsObject({ (lvl_x + x), (y - h) }, texSize, { org.x,  org.y }, angle,
                                                tex, staticBody, &fixtureDef);
    object->Init(Objects::Platform, 2);

    DBG_OUTPUT("Added: %s %s", DBG_GM_GO(object), DBG_PM_BODY(object->GetBody(), "LVL_PLATFORM").c_str());
    //objectList.push_back(object);
    return object;
}

GameObject* ObjectManager::AddSawblade(float x, float y)
{
    auto sawTex = textures[Textures::SawBlade];
    auto sawSize = glm::vec2(sawTex.Get()->GetWidth(), sawTex.Get()->GetHeight());

    float d = -sign(x);
    float r = (sawSize.x + sawSize.y) * 0.5f;

    FixtureData fixtureDef = FixtureData::Category(FixtureData::METAL, 1 << Category::Interactive);
    b2Body* physBody = physicsMgr->AddCircle((lvl_x + x) * LVL_SCALE, y * LVL_SCALE, r * 0.40f * LVL_SCALE, kinematicBody, &fixtureDef);

    FixtureData fixtureDef2 = FixtureData::Category(FixtureData::SENSOR, 1 << Category::Interactive);
    b2Body* sensorBody = physicsMgr->AddCircle((lvl_x + x) * LVL_SCALE, y * LVL_SCALE, r * 0.46f * LVL_SCALE, staticBody, &fixtureDef2);

#if DEBUG
    //sensorBody->_debug = "sawblade_sensor";
#endif

    b2RevoluteJointDef jointDef;
    jointDef.Initialize(physBody, sensorBody, physBody->GetWorldCenter());
    jointDef.enableLimit = false;
    jointDef.maxMotorTorque = 999.0f;
    jointDef.motorSpeed = SAW_ROT_SPEED * -d;
    jointDef.enableMotor = true;
    physicsMgr->GetPhysicsWorld()->CreateJoint(&jointDef);

    physBody->SetAngularVelocity(SAW_ROT_SPEED * -d);

    GameObject* object = new GameObject(physBody, sawTex, { (sawSize.x * d) * LVL_SCALE, sawSize.y * LVL_SCALE }, { 0.5f, 0.5f });
    object->Init(Objects::SawBlade, 3);

    b2BodyUserData data;
    data.pointer = reinterpret_cast<uintptr_t>(object);
    sensorBody->SetUserData(data);

    DBG_OUTPUT("Added: %s %s", DBG_GM_GO(object), DBG_PM_BODY(physBody, "LVL_SAW").c_str());
    objectList.push_back(object);
    return object;
}

GameObject* ObjectManager::AddSpike(float x, float y, float angle)
{
    auto spikeTex = textures[Textures::Spike];
    auto spikeSize = glm::vec2(spikeTex.Get()->GetWidth(), spikeTex.Get()->GetHeight());

    FixtureData fixtureDef2 = FixtureData::Category(FixtureData::SENSOR, 1 << Category::Interactive);
    b2Body* physBody = physicsMgr->AddBox((lvl_x + x)* LVL_SCALE, (y)* LVL_SCALE,
        (float)spikeSize.x * LVL_SCALE, (float)spikeSize.y * LVL_SCALE, angle, staticBody, &fixtureDef2);

    GameObject* object = new GameObject(physBody, spikeTex, { spikeSize.x * LVL_SCALE, spikeSize.y * LVL_SCALE }, { .5f, .5f });
    object->Init(Objects::Spike, 5);

    DBG_OUTPUT("Added: %s %s", DBG_GM_GO(object), DBG_PM_BODY(physBody, "LVL_SPIKE").c_str());
    objectList.push_back(object);
    return object;
}

void ObjectManager::AddSpikes(float x, float y, float angle)
{
    const int count = 4;
    auto tex = textures[Textures::Spike];
    auto size = glm::vec2(tex.Get()->GetWidth(), tex.Get()->GetHeight());

    float rads = toRadians(angle);
    glm::vec2 dir(cos(rads), sin(rads));

    float radn = toRadians(angle - 90);
    //glm::vec2 norm(cos(radn), sin(radn));
    glm::vec2 start(x + cos(radn) * size.y * 0.4f, y + sin(radn) * size.y * 0.4f);

    for (int i = 0; i <= count; i++)
    {
        float a = (i - count * 0.5f) * size.x;
        glm::vec2 pos(start.x + dir.x * a, start.y + dir.y * a);

        AddSpike(pos.x, pos.y, angle);
    }
}

// Player can squeeze through
#define LVL_PLAYER_X 0.5f
#define LVL_PLAYER_S 1.0f - LVL_PLAYER_X

void ObjectManager::AddPlatforms(float wallOffX, int modeID)
{
    int max;
    Textures::Type tex = RandomTexture(Textures::Lvl_Platform_0, Textures::Lvl_Platform_1);
    float platformScale = tex == Textures::Lvl_Platform_0 ? 0.67f : 1.0f; // small or big platform

    float rx = randfunc(-1.0f, 1.0f); // x-pos (-1=left, 0=center, 1=right)

    // similar to previous placement: try again
    if (abs(lvl_prev_x - rx) < LVL_MIN_CHANCE * platformScale)
        rx = randfunc(-1.0f, 1.0f);

    // previous step placed 2 platform (ends are blocked most likely)
    if (lvl_prev_double)
    {
        if (tex == Textures::Lvl_Platform_1 && abs(rx) < 0.5f)
        {
            // no large platform near center
            rx = sign(rx) * (1.0f - abs(rx) * 0.2f); // force to side
        }
    }

    // do not place large platform in center
    max = 2;
    while (tex == Textures::Lvl_Platform_1 && abs(rx) < LVL_MIN_CHANCE)
    {
        float trx = randfunc(-1.0f, 1.0f);
        rx = sign(trx) * fmax(abs(trx), abs(rx)); // keep max
        //addSpike(wallOffX * rx, lvl_y, max * 45.0f);
        if (--max <= 0) break;
    }

    bool side = abs(rx) > LVL_MIN_DOUBLE;
    rx = trunc(rx * 3) / 2.0f; // 0.0 - 0.5 - 1.0 - 1.5

    lvl_prev_x = rx;
    float ox = (rx + 1.f) * .5f; // x-origin (0-1)

    GameObject* go =
        AddPlatform(wallOffX * rx, lvl_y, { ox, 0.f }, 0, tex);

    if (modeID == 1)
        go->SetColor({ 1,1,0,1 });
    if (modeID == 2)
        go->SetColor({ 0,1,1,1 });

    lvl_prev_double = false;

    // small platform on left or right: add another
    if (tex == Textures::Lvl_Platform_0 && side)
    {
#define LVL_SECONDS_PUSH 1.25f
#define LVL_SECONDS_RAND 0.1f
        // push to other side, randomize lil bit
        rx = randfunc((-rx * LVL_SECONDS_PUSH) - LVL_SECONDS_RAND, (-rx * LVL_SECONDS_PUSH) + LVL_SECONDS_RAND);

        GameObject*  go2 = AddPlatform(wallOffX * rx, lvl_y + randfunc(40, 60), { 1 - ox, 0.f }, 0, Textures::Lvl_Platform_0);
        if (modeID == 1)
            go2->SetColor({ 1,1,0,1 });
        if (modeID == 2)
            go2->SetColor({ 0,1,1,1 });

        //go->setColor({ 0.0f, 1.0f, 1.0f, 1.0f });
        lvl_prev_double = true;
    }
    else
    {
        if (lvl_y > 500 && (randfunc(0.0f, 1.0f) < LVL_ENEMY_CHANCE))
        {
            auto pp = go->GetPosition({ .5f, 0.f }) / LVL_SCALE;
            //auto enemy =
                CreateEnemy(pp.x, pp.y, 33);
            //enemy->GetBody()->SetEnabled(false);
        }
    }
}

void ObjectManager::GenerateLevel(float y)
{
    auto floor = AddPlatform(0, LVL_END_Y, { 0.5f, 1 }, 0, Textures::Lvl_Ground);
    floor->m_type = Objects::Ground;

    lvl_l_y = LVL_END_Y;
    lvl_r_y = LVL_END_Y;
    lvl_c_y = LVL_END_Y - 50;
    lvl_y = LVL_END_Y - LVL_OFFS_Y;
    
    lvl_prev_x = 0; // previous step platform.x
    lvl_prev_double = false; // placed 2 platforms previous step

    UpdateLevel(y);

    while (lvl_y < y / LVL_SCALE && lvl_c_y < y / LVL_SCALE)
        UpdateLevel(y);
}

// background   =  610 x 182
// big-walls    =  198 x 640
// small-walls  =  198 x 246

#define LVL_SIDE_CHANCE 0.5f
#define LVL_WAYOFF 6.2f
#if DEBUG
#define LVL_DEBUG true
#endif

void ObjectManager::UpdateLevel(float y)
{
    y = y / LVL_SCALE;
    float bgHalfX = textures[Textures::Lvl_Background].Get()->GetWidth() * 0.5f;
    float wallOffX = bgHalfX - LVL_WALLS_X;

    static bool wall_side_l = false;
    static bool wall_side_r = false;
    static float wall_sidew_l = 0;
    static float wall_sidew_r = 0;
    static GameObject* last_wall_l = nullptr;
    static GameObject* last_wall_r = nullptr;


    bool goleft = false;
    bool goright = false;

    while (lvl_c_y < y && !goleft && !goright)
    {
        lvl_centerPool[lvl_centerPoolIndex] = glm::vec2(lvl_x, lvl_c_y);
        lvl_centerPoolIndex = --lvl_centerPoolIndex % lvl_centerPool.size();

        AddBackground(lvl_c_y);

        goleft = (abs(lvl_c_y - lvl_l_y) <= 32) && (lvl_c_y - lvl_l_y) > -10 && y > 1;
        goright = (abs(lvl_c_y - lvl_r_y) <= 32) && (lvl_c_y - lvl_r_y) > -10 && y > 1;
        //DBG_OUTPUT("l: %.1f   r: %.1f", (lvl_c_y - lvl_l_y), (lvl_c_y - lvl_r_y));
    }

    if (lvl_y < y)
    {
        lvl_y += LVL_PLATFORM_Y;
        AddPlatforms(wallOffX);
    }

    float lvl_x_prev = lvl_x;

    if (goleft && !goright && lvl_x > -1)
    {
        lvl_centerPool[lvl_centerPoolIndex] = glm::vec2(lvl_x, lvl_c_y);
        lvl_centerPoolIndex = --lvl_centerPoolIndex % lvl_centerPool.size();

        while (lvl_y < lvl_l_y)
        {
            lvl_y += LVL_PLATFORM_Y;
            AddPlatforms(wallOffX, 2);
        }

        // ctr
        float bgHalfY = textures[Textures::Lvl_Background].Get()->GetHeight() * 0.5f;
        float lvl_c_y_prev = lvl_c_y;
        lvl_x = lvl_x_prev;
        AddBackground(lvl_c_y, 0.0f);
        lvl_y += LVL_PLATFORM_Y;
        AddPlatforms(wallOffX, 1);
        lvl_x = lvl_x_prev - LVL_WAYOFF / LVL_SCALE;
        AddBackground(lvl_c_y_prev, 0.0f);

        do
        {
            lvl_x = lvl_x_prev;
            AddBackground(lvl_c_y, 0.0f);
            if (lvl_c_y_prev + bgHalfY * 2 < lvl_r_y)
            {
                lvl_y += LVL_PLATFORM_Y;
                AddPlatforms(wallOffX, 1);
            }
            lvl_x = lvl_x_prev - LVL_WAYOFF / LVL_SCALE;
            AddBackground(lvl_c_y_prev, 0.0f);
        } while (lvl_c_y_prev + bgHalfY < lvl_r_y);

        lvl_y -= LVL_PLATFORM_Y;
        lvl_y -= LVL_PLATFORM_Y;
        float dy = lvl_c_y - lvl_r_y;

        // right
        if (dy > 0)
        {
            lvl_x = lvl_x_prev;
            Textures::Type texType2 = dy > 320
                ? RandomTexture(Textures::Lvl_Wall_Right_Big_0, Textures::Lvl_Wall_Right_Big_1)
                : RandomTexture(Textures::Lvl_Wall_Right_Small_0, Textures::Lvl_Wall_Right_Small_1);
            auto rwallTex = textures[texType2];
            auto rwallSize = glm::vec2(rwallTex.Get()->GetWidth(), rwallTex.Get()->GetHeight());
            rwallSize.y = dy;
            float roffset = wall_sidew_r > 0 ? (rwallSize.x + (LVL_OVERLAP * 2)) : 0;
            //rwallSize.y = (lvl_c_y - lvl_r_y);
            last_wall_r = AddWall(wallOffX + wall_sidew_r / LVL_SCALE - roffset, 0,
                                 lvl_r_y, rwallSize.x, rwallSize.y, rwallTex);
#if LVL_DEBUG
            last_wall_r->SetColor({ 0,1,1,1 });
#endif
        }
        else
        {
            //lvl_r_y -= last_wall_r->height / LVL_SCALE;
            last_wall_r->height += dy * LVL_SCALE;
            lvl_r_y += dy;
        }

        // right ground
        lvl_x = lvl_x_prev;
        auto floor2 = AddPlatform(wallOffX + 198 + 48, lvl_r_y, { 1, 0 }, 0, Textures::Lvl_Ground);
        floor2->m_type = Objects::Ground;
        lvl_r_y += floor2->GetHeight() / LVL_SCALE - LVL_OVERLAP;
#if LVL_DEBUG
        floor2->SetColor({ 0,1,0,1 });
#endif
        //AddRightWall(wallOffX, lvl_r_y);

        // left
        lvl_x = lvl_x_prev;
        auto floor = AddPlatform(-(wallOffX), lvl_l_y, { 1, 0 }, 0, Textures::Lvl_Ground);
        floor->m_type = Objects::Ground;
        lvl_l_y += floor->GetHeight() / LVL_SCALE - LVL_OVERLAP;
#if LVL_DEBUG
        floor->SetColor({ 1, 0, 0, 1 });
#endif

        lvl_x = lvl_x_prev - LVL_WAYOFF / LVL_SCALE;
        wall_sidew_l = 2.48000002f;
        wall_side_l = true;

        Textures::Type texType = RandomTexture(Textures::Lvl_Wall_Left_Small_0, Textures::Lvl_Wall_Left_Small_1);
        auto lwallTex = textures[texType];
        auto lwallSize = glm::vec2(lwallTex.Get()->GetWidth(), lwallTex.Get()->GetHeight());
        last_wall_l = AddWall(-(wallOffX + wall_sidew_l / LVL_SCALE - lwallSize.x - (LVL_OVERLAP * 2)), 1,
                             lvl_l_y, lwallSize.x, lwallSize.y, lwallTex);
#if LVL_DEBUG
        last_wall_l->SetColor({ 1, 1, 0, 1 });
#endif
    }


    if (goright && !goleft && lvl_x < 1)
    {
        lvl_centerPool[lvl_centerPoolIndex] = glm::vec2(lvl_x, lvl_c_y);
        lvl_centerPoolIndex = --lvl_centerPoolIndex % lvl_centerPool.size();

        while (lvl_y < lvl_r_y)
        {
            lvl_y += LVL_PLATFORM_Y;
            AddPlatforms(wallOffX, 2);
        }

        // ctr
        float bgHalfY = textures[Textures::Lvl_Background].Get()->GetHeight() * 0.5f;
        float lvl_c_y_prev = lvl_c_y;
        lvl_x = lvl_x_prev;
        AddBackground(lvl_c_y, 0.0f);
        lvl_y += LVL_PLATFORM_Y;
        AddPlatforms(wallOffX, 1);
        lvl_x = lvl_x_prev + LVL_WAYOFF / LVL_SCALE;
        AddBackground(lvl_c_y_prev, 0.0f);
        do
        {
            lvl_x = lvl_x_prev;
            AddBackground(lvl_c_y, 0.0f);
            if (lvl_c_y_prev + bgHalfY * 2 < lvl_l_y)
            {
                lvl_y += LVL_PLATFORM_Y;
                AddPlatforms(wallOffX, 1);
            }
            lvl_x = lvl_x_prev + LVL_WAYOFF / LVL_SCALE;
            AddBackground(lvl_c_y_prev, 0.0f);
        } while (lvl_c_y_prev + bgHalfY < lvl_l_y);

        lvl_y -= LVL_PLATFORM_Y;
        lvl_y -= LVL_PLATFORM_Y;
        float dy = lvl_c_y - lvl_l_y;

        // left
        if (dy > 0)
        {
            lvl_x = lvl_x_prev;
            Textures::Type texType2 = dy > 320
                ? RandomTexture(Textures::Lvl_Wall_Left_Big_0, Textures::Lvl_Wall_Left_Big_1)
                : RandomTexture(Textures::Lvl_Wall_Left_Small_0, Textures::Lvl_Wall_Left_Small_1);
            auto lwallTex = textures[texType2];
            auto lwallSize = glm::vec2(lwallTex.Get()->GetWidth(), lwallTex.Get()->GetHeight());
            lwallSize.y = dy;
            float loffset = wall_sidew_l > 0 ? (lwallSize.x + (LVL_OVERLAP * 2)) : 0;
            last_wall_l = AddWall(-(wallOffX + wall_sidew_l / LVL_SCALE - loffset), 1,
                                 lvl_l_y, lwallSize.x, lwallSize.y, lwallTex);
#if LVL_DEBUG
            last_wall_l->SetColor({ 0,1,1,1 });
#endif
        }
        else
        {
            //lvl_l_y -= last_wall_l->height / LVL_SCALE;
            last_wall_l->height += dy * LVL_SCALE;
            lvl_l_y += dy;
        }

        // left ground
        lvl_x = lvl_x_prev;
        auto floor2 = AddPlatform(-(wallOffX + 198 + 48), lvl_l_y, { 0, 0 }, 0, Textures::Lvl_Ground);
        floor2->m_type = Objects::Ground;
        lvl_l_y += floor2->GetHeight() / LVL_SCALE - LVL_OVERLAP;
#if LVL_DEBUG
        floor2->SetColor({ 0,1,0,1 });
#endif
        //AddRightWall(wallOffX, lvl_r_y);

        // right
        lvl_x = lvl_x_prev;
        auto floor = AddPlatform(wallOffX, lvl_r_y, { 0, 0 }, 0, Textures::Lvl_Ground);
        floor->m_type = Objects::Ground;
        lvl_r_y += floor->GetHeight() / LVL_SCALE - LVL_OVERLAP;
#if LVL_DEBUG
        floor->SetColor({ 1, 0, 0, 1 });
#endif

        lvl_x = lvl_x_prev + LVL_WAYOFF / LVL_SCALE;
        wall_sidew_r = 2.48000002f;
        wall_side_r = true;

        Textures::Type texType = RandomTexture(Textures::Lvl_Wall_Right_Small_0, Textures::Lvl_Wall_Right_Small_1);
        auto rwallTex = textures[texType];
        auto rwallSize = glm::vec2(rwallTex.Get()->GetWidth(), rwallTex.Get()->GetHeight());
        last_wall_r = AddWall((wallOffX + wall_sidew_r / LVL_SCALE - rwallSize.x - (LVL_OVERLAP * 2)), 0,
                             lvl_r_y, rwallSize.x, rwallSize.y, rwallTex);
#if LVL_DEBUG
        last_wall_r->SetColor({ 1, 1, 0, 1 });
#endif
    }

    // ################

    if (lvl_l_y < y)
    {
        if ((lvl_l_y > (LVL_END_Y * 0.5f) && (randfunc(0.0f, 1.0f) < LVL_SIDE_CHANCE)))
        {
            // outset wall
            if (!wall_side_l)
            {
                auto floor = AddPlatform(-wallOffX, lvl_l_y, { 1, 0 }, 0, Textures::Lvl_Platform_1);
                floor->m_type = Objects::Ground;
                lvl_l_y += floor->GetHeight() / LVL_SCALE - LVL_OVERLAP;
                wall_sidew_l = floor->GetWidth();
            }

            wall_side_l = true;
            Textures::Type texType = RandomTexture(Textures::Lvl_Wall_Left_Small_0, Textures::Lvl_Wall_Left_Small_1);
            auto lwallTex = textures[texType];
            auto lwallSize = glm::vec2(lwallTex.Get()->GetWidth(), lwallTex.Get()->GetHeight());
            last_wall_l = AddWall(-(wallOffX + wall_sidew_l / LVL_SCALE - lwallSize.x - (LVL_OVERLAP * 2)), 1,
                    lvl_l_y, lwallSize.x, lwallSize.y, lwallTex);
        }
        else
        {
            // normal wall
            if (wall_side_l)
            {
                // Lvl_Platform_1 = 2.48000002;  Lvl_Platform_0 = 1.45999992;  Ground = 8.78999996;
                Textures::Type tex = (wall_sidew_l > 4 ? Textures::Lvl_Ground : Textures::Lvl_Platform_1);
                auto floor2 = AddPlatform(-wallOffX, lvl_l_y, { 1, 0 }, 0, tex);// Textures::Lvl_Platform_1);
                floor2->m_type = Objects::Ground;
                lvl_l_y += floor2->GetHeight() / LVL_SCALE - LVL_OVERLAP;
            }
            wall_side_l = false;
            wall_sidew_l = 0;
            last_wall_l = AddLeftWall(wallOffX, lvl_l_y);
        }
    }

    if (lvl_r_y < y)
    {
        if (lvl_r_y > (LVL_END_Y * 0.5f) && (randfunc(0.0f, 1.0f) < LVL_SIDE_CHANCE))
        {
            if (!wall_side_r)
            {
                auto floor = AddPlatform(wallOffX, lvl_r_y, { 0, 0 }, 0, Textures::Lvl_Platform_1);
                floor->m_type = Objects::Ground;
                lvl_r_y += floor->GetHeight() / LVL_SCALE - LVL_OVERLAP;
                wall_sidew_r = floor->GetWidth();
            }
            wall_side_r = true;

            Textures::Type texType = RandomTexture(Textures::Lvl_Wall_Right_Small_0, Textures::Lvl_Wall_Right_Small_1);
            auto rwallTex = textures[texType];
            auto rwallSize = glm::vec2(rwallTex.Get()->GetWidth(), rwallTex.Get()->GetHeight());
            last_wall_r = AddWall(wallOffX + wall_sidew_r / LVL_SCALE - rwallSize.x - (LVL_OVERLAP*2), 0,
                    lvl_r_y, rwallSize.x, rwallSize.y, rwallTex);
        }
        else
        {
            if (wall_side_r)
            {
                auto floor2 = AddPlatform(wallOffX, lvl_r_y, { 0, 0 }, 0, Textures::Lvl_Platform_1);
                floor2->m_type = Objects::Ground;
                lvl_r_y += floor2->GetHeight() / LVL_SCALE - LVL_OVERLAP;
            }
            wall_side_r = false;
            wall_sidew_r = 0;
            last_wall_r = AddRightWall(wallOffX, lvl_r_y);
        }
    }

}

int ObjectManager::RemoveObjectsBelow(const float y) const
{
    int count = 0;
    for (std::list<GameObject*>::iterator i = objectList.begin(); i != objectList.end();)
    {
        if ((*i)->dontDestroy)
        {
            ++i; //next object
        }
        else
        {
            auto pos = (*i)->GetPosition();
            auto h = (*i)->GetHeight();
            if ((pos.y + h * .5f < y) || pos.y > y + 64)
            {
                GameObject* go = *i;
                i = objectList.erase(i); // update iterator

                DBG_OUTPUT("Deleted: Object %s", Objects::EnumStrings[go->m_type]);

                physicsMgr->RemoveBody(go->GetBody());
                //delete go;

                count++;
            }
            else
                ++i; //next object
        }
    }
    if (count > 0)
        DBG_OUTPUT("Deleted objects: %d", count);
    return count;
}

int ObjectManager::Remove(GameObject* go)
{
    if (go)
    {
        DBG_OUTPUT("Delete: Object %s", Objects::EnumStrings[go->m_type]);

        auto c = objectList.size();
        objectList.remove(go);
        bool removed = c != objectList.size();

        int dc = physicsMgr->RemoveBody(go->m_body);
        go->m_body = (nullptr);
        if (!go->destroyed)
            delete go;

        return dc > 0 ? dc : (removed ? 1 : 0);
    }
    return 0;
}

void ObjectManager::UpdateStep(const float dt) const
{
    physicsMgr->Update(dt);
    UpdateObjects(dt);
}

void ObjectManager::UpdateObjects(const float dt) const
{
    static GameObject* goBuffer[8192];
    int i = 0;
    for (std::list<GameObject*>::iterator iter = objectList.begin(); iter != objectList.end(); ++iter)
    {
        goBuffer[i++] = (*iter);
        (*iter)->Update(dt);
    }

    for (int j = i- 1; j >= 0 ; j--)
    {
        goBuffer[j]->LateUpdate(dt);
    }
}


void ObjectManager::DrawObjects(const int layer) const
{
    for (std::list<GameObject*>::iterator iter = objectList.begin(); iter != objectList.end(); ++iter)
        (*iter)->Draw(layer);
}

glm::vec2 ObjectManager::GetLevelCenter(glm::vec2 playerPos)
{
    glm::vec2 center(playerPos.x, playerPos.y);
    auto playerLvlPosX = playerPos.x / LVL_SCALE;
    auto playerLvlPosY = playerPos.y / LVL_SCALE;
    float prevLvlX = 0;
    float prevLvlY = 0;
    
    int poolSize = static_cast<int>(lvl_centerPool.size());
    for (int i = 0; i < poolSize; i++)
    {
        int j = (lvl_centerPoolIndex + i) - ((lvl_centerPoolIndex + i) / poolSize) * poolSize;
        auto& lvlctrx = lvl_centerPool[j];
    
    //for (auto& lvlctrx : lvl_centerPool)
    //{
        if (lvlctrx.y == 0)
            continue;
        if (lvlctrx.y < playerLvlPosY)
        {
            float xt = clamp01(abs(prevLvlX - lvlctrx.x) < 0.1f ? 0.0f : ((playerLvlPosX - lvlctrx.x) / (prevLvlX - lvlctrx.x)));
            float yt = clamp01((playerLvlPosY - lvlctrx.y) / (prevLvlY - lvlctrx.y));

            /*
            float t0 = clamp01(xt * yt);
            float t1 = clamp01(1 - ((1 - xt) * (1 - yt)));

            float t00 = 1 - clamp01(t0 * 2);
            float t11 = clamp01((t1 - 0.5f) * 2);

            float topright = Interpolate::Linearf(1.f, (1 - t00) * 0.5f + 0.5f, t00 * t1);;
            float tt = t00 * t0 + t11 * t1 * topright;
            */

            float yn = clamp01((yt - yt * yt) * 4);
            float tt = Interpolate::Linearf(Interpolate::Hermite(0.f, 1.f, yt), xt, 1 - (1-yn) * (1-yn));

            //float t = (xt + yt) * 0.5f; // max(xt, yt);
            float tx = Interpolate::Linearf(lvlctrx.x, prevLvlX, tt);
            float ty = Interpolate::Linearf(lvlctrx.y, prevLvlY, yt);

            center.x = tx * LVL_SCALE;
            center.y = ty * LVL_SCALE;
            break;
        }
        prevLvlX = lvlctrx.x;
        prevLvlY = lvlctrx.y;
    }
    return center;
}
