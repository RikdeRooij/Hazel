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
    physicsMgr = new PhysicsManager();
    if (objectList.empty())
        objectList = std::list<GameObject*>();

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

    delete physicsMgr;
    physicsMgr = nullptr;
}

PhysicsManager* ObjectManager::GetPhysicsMgr()
{
    return physicsMgr;
}

Player* ObjectManager::CreatePlayer(float x, float y, float size)
{
    auto fixtureDef = FixtureData(1.0f, 0.5f, 0.45f, "Player");
    auto bodyBox = physicsMgr->AddBox(x * LVL_SCALE, y * LVL_SCALE, size * LVL_SCALE, size * 0.9f * LVL_SCALE, 0,
                                      BodyType::dynamicBody, &fixtureDef, 0.5f, 0.55f);

    //TextureAtlas textureRef = TextureRef(Hazel::Texture2D::Create("assets/Jelly2.png"));
    TextureAtlas textureRef = TextureAtlas("assets/jelly_anim.xml");

    Player* player = new Player(bodyBox, textureRef, size * 1.4f * LVL_SCALE, size * 1.0f * LVL_SCALE, LVL_SCALE);
    player->Init(Objects::Player, 3);

    DBG_OUTPUT("Added: %s %s", DBG_GM_GO(player), DBG_PM_BODY(player->GetBody(), "Player").c_str());
    //objectList.push_back(player);
    return player;
}

Enemy* ObjectManager::CreateEnemy(float x, float y, float size)
{
    auto fixtureDef = FixtureData(1.0f, 0.5f, 0.45f, "Enemy");
    auto bodyBox = physicsMgr->AddBox(x * LVL_SCALE, y * LVL_SCALE, size * LVL_SCALE, size * 0.9f * LVL_SCALE, 0,
                                      BodyType::dynamicBody, &fixtureDef, 0.5f, 0.55f);

    //TextureAtlas textureRef = TextureRef(Hazel::Texture2D::Create("assets/Jelly2.png"));
    TextureAtlas textureRef = TextureAtlas("assets/jelly_anim.xml");

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
    lava->Init(Objects::Lava, 3);

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
    object->m_type = Objects::Object;
    object->Init(Objects::Object, 0);

    b2BodyUserData data;
    data.pointer = reinterpret_cast<uintptr_t>(object);
    physBody->SetUserData(data);

    objectList.push_back(object);
    return object;
}

// y-in = @bottom, y-out = @top
GameObject* ObjectManager::AddBackground(float &y)
{
    auto bgTex = textures[Textures::Lvl_Background];
    glm::vec2 bgSize = { bgTex.Get()->GetWidth(), bgTex.Get()->GetHeight() };

    float bgHeight = (bgSize.y - LVL_OVERLAP);
    y += bgHeight;

    int i = (int)(y / bgHeight);
    bool flip = abs(i % 2) == 1;

    GameObject* object = new GameObject(bgTex, { 0, (y - bgHeight) * LVL_SCALE }, { (flip ? -bgSize.x : bgSize.x) * LVL_SCALE, bgSize.y * LVL_SCALE }, { 0.5f, 0.f });
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

    float h = (lwallSize.y - LVL_OVERLAP);
    y += h;

    GameObject* object = CreateBoxPhysicsObject({ (-offX - lwallSize.x * 0.0f), (y - h) }, lwallSize, { 1, 0 }, 0,
                                                lwallTex, staticBody, LVL_FIXTURE);
    object->Init(Objects::Wall, 3);

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

    DBG_OUTPUT("Added: %s %s", DBG_GM_GO(object), DBG_PM_BODY(object->GetBody(), "LVL_L_WALL").c_str());
    //objectList.push_back(object);
    return object;
}

// y-in = @bottom, y-out = @top
GameObject* ObjectManager::AddRightWall(float offX, float &y)
{
    Textures::Type texType = RandomTexture(Textures::Lvl_Wall_Right_Big_0, Textures::Lvl_Wall_Right_Small_1);
    auto rwallTex = textures[texType];
    auto rwallSize = glm::vec2(rwallTex.Get()->GetWidth(), rwallTex.Get()->GetHeight());

    float h = (rwallSize.y - LVL_OVERLAP);
    y += h;

    GameObject* object = CreateBoxPhysicsObject({ (offX + rwallSize.x * 0.0f), (y - h) }, rwallSize, { 0, 0 }, 0,
                                                rwallTex, staticBody, LVL_FIXTURE);
    object->Init(Objects::Wall, 3);

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

    DBG_OUTPUT("Added: %s %s", DBG_GM_GO(object), DBG_PM_BODY(object->GetBody(), "LVL_R_WALL").c_str());
    //objectList.push_back(object);
    return object;
}

GameObject* ObjectManager::AddPlatform(float x, float y, glm::vec2 org, float angle, Textures::Type type)
{
    auto tex = textures[type];
    auto texSize = glm::vec2(tex.Get()->GetWidth(), tex.Get()->GetHeight());

    float h = (texSize.y - LVL_OVERLAP);
    y += h;

    GameObject* object = CreateBoxPhysicsObject({ (x), (y - h) }, texSize, { org.x,  org.y }, angle,
                                                tex, staticBody, LVL_FIXTURE);
    object->Init(Objects::Platform, 1);

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

    b2Body* physBody = physicsMgr->AddCircle(x * LVL_SCALE, y * LVL_SCALE, r * 0.40f * LVL_SCALE, kinematicBody, &FixtureData::METAL);

    b2Body* sensorBody = physicsMgr->AddCircle(x * LVL_SCALE, y * LVL_SCALE, r * 0.46f * LVL_SCALE, staticBody, &FixtureData::SENSOR);

#if DEBUG
    sensorBody->_debug = "sawblade_sensor";
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
    object->Init(Objects::SawBlade, 2);

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

    b2Body* physBody = physicsMgr->AddBox((x)* LVL_SCALE, (y)* LVL_SCALE,
        (float)spikeSize.x * LVL_SCALE, (float)spikeSize.y * LVL_SCALE, angle, staticBody, &FixtureData::SENSOR);

    GameObject* object = new GameObject(physBody, spikeTex, { spikeSize.x * LVL_SCALE, spikeSize.y * LVL_SCALE }, { .5f, .5f });
    object->Init(Objects::Spike, 2);

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

void ObjectManager::AddPlatforms(float wallOffX)
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

    lvl_prev_double = false;

    // small platform on left or right: add another
    if (tex == Textures::Lvl_Platform_0 && side)
    {
#define LVL_SECONDS_PUSH 1.25f
#define LVL_SECONDS_RAND 0.1f
        // push to other side, randomize lil bit
        rx = randfunc((-rx * LVL_SECONDS_PUSH) - LVL_SECONDS_RAND, (-rx * LVL_SECONDS_PUSH) + LVL_SECONDS_RAND);

        auto go = AddPlatform(wallOffX * rx, lvl_y + randfunc(40, 60), { 1 - ox, 0.f }, 0, Textures::Lvl_Platform_0);
        //go->setColor({ 0.0f, 1.0f, 1.0f, 1.0f });
        lvl_prev_double = true;
    }
    else
    {
        if (lvl_y > 500 && (randfunc(0.0f, 1.0f) < LVL_ENEMY_CHANCE))
        {
            auto pp = go->GetPosition({ .5f, 0.f }) / LVL_SCALE;
            auto enemy = CreateEnemy(pp.x, pp.y, 33);
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
    lvl_c_y = LVL_END_Y;
    lvl_y = LVL_END_Y - LVL_OFFS_Y;

    UpdateLevel(y);

    while (lvl_y < y / LVL_SCALE && lvl_c_y < y / LVL_SCALE)
        UpdateLevel(y);
}

void ObjectManager::UpdateLevel(float y)
{
    y = y / LVL_SCALE;
    float bgHalfX = textures[Textures::Lvl_Background].Get()->GetWidth() * 0.5f;
    float wallOffX = bgHalfX - LVL_WALLS_X;

    if (lvl_c_y < y)
        AddBackground(lvl_c_y);

    if (lvl_l_y < y)
        AddLeftWall(wallOffX, lvl_l_y);

    if (lvl_r_y < y)
        AddRightWall(wallOffX, lvl_r_y);

    if (lvl_y < y)
    {
        lvl_y += LVL_PLATFORM_Y;
        AddPlatforms(wallOffX);
    }
}

int ObjectManager::RemoveObjectsBelow(float y)
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
            if ((pos.y + h * .5f < y) || pos.y > y + 32)
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

        int dc = physicsMgr->RemoveBody(go->GetBody());
        if (!go->destroyed)
            delete go;

        return dc > 0 ? dc : (removed ? 1 : 0);
    }
    return 0;
}

void ObjectManager::UpdateStep(float dt)
{
    UpdateObjects(dt);
    physicsMgr->Update(dt);
}

void ObjectManager::UpdateObjects(float dt)
{
    for (std::list<GameObject*>::iterator iter = objectList.begin(); iter != objectList.end(); ++iter)
        (*iter)->Update(dt);
}

void ObjectManager::DrawObjects(int layer)
{
    for (std::list<GameObject*>::iterator iter = objectList.begin(); iter != objectList.end(); ++iter)
        (*iter)->Draw(layer);
}
