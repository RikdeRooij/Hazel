#include "Sandbox2D.h"
#include <imgui/imgui.h>

#include <glm/gtc/type_ptr.hpp>
#include "DebugDraw.h"
#include "glm/gtx/io.hpp"


// re-declare statics
ObjectManager* Sandbox2D::objectManager;

Sandbox2D::Sandbox2D()
    : Layer("Sandbox2D"), m_CameraController(1280.0f / 720.0f), player(nullptr), lava(nullptr), debugDraw(nullptr)
{
    m_CameraController.SetZoomLevel(3);
    m_CameraController.OnResize(1280, 720);
}

void Sandbox2D::OnAttach()
{
    HZ_PROFILE_FUNCTION();

    OnResize(float(Hazel::Application::Get().GetWindow().GetWidth()), float(Hazel::Application::Get().GetWindow().GetHeight()));
    m_CameraController.OnResize(m_ScreenWidth, m_ScreenHeight);

#if !DEBUG
    std::srand(std::time(nullptr));
#else
    std::srand(50);
#endif
    StartGame();
}

void Sandbox2D::OnDetach()
{
    DestroyGame();
    HZ_PROFILE_FUNCTION();
}

void Sandbox2D::StartGame()
{
    objectManager = new ObjectManager();
    //physicsMgr = new PhysicsManager();
    auto physicsMgr = objectManager->getPhysicsMgr();

    b2World* pWorld = physicsMgr->getPhysicsWorld();

    pWorld->SetGravity({ 0, -9.81f });

    pWorld->SetContactListener(physicsMgr);

    debugDraw = new DebugDraw(pWorld);
    pWorld->SetDebugDraw(debugDraw);

    glm::vec3 ctr = m_CameraController.GetCamera().GetPosition();
    objectManager->generateLevel(ctr.y);

    this->player = objectManager->createPlayer(0, 0, 50);

    this->lava = objectManager->createLava(0, -800, 600 * 4, 200 * 4);
}

void Sandbox2D::DestroyGame() const
{
    delete lava;
    delete player;
    delete debugDraw;
    //delete physicsMgr;
    //physicsMgr = NULL;
    delete objectManager;
    objectManager = nullptr;
}


#define DRAW_LAYER_COUNT 4

#define LVL_DIST_ADD 0.0f
#define LVL_DIST_DEL 4.0f

#define LAVA_MOVE_SPEED 1.0f
#define LAVA_MAX_DIST (LVL_DIST_DEL - 200.0f)


glm::vec2 UpdateLava(GameObject* lava, float dt, float viewY)
{
#if DEBUG
    viewY -= 4.f;
#endif
    lava->update(dt);
    glm::vec2 lavaPos = lava->getPosition({ 0.5f, 0.5f });

    float lavaAddSpeed = 0;
    if (lavaPos.y < viewY - (LAVA_MAX_DIST * LVL_SCALE))
        lavaAddSpeed = -(lavaPos.y - (viewY - (LAVA_MAX_DIST * LVL_SCALE)));
    if (lavaPos.y > viewY)
        lavaAddSpeed = -(lavaPos.y - viewY) * 100;

    lava->GetBody()->SetLinearVelocity(b2Vec2(0, (LAVA_MOVE_SPEED + lavaAddSpeed * LVL_SCALE)));

    auto lavaoffs = lava->getTilingOffset();
    lavaoffs.x += dt * 0.0005f;
    lava->setTilingOffset(lavaoffs);

    return lavaPos;
}

glm::vec2 UpdatePlayer(Player* player, float dt, Hazel::OrthographicCameraController& camCtrl)
{
    player->update(dt);
    glm::vec2 player_pos = player->getPosition();
    return player_pos;
}

void Sandbox2D::OnUpdate(Hazel::Timestep ts)
{
    HZ_PROFILE_FUNCTION();

    this->deltaTime = ts.GetMilliseconds();
    dt_smooth = (dt_smooth * 0.67f + deltaTime * 0.33f);

    // Update
    m_CameraController.OnUpdate(ts);

    if (Hazel::Input::BeginKeyPress(Hazel::Key::R))
    {
        DestroyGame();
        StartGame();
        return;
    }

    UpdateGame(ts);
    DrawGame(ts);
}

void Sandbox2D::UpdateGame(Hazel::Timestep& ts)
{
    float dt = ts.GetSeconds();

    //glm::vec3 cam_pos = m_CameraController.GetCamera().GetPosition();
    glm::vec3 cam_pos = m_CameraController.GetCameraPosition();
    float zoom = m_CameraController.GetZoomLevel();
#if !DEBUG
    if (zoom > 5.0f)
        m_CameraController.SetZoomLevel(5);
#endif

    objectManager->UpdateStep(dt);
    glm::vec2 player_pos = UpdatePlayer(player, dt, m_CameraController);
    auto lavaPos = UpdateLava(lava, dt, player_pos.y);

    objectManager->updateLevel(cam_pos.y + zoom + LVL_DIST_ADD);
    float delBelow = std::min(lavaPos.y, cam_pos.y - zoom);
    objectManager->removeObjectsBelow(delBelow - LVL_DIST_DEL);

    auto newctr = glm::vec3(
        Interpolate::Linear(cam_pos.x, player_pos.x, dt_smooth * 0.002f),
        Interpolate::Linear(cam_pos.y, player_pos.y, dt_smooth * 0.002f),
        cam_pos.z);
    //m_CameraController.GetCamera().SetPosition(newctr);
    m_CameraController.SetCameraPosition(newctr);

    playerMaxY = static_cast<long>(round(fmax((player->getPosition().y + 38), 0.0f)));

    if (Hazel::Input::BeginMouseButtonPress(Hazel::Mouse::ButtonLeft))
    {
        const glm::vec2& mousePosn{
            (Hazel::Input::GetMouseX() / this->m_ScreenWidth),
            (Hazel::Input::GetMouseY() / this->m_ScreenHeight) };

        const glm::vec2& mousePosf{
            cam_pos.x + (mousePosn.x - 0.5f) * zoom * 2 * m_AspectRatio,
            cam_pos.y + (0.5f - mousePosn.y) * zoom * 2 };
        objectManager->createBox(mousePosf.x, mousePosf.y, 32.0f, 32.0f, { .5f, .5f, .5f, 1.f }, BodyType::dynamicBody, &FixtureData::TEST);
    }

}

void Sandbox2D::DrawGame(Hazel::Timestep &ts)
{
    static bool doDebugDraw = false;
    if (Hazel::Input::BeginKeyPress(Hazel::Key::Minus))
        doDebugDraw = !doDebugDraw;

    static bool doDebugDraw2 = false;
    if (Hazel::Input::BeginKeyPress(Hazel::Key::Equal))
        doDebugDraw2 = !doDebugDraw2;


    if (Hazel::Input::BeginKeyPress(Hazel::Key::P))
    {
        objectManager->getPhysicsMgr()->oneWayPlatforms = !objectManager->getPhysicsMgr()->oneWayPlatforms;
        objectManager->getPhysicsMgr()->oneWayPlatforms2 = !objectManager->getPhysicsMgr()->oneWayPlatforms2;
        objectManager->getPhysicsMgr()->m_numFootContacts = 0;
    }


    // Render
    Hazel::Renderer2D::ResetStats();
    {
        HZ_PROFILE_SCOPE("Renderer Prep");
        //Hazel::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
        Hazel::RenderCommand::SetClearColor({ 0.1451f, 0.1098f, 0.1529f, 1 });
        Hazel::RenderCommand::Clear();
    }

    {
        static float rotation = 0.0f;
        rotation += ts * 50.0f;

        HZ_PROFILE_SCOPE("Renderer Draw");

        Hazel::Renderer2D::BeginScene(m_CameraController.GetCamera());
        for (int i = 0; i < DRAW_LAYER_COUNT; i++)
        {
            objectManager->drawObjects(i);
            if (i == 0) player->draw(3);
        }
        Hazel::Renderer2D::EndScene();

        if (doDebugDraw)
        {
            if (!debugDraw->HasBeginDraw())
                Hazel::Renderer2D::BeginScene(m_CameraController.GetCamera());
            objectManager->getPhysicsMgr()->getPhysicsWorld()->DebugDraw();
            if (!debugDraw->HasBeginDraw())
                Hazel::Renderer2D::EndScene();
        }

        Hazel::Renderer2D::BeginScene(m_CameraController.GetCamera());

        //player->draw(3);
        lava->draw(3);

#if DEBUG
        if (doDebugDraw2)
        {
            auto cds = player->contacts;
            for (std::vector<struct Player::ContactData>::iterator it = cds.begin(); it != cds.end(); ++it)
            {
                glm::vec4 dbgcolor2 = (*it).down ? glm::vec4(0, 1, 0, 1) : ((*it).up ? glm::vec4(1, 0, 0, 1) : glm::vec4(1, 1, 0, 1));
                DebugDraw::DrawRay((*it).pos, (*it).normal, dbgcolor2);
            }

            const float epsilon = 0.1f;
            //auto plyr_pos = player->GetBody()->GetPosition();
            //auto plyr_velo = player->GetBody()->GetLinearVelocity();
            auto plyr_fixlist = player->GetBody()->GetFixtureList();
            if (plyr_fixlist)
            {
                auto plyr_aabb = plyr_fixlist->GetAABB(0);
                auto plyr_mins = plyr_aabb.GetCenter() - plyr_aabb.GetExtents();
                DebugDraw::DrawRay({ plyr_mins.x * RATIO, plyr_mins.y * RATIO }, { 0, -0.5f }, { 1,0,1,1 });

                auto objectList = objectManager->getObjectList();
                for (std::list<GameObject*>::iterator iter = objectList.begin(); iter != objectList.end(); ++iter)
                {
                    auto obj_body = (*iter)->GetBody();
                    if (obj_body)
                    {
                        auto obj_fixlist = obj_body->GetFixtureList();
                        if (obj_fixlist)
                        {
                            auto obj_aabb = obj_fixlist->GetAABB(0);
                            auto obj_maxs = obj_aabb.GetCenter() + obj_aabb.GetExtents();

                            //auto below = (plyr_mins.y < obj_maxs.y - epsilon);

                            DebugDraw::DrawRay({ obj_maxs.x * RATIO, obj_maxs.y * RATIO }, { 0, 0.5f }, { 0,1,1,1 });
                            DebugDraw::DrawRay({ obj_maxs.x * RATIO, (obj_maxs.y - epsilon) * RATIO }, { 0.5f, 0 }, { 0,1,1,1 });
                        }
                    }
                }
            }
        }
#endif

        Hazel::Renderer2D::EndScene();
    }
}

void Sandbox2D::OnImGuiRender()
{
    HZ_PROFILE_FUNCTION();

    ImGui::Begin("Info");

    auto stats = Hazel::Renderer2D::GetStats();
    ImGui::Text("Renderer2D Stats:");
    ImGui::Text("  Draw Calls: %d", stats.DrawCalls);
    ImGui::Text("  Quads: %d", stats.QuadCount);
    ImGui::Text("  Vertices: %d", stats.GetTotalVertexCount());
    ImGui::Text("  Indices: %d", stats.GetTotalIndexCount());

    ImGui::Separator();

    ImGui::Text("Objects Stats:");
    ImGui::Text("  objectCount: %d", objectManager->getObjectCount());

    ImGui::Separator();

    ImGui::Text("Physics Stats:");
    ImGui::Text("  objectCount: %d", objectManager->getPhysicsMgr()->getObjectCount());
    ImGui::Text("  bodyCount: %d", objectManager->getPhysicsMgr()->getPhysicsWorld()->GetBodyCount());
    ImGui::Text("  GetContactCount: %d", objectManager->getPhysicsMgr()->getPhysicsWorld()->GetContactCount());
    ImGui::Text("  GetJointCount: %d", objectManager->getPhysicsMgr()->getPhysicsWorld()->GetJointCount());
    ImGui::Text("  GetProxyCount: %d", objectManager->getPhysicsMgr()->getPhysicsWorld()->GetProxyCount());
    ImGui::Text("  GetContactCount: %d", objectManager->getPhysicsMgr()->getPhysicsWorld()->GetSubStepping());

    ImGui::Separator();

    ImGui::Text("Stats:");
    ImGui::Text("  deltaTime: %.2f", (dt_smooth));
    ImGui::Text("  FPS: %.0f", (1.0f / (dt_smooth * 0.001f)));

    ImGui::Separator();

    ImGui::Text("Player:");
    ImGui::Text("  playerMaxY: %d", (playerMaxY));
    ImGui::Text("  grounded: %d", (player->grounded));
    ImGui::Text("  wallLeft: %d", (player->wallLeft));
    ImGui::Text("  wallRight: %d", (player->wallRight));
    ImGui::Text("  ceiling: %d", (player->ceiling));
    ImGui::Text("  inside: %d", (player->inside));
    ImGui::Text("  velocity: %.1f, %.1f", player->GetBody()->GetLinearVelocity().x, player->GetBody()->GetLinearVelocity().y);

    glm::vec3 ctr = m_CameraController.GetCameraPosition();
    ImGui::Text("  cam.y: %.1f", (ctr.y));
    ImGui::Text("  player.y: %.1f", (player->getPosition().y));
    auto lavaPos = lava->getPosition({ 0.5f, 0.5f });
    ImGui::Text("  lava.y: %.1f", (lavaPos.y));


    ImGui::Separator();

    ImGui::Text("Misc:");
    ImGui::Text("  oneWayPlatforms: %d", (objectManager->getPhysicsMgr()->oneWayPlatforms));
    ImGui::Text("  oneWayPlatforms2: %d", (objectManager->getPhysicsMgr()->oneWayPlatforms2));
    ImGui::Text("  m_numFootContacts: %d", (objectManager->getPhysicsMgr()->m_numFootContacts));

    //ImGui::ColorEdit4("Square Color", glm::value_ptr(m_SquareColor));
    ImGui::End();
}

void Sandbox2D::OnEvent(Hazel::Event& e)
{
    m_CameraController.OnEvent(e);

    Hazel::EventDispatcher dispatcher(e);
    dispatcher.Dispatch<Hazel::WindowResizeEvent>(HZ_BIND_EVENT_FN(Sandbox2D::OnWindowResized));
}

bool Sandbox2D::OnWindowResized(Hazel::WindowResizeEvent& e)
{
    HZ_PROFILE_FUNCTION();
    OnResize(float(e.GetWidth()), float(e.GetHeight()));
    return false;
}

void Sandbox2D::OnResize(float width, float height)
{
    m_ScreenWidth = width;
    m_ScreenHeight = height;
    m_AspectRatio = width / height;
}

