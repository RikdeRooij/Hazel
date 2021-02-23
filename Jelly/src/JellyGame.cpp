#include "JellyGame.h"
#include <imgui/imgui.h>

using namespace Jelly;

// re-declare statics
ObjectManager* Jelly::JellyGame::objectManager;

JellyGame::JellyGame()
    : Layer("JellyGame"), m_CameraController(1280.0f / 720.0f, true), debugDraw(nullptr), player(nullptr), lava(nullptr), clockStart(0)
{
    m_CameraController.SetZoomLevel(2.5f);
    m_CameraController.OnResize(1280, 720);
}

void JellyGame::OnAttach()
{
    HZ_PROFILE_FUNCTION();

    OnResize(float(Hazel::Application::Get().GetWindow().GetWidth()), float(Hazel::Application::Get().GetWindow().GetHeight()));
    m_CameraController.OnResize(m_ScreenWidth, m_ScreenHeight);

    // Init here
    lavaParticle.LifeTime = 1.0f;
    lavaParticle.Velocity = { -0.2f, 0.4f };
    lavaParticle.VelocityVariation = { 0.3f, 0.3f };
    lavaParticle.Position = { 0.0f, 0.0f };

    lavaParticle.ColorBegin = { 1,1,1, 1.0f };
    lavaParticle.ColorEnd = { 1,1,1, 1.0f };

    lavaParticle.SizeBegin = 0.2f, lavaParticle.SizeVariation = 0.2f, lavaParticle.SizeEnd = 0.3f;

    lavaParticle.Animation = TextureAtlas("assets/bubble.xml");

#if !DEBUG
    std::srand(std::time(nullptr));
#else
    std::srand(50);
#endif
    StartGame();
}

void JellyGame::OnDetach()
{
    DestroyGame();
    HZ_PROFILE_FUNCTION();
}

void JellyGame::StartGame()
{
    objectManager = new ObjectManager();
    //physicsMgr = new PhysicsManager();
    auto physicsMgr = objectManager->GetPhysicsMgr();

    b2World* pWorld = physicsMgr->GetPhysicsWorld();

    pWorld->SetGravity({ 0, -9.81f });

    pWorld->SetContactListener(physicsMgr);

    debugDraw = new DebugDraw(pWorld);
    pWorld->SetDebugDraw(debugDraw);

    glm::vec3 ctr = m_CameraController.GetCamera().GetPosition();
    objectManager->GenerateLevel(ctr.y);

    float startX =
#if DEBUG
        -250;
#else
        0;
#endif
    this->player = objectManager->CreatePlayer(startX, 0, 50);

    this->lava = objectManager->CreateLava(0, -800, 600 * 4, 200 * 4);

    playerScoreY = 0;
    playerScoreMaxY = 0;
    //playerScoreBestMaxY = 0;

    clockStart = clock();
}

void JellyGame::DestroyGame() const
{
    delete lava;
    delete player;
    delete debugDraw;
    //delete physicsMgr;
    //physicsMgr = NULL;
    delete objectManager;
    objectManager = nullptr;
}


#define DRAW_LAYER_COUNT 3

#define LVL_DIST_ADD 0.0f
#define LVL_DIST_DEL 4.0f


#define PARTICLES_DX 16.0f
#define PARTICLES_DY 1.5f
#define LAVA_MOVE_SPEED 1.0f
#define LAVA_MAX_DIST (400.0f)


glm::vec2 UpdateLava(GameObject* lava, float dt, float viewY)
{
#if DEBUG
    viewY -= 3.f;
#endif
    lava->Update(dt);
    glm::vec2 lavaPos = lava->GetPosition({ 0.5f, 0.5f });

    float lavaAddSpeed = 0;
    if (lavaPos.y < viewY - (LAVA_MAX_DIST * LVL_SCALE))
        lavaAddSpeed = -(lavaPos.y - (viewY - (LAVA_MAX_DIST * LVL_SCALE)));
    if (lavaPos.y > viewY)
        lavaAddSpeed = -(lavaPos.y - viewY);

    lava->GetBody()->SetLinearVelocity(b2Vec2(0, (LAVA_MOVE_SPEED + lavaAddSpeed)));

    auto lavaoffs = lava->GetTilingOffset();
    lavaoffs.x += dt * 0.08f;
    lava->SetTilingOffset(lavaoffs);

    return lavaPos;
}

void UpdateLavaParticles(ParticleProps& particle, const glm::vec2& lavaPos, float dt)
{
    static float s_lastParticleTime = 0;
    s_lastParticleTime -= dt;
    if (s_lastParticleTime < 0)
    {
        s_lastParticleTime = 0.01f + Random::Float() * 0.7f;
        auto ppos = lavaPos;
        ppos.x += (Random::Float() - 0.5f) * PARTICLES_DX;
        float rf = Random::Float();
        ppos.y += ((1.f - rf * rf) - 0.5f - 0.5f) * PARTICLES_DY - 0.08f;
        particle.Position = ppos;
        ParticleSystem::S_Emit(particle);
    }
}

glm::vec2 UpdatePlayer(Player* player, float dt, Hazel::OrthographicCameraController& camCtrl)
{
    player->Update(dt);
    glm::vec2 player_pos = player->GetPosition();
    return player_pos;
}

void JellyGame::OnUpdate(Hazel::Timestep ts)
{
    HZ_PROFILE_FUNCTION();

    auto deltaTime = ts.GetMilliseconds();

    //dt_smooth = (dt_smooth * 0.67f + deltaTime * 0.33f);

    avg_counter++;
    dt_next += deltaTime;
    if (dt_next >= 500.0f)
    {
        avg_fps = (avg_counter / (dt_next * 0.001f));
        dt_next = 0;
        avg_counter = 0;
    }

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

void JellyGame::UpdateGame(Hazel::Timestep& ts)
{
    float dt = ts.GetSeconds();

    auto runSeconds = ((double)(clock() - clockStart) / CLOCKS_PER_SEC);
    m_CameraController.SetCameraRotation((float)sin(runSeconds * 0.67) * 1.5f);
    lava->GetBody()->SetTransform(lava->GetBody()->GetPosition(), ((float)sin(runSeconds * 0.67) * 1.5f * DEG2RAD));

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
    UpdateLavaParticles(lavaParticle, lavaPos, dt);

    objectManager->UpdateLevel(cam_pos.y + zoom + LVL_DIST_ADD);
    float delBelow = std::min(lavaPos.y, cam_pos.y - zoom);
    objectManager->RemoveObjectsBelow(delBelow - LVL_DIST_DEL);

    auto velo = player->GetBody()->GetLinearVelocity();
    const float velo_power = 0;
    glm::vec2 look_pos = { player_pos.x + velo.x * RATIO * velo_power, player_pos.y + 0.5f + velo.y * RATIO * velo_power };
    auto newctr = glm::vec3(
        Interpolate::Linear(cam_pos.x, look_pos.x, dt * 2.f),
        Interpolate::Linear(cam_pos.y, look_pos.y, dt * 2.f),
        cam_pos.z);
    //m_CameraController.GetCamera().SetPosition(newctr);
    m_CameraController.SetCameraPosition(newctr);

    playerScoreY = static_cast<ulong>(round(max(player->GetPosition().y * 2, 0.f)));
    playerScoreMaxY = max(playerScoreMaxY, playerScoreY);
    playerScoreBestMaxY = max(playerScoreBestMaxY, playerScoreMaxY);

    if (Hazel::Input::BeginMouseButtonPress(Hazel::Mouse::ButtonLeft))
    {
        const glm::vec2& mousePosn{
            (Hazel::Input::GetMouseX() / this->m_ScreenWidth),
            (Hazel::Input::GetMouseY() / this->m_ScreenHeight) };

        const glm::vec2& mousePosf{
            cam_pos.x + (mousePosn.x - 0.5f) * zoom * 2 * m_AspectRatio,
            cam_pos.y + (0.5f - mousePosn.y) * zoom * 2 };
        objectManager->CreateBox(mousePosf.x, mousePosf.y, 32.0f, 32.0f, { .5f, .5f, .5f, 1.f }, BodyType::dynamicBody, &FixtureData::TEST);
    }

    if (Hazel::Input::IsMouseButtonPressed(Hazel::Mouse::ButtonRight))
    {
        auto mp = Hazel::Input::GetMousePosition();
        auto x = mp.x;
        auto y = mp.y;
        auto width = m_ScreenWidth;
        auto height = m_ScreenHeight;

        glm::vec4 bounds = { -m_AspectRatio * zoom, m_AspectRatio * zoom, -zoom, zoom };
        auto boundsWidth = bounds.y - bounds.x;
        auto boundsHeight = bounds.w - bounds.z;
        auto pos = m_CameraController.GetCamera().GetPosition();
        x = (x / width) * boundsWidth - boundsWidth * 0.5f;
        y = boundsHeight * 0.5f - (y / height) * boundsHeight;
        lavaParticle.Position = { x + pos.x, y + pos.y };
        for (int i = 0; i < 5; i++)
        {
            lavaParticle.Position = { x + pos.x + (Random::Float() - 0.5f) * 3, y + pos.y + (Random::Float() - 0.5f) * 3 };
            m_ParticleSystem.Emit(lavaParticle);
        }
    }

    m_ParticleSystem.OnUpdate(ts);
}

void JellyGame::DrawGame(Hazel::Timestep &ts)
{
    static bool doDebugDraw = false;
    if (Hazel::Input::BeginKeyPress(Hazel::Key::Minus))
        doDebugDraw = !doDebugDraw;

    static bool doDebugDraw2 = false;
    if (Hazel::Input::BeginKeyPress(Hazel::Key::Equal))
        doDebugDraw2 = !doDebugDraw2;


    if (Hazel::Input::BeginKeyPress(Hazel::Key::P))
    {
        auto oneway = objectManager->GetPhysicsMgr()->oneWayPlatforms;
        if (++oneway > 2) oneway = 0;
        objectManager->GetPhysicsMgr()->oneWayPlatforms = oneway;
        objectManager->GetPhysicsMgr()->m_numFootContacts = 0;
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
        HZ_PROFILE_SCOPE("Renderer Draw");

        Hazel::Renderer2D::BeginScene(m_CameraController.GetCamera());
        for (int i = 0; i <= DRAW_LAYER_COUNT; i++)
        {
            objectManager->DrawObjects(i);
            if (i == 0) player->Draw(3);
            if (i == DRAW_LAYER_COUNT) lava->Draw(3);
        }

        m_ParticleSystem.OnRender();

        Hazel::Renderer2D::EndScene();


        //Hazel::Renderer2D::BeginScene(m_CameraController.GetCamera());
        //m_ParticleSystem.OnRender();
        //Hazel::Renderer2D::EndScene();

        if (doDebugDraw)
        {
            if (!debugDraw->HasBeginDraw())
                Hazel::Renderer2D::BeginScene(m_CameraController.GetCamera());
            objectManager->GetPhysicsMgr()->GetPhysicsWorld()->DebugDraw();
            if (!debugDraw->HasBeginDraw())
                Hazel::Renderer2D::EndScene();
        }

#if DEBUG
        if (doDebugDraw2)
        {
            Hazel::Renderer2D::BeginScene(m_CameraController.GetCamera());

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

                auto objectList = objectManager->GetObjectList();
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


             glm::vec2 lavaPos = lava->GetPosition({ 0.5f, 0.5f });
             auto ppos = lavaPos;
             auto psize = glm::vec2(PARTICLES_DX * .5f, PARTICLES_DY * .5f);
             ppos.y = ppos.y - (psize.y + 0.08f);
             
             //DebugDraw::DrawLine(ppos - psize, ppos + psize, { 1, 1, 0, 1 });
             DebugDraw::DrawLineRect(ppos - psize, ppos + psize, { 1, 1, 0, 1 });

            Hazel::Renderer2D::EndScene();
        }

        //for (int i = 0; i < 15; i++)
        //{
        //    DebugDraw::DrawRay({ -1, i - 0.5f }, { 2.0f, 0 }, { 0,1,0,1 });
        //}
#endif
    }
}

// 0 = top-left, 1 = top-right, 2 = bottom-left, 3 = bottom-right
ImGuiWindowFlags SetWindowPos(int corner = -1, float xDistance = 10.0f, float yDistance = 10.0f)
{
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImVec2 work_area_pos = viewport->GetWorkPos();   // Instead of using viewport->Pos we use GetWorkPos() to avoid menu bars, if any!
    ImVec2 work_area_size = viewport->GetWorkSize();
    ImVec2 window_pos = ImVec2((corner & 1) ? (work_area_pos.x + work_area_size.x - xDistance) : (work_area_pos.x + xDistance), 
                               (corner & 2) ? (work_area_pos.y + work_area_size.y - yDistance) : (work_area_pos.y + yDistance));
    ImVec2 window_pos_pivot = ImVec2((corner & 1) ? 1.0f : 0.0f, (corner & 2) ? 1.0f : 0.0f);
    ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
    if (corner != -1)
        window_flags |= ImGuiWindowFlags_NoMove;
    return window_flags;
}

void JellyGame::OnImGuiRender()
{
    HZ_PROFILE_FUNCTION();

    if (player && player->dead)
    {
        ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowBgAlpha(0.35f); // Transparent background
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_AlwaysAutoResize |
            ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
        if (ImGui::Begin("GAME OVER", nullptr, window_flags))
        {
            ImGui::Text("Press 'r' to restart");
        }
        ImGui::End();
    }
    
    if (ImGui::Begin("Info", nullptr, SetWindowPos(0)))
    {
        ImGui::Text("  Best Highscore: %d", (playerScoreBestMaxY));
        ImGui::Separator();
        auto runSeconds = ((double)(clock() - clockStart) / CLOCKS_PER_SEC);
        ImGui::Text("  Time: %.1f", (round(runSeconds * 5) * .2));
        ImGui::Text("  Current Highscore: %d", (playerScoreMaxY));
        ImGui::Text("  Score: %d", (playerScoreY));
    }
    ImGui::End();
    
    if (ImGui::Begin("System", nullptr, SetWindowPos(1)))
    {
        ImGui::Text("  FPS: %.0f", (avg_fps));
    }
    ImGui::End();

#if DEBUG
    auto window_flags = SetWindowPos(3);
    ImGui::SetNextWindowBgAlpha(0.5f);
    window_flags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
    if (ImGui::Begin("Debug", nullptr, window_flags))
    {
        auto stats = Hazel::Renderer2D::GetStats();
        ImGui::Text("Renderer2D Stats:");
        ImGui::Text("  Draw Calls: %d", stats.DrawCalls);
        ImGui::Text("  Quads: %d", stats.QuadCount);
        ImGui::Text("  Vertices: %d", stats.GetTotalVertexCount());
        ImGui::Text("  Indices: %d", stats.GetTotalIndexCount());

        ImGui::Separator();

        ImGui::Text("Objects Stats:");
        ImGui::Text("  objectCount: %d", objectManager->GetObjectCount());

        ImGui::Separator();

        ImGui::Text("Physics Stats:");
        ImGui::Text("  objectCount: %d", objectManager->GetPhysicsMgr()->GetObjectCount());
        ImGui::Text("  bodyCount: %d", objectManager->GetPhysicsMgr()->GetPhysicsWorld()->GetBodyCount());
        ImGui::Text("  GetContactCount: %d", objectManager->GetPhysicsMgr()->GetPhysicsWorld()->GetContactCount());
        ImGui::Text("  GetJointCount: %d", objectManager->GetPhysicsMgr()->GetPhysicsWorld()->GetJointCount());
        ImGui::Text("  GetProxyCount: %d", objectManager->GetPhysicsMgr()->GetPhysicsWorld()->GetProxyCount());
        ImGui::Text("  GetContactCount: %d", objectManager->GetPhysicsMgr()->GetPhysicsWorld()->GetSubStepping());

        ImGui::Separator();

        ImGui::Text("Stats:");
        ImGui::Text("  FPS: %.0f", (avg_fps));
        ImGui::Text("  deltaTime: %.2f", (1.0f / (avg_fps * 0.001f)));

        ImGui::Separator();

        ImGui::Text("Player:");
        ImGui::Text("  speed: %.2f", (player->speed * 10));
        ImGui::Text("  grounded: %d", (player->grounded));
        ImGui::Text("  wallLeft: %d", (player->wallLeft));
        ImGui::Text("  wallRight: %d", (player->wallRight));
        ImGui::Text("  ceiling: %d", (player->ceiling));
        ImGui::Text("  inside: %d", (player->inside));
        ImGui::Text("  velocity: %.1f, %.1f", player->GetBody()->GetLinearVelocity().x, player->GetBody()->GetLinearVelocity().y);
        ImGui::Text("  damping: %.1f", player->GetBody()->GetLinearDamping());

        glm::vec3 ctr = m_CameraController.GetCameraPosition();
        auto zoom = m_CameraController.GetZoomLevel();
        ImGui::Text("  cam.y: %.1f", (ctr.y));
        ImGui::Text("  zoom: %.1f", (zoom));

        ImGui::Text("  player.y: %.1f", (player->GetPosition().y));
        auto lavaPos = lava->GetPosition({ 0.5f, 0.5f });
        ImGui::Text("  lava.y: %.1f", (lavaPos.y));


        ImGui::Separator();

        ImGui::Text("Misc:");
        ImGui::Text("  oneWayPlatforms: %d", (objectManager->GetPhysicsMgr()->oneWayPlatforms));
        ImGui::Text("  m_numFootContacts: %d", (objectManager->GetPhysicsMgr()->m_numFootContacts));

        //ImGui::ColorEdit4("Square Color", glm::value_ptr(m_SquareColor));
    }
    ImGui::End();
#endif
}

void JellyGame::OnEvent(Hazel::Event& e)
{
    m_CameraController.OnEvent(e);

    Hazel::EventDispatcher dispatcher(e);
    dispatcher.Dispatch<Hazel::WindowResizeEvent>(HZ_BIND_EVENT_FN(JellyGame::OnWindowResized));
}

bool JellyGame::OnWindowResized(Hazel::WindowResizeEvent& e)
{
    HZ_PROFILE_FUNCTION();
    OnResize(float(e.GetWidth()), float(e.GetHeight()));
    return false;
}

void JellyGame::OnResize(float width, float height)
{
    m_ScreenWidth = width;
    m_ScreenHeight = height;
    m_AspectRatio = width / height;
}

