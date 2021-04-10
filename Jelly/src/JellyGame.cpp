#include "JellyGame.h"
#include <imgui/imgui.h>

using namespace Jelly;

// re-declare statics
ObjectManager* Jelly::JellyGame::objectManager;
AudioManager* Jelly::JellyGame::audioManager;

JellyGame::JellyGame()
    : Layer("JellyGame"), m_CameraController(1280.0f / 720.0f, true), debugDraw(nullptr), player(nullptr), lava(nullptr), clockStart(0)
{
    instance = this;
    m_CameraController.SetZoomLevel(2.5f);
    m_CameraController.OnResize(1280, 720);
}

JellyGame::~JellyGame()
{
    instance = nullptr;
}

void JellyGame::OnAttach()
{
    HZ_PROFILE_FUNCTION();

    OnResize(float(Hazel::Application::Get().GetWindow().GetWidth()), float(Hazel::Application::Get().GetWindow().GetHeight()));
    m_CameraController.OnResize(m_ScreenWidth, m_ScreenHeight);

    audioManager = new AudioManager();

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
    delete audioManager;
    DestroyGame();
    HZ_PROFILE_FUNCTION();
}

void JellyGame::StartGame()
{
    blackFadeAlpha = 1.0f;

    m_ParticleSystem.Clear();
    m_CameraController.SetCameraPosition({ 0,0,0 });

    objectManager = new ObjectManager();
    //physicsMgr = new PhysicsManager();
    auto physicsMgr = objectManager->GetPhysicsMgr();

    b2World* pWorld = physicsMgr->GetPhysicsWorld();

    pWorld->SetGravity({ 0, -9.81f });

    pWorld->SetContactListener(physicsMgr);

    debugDraw = new DebugDraw(pWorld);
    pWorld->SetDebugDraw(debugDraw);

    //glm::vec3 ctr = m_CameraController.GetCamera().GetPosition();
    objectManager->GenerateLevel(0);

    float startX =
#if DEBUG
        - 150;
#else
        0;
#endif
#if DEBUG
    objectManager->CreateEnemy(-startX, 300 - (20 + 13), 40);
#endif
    this->player = objectManager->CreatePlayer(startX, -13, 50);

    this->lava = objectManager->CreateLava(0, -800, 600 * 4, 200 * 4);

    playerScoreBestMaxY_prev = playerScoreBestMaxY;
    playerScoreY = 0;
    playerScoreMaxY = 0;
    //playerScoreBestMaxY = 0;
    newBest = playerScoreBestMaxY_prev <= 0;

    clockStart = clock();
    startedMove = false;
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


#define DRAW_LAYER_COUNT 5

#define LVL_DIST_ADD 2.0f
#define LVL_DIST_DEL 4.0f


#define PARTICLES_DX 16.0f
#define PARTICLES_DY 1.5f
#define LAVA_MOVE_SPEED 1.0f
#define LAVA_MAX_DIST (400.0f)


glm::vec2 UpdateLava(GameObject* lava, float dt, float viewY, bool move)
{
#if DEBUG
    viewY -= 3.f;
#endif
    lava->Update(dt);
    glm::vec2 lavaPos = lava->GetPosition({ 0.5f, 0.05f });

    float lavaAddSpeed = 0;
    if (lavaPos.y < viewY - (LAVA_MAX_DIST * LVL_SCALE))
        lavaAddSpeed = -(lavaPos.y - (viewY - (LAVA_MAX_DIST * LVL_SCALE)));
    if (lavaPos.y > viewY)
        lavaAddSpeed = -(lavaPos.y - viewY);

    lava->GetBody()->SetLinearVelocity(b2Vec2(0, !move ? 0 : (LAVA_MOVE_SPEED + lavaAddSpeed)));

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

    static bool restarting = false;
    if (Hazel::Input::BeginKeyPress(Hazel::Key::R))
    {
        restarting = true;
        //DestroyGame();
        //StartGame();
        //return;
    }

    const float fadeSpeed = 5.0f;
    if (restarting)
    {
        blackFadeAlpha += ts.GetSeconds() * fadeSpeed;
        if (blackFadeAlpha >= 1.0f)
        {
            blackFadeAlpha = 1.0f;
            restarting = false;
            DestroyGame();
            StartGame();
        }
    }
    else if (blackFadeAlpha > 0.0f)
    {
        blackFadeAlpha -= ts.GetSeconds() * fadeSpeed;
    }

    UpdateGame(ts);
    DrawGame(ts);
}

void JellyGame::UpdateGame(Hazel::Timestep& ts)
{
    float dt = ts.GetSeconds();

    auto runSeconds = ((double)(clock() - clockStart) / CLOCKS_PER_SEC);
    if (!startedMove) runSeconds = 0;


    if (Hazel::Input::BeginKeyPress(Hazel::Key::B))
        screenShake = 0.2f;

    float camAngle = (float)sin(runSeconds * 0.67) * 1.5f;
    if (screenShake > 0)
    {
        screenShake -= ts.GetSeconds();
        camAngle = Interpolate::Linearf(m_CameraController.GetCameraRotation(),
            (Random::Float() - 0.5f) * (screenShake + 0.2f) * 25,
                                        0.1f);
}
    m_CameraController.SetCameraRotation(camAngle);
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

    auto lavaPos = UpdateLava(lava, dt, player_pos.y, startedMove);
    UpdateLavaParticles(lavaParticle, lavaPos, dt);

    auto lvly = player_pos.y < 2 ? player_pos.y : cam_pos.y; // use player on start (ignore restart cam movement)
    objectManager->UpdateLevel(lvly + zoom + LVL_DIST_ADD);
    float delBelow = std::min(lavaPos.y, cam_pos.y - zoom);
    objectManager->RemoveObjectsBelow(delBelow - LVL_DIST_DEL);

    float camyoff = 0.5f - (1.0f - clamp01((player_pos.y - lavaPos.y - 1.0f) * 0.5f));

    static bool fixedCamPos = false;
    if (Hazel::Input::BeginKeyPress(Hazel::Key::C))
        fixedCamPos = !fixedCamPos;
    glm::vec2 look_pos = { player_pos.x, player_pos.y + camyoff };
    const float look_limit = 0.5f;
    float llf = clamp01(abs(look_pos.x) - look_limit);
    look_pos.x = fixedCamPos ? 0.0f : Interpolate::Linearf(look_pos.x, clamp(look_pos.x, -look_limit, look_limit), llf * 0.5f);
    if (look_pos.y < 1.2f) look_pos.y = 1.2f;

    auto newctr = glm::vec3(
        Interpolate::Linear(cam_pos.x, look_pos.x, dt * 2.f),
        Interpolate::Linear(cam_pos.y, look_pos.y, dt * 2.f),
        cam_pos.z);
    //m_CameraController.GetCamera().SetPosition(newctr);
    m_CameraController.SetCameraPosition(newctr);

    playerScoreY = static_cast<ulong>(round(max(player->GetPosition().y * 2, 0.f)));
    if (!startedMove && dt > 0)
    {
        if (playerScoreY > playerScoreMaxY)
        {
            startedMove = true;
            clockStart = clock();
        }
    }

    playerScoreMaxY = max(playerScoreMaxY, playerScoreY);
    if (playerScoreMaxY >= playerScoreBestMaxY && !newBest)
    {
        newBest = true;
        AudioManager::PlaySoundType(Sounds::Powerup);
    }
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

#if DEBUG
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
#endif

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
        if (++oneway > 4) oneway = 0;
        objectManager->GetPhysicsMgr()->oneWayPlatforms = oneway;
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
            if (i == 0) player->Draw(DRAW_LAYER_COUNT);
            if (i == DRAW_LAYER_COUNT) lava->Draw(DRAW_LAYER_COUNT);
        }

        m_ParticleSystem.OnRender();

        if (playerScoreBestMaxY_prev > 0 && blackFadeAlpha <= 0.0f)
        {
            glm::vec4 bcolor = playerScoreMaxY >= playerScoreBestMaxY_prev ? glm::vec4(0, 1, 0, 0.5f) : glm::vec4(1, 0, 0, 0.5f);
            DebugDraw::DrawRay({ -10, playerScoreBestMaxY_prev * 0.5f - 0.5f }, { 20.0f, 0 }, bcolor);
        }

        Hazel::Renderer2D::EndScene();

        if (blackFadeAlpha > 0.0f)
        {
            glm::vec3 cam_pos = m_CameraController.GetCameraPosition();
            cam_pos.z = 0.999f;
            float zoom = m_CameraController.GetZoomLevel();

            Hazel::Renderer2D::BeginScene(m_CameraController.GetCamera());
            Hazel::Renderer2D::DrawQuad(cam_pos, { m_ScreenWidth * zoom, m_ScreenHeight * zoom }, { 0.0f, 0.0f, 0.0f, blackFadeAlpha });
            Hazel::Renderer2D::EndScene();
        }

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

            auto objectList = objectManager->GetObjectList();
            for (std::list<GameObject*>::iterator iter = objectList.begin(); iter != objectList.end(); ++iter)
                (*iter)->DebugDraw();
            player->DebugDraw();

            auto plyr_body = player->GetBody();
            if (plyr_body && plyr_body->IsEnabled())
            {
                const float epsilon = 0.1f;
                //auto plyr_pos = plyr_body->GetPosition();
                //auto plyr_velo = plyr_body->GetLinearVelocity();
                auto plyr_fixlist = plyr_body->GetFixtureList();
                if (plyr_fixlist)
                {
                    auto plyr_aabb = plyr_fixlist->GetAABB(0);
                    auto plyr_extents = plyr_aabb.GetExtents();
                    auto plyr_mins = plyr_aabb.GetCenter() - plyr_extents;
                    DebugDraw::DrawRay({ plyr_mins.x * RATIO, plyr_mins.y * RATIO }, { plyr_extents.x * RATIO * 2,  plyr_extents.y * RATIO * 2 }, { 1,0,1,1 });

                    for (std::list<GameObject*>::iterator iter = objectList.begin(); iter != objectList.end(); ++iter)
                    {
                        auto obj_body = (*iter)->GetBody();
                        if (obj_body && obj_body->IsEnabled())
                        {
                            auto obj_fixlist = obj_body->GetFixtureList();
                            if (obj_fixlist)
                            {
                                auto obj_aabb = obj_fixlist->GetAABB(0);
                                auto obj_maxs = obj_aabb.GetCenter() + obj_aabb.GetExtents();

                                //auto below = (plyr_mins.y < obj_maxs.y - epsilon);

                                //DebugDraw::DrawRay({ obj_maxs.x * RATIO, obj_maxs.y * RATIO }, { 0, 0.5f }, { 0,1,1,1 });
                                DebugDraw::DrawRay({ obj_maxs.x * RATIO, (obj_maxs.y - epsilon) * RATIO }, { 0.5f, 0 }, { 0,1,1,1 });
                            }
                        }
                    }
                }
            }

            glm::vec2 lavaPos = lava->GetPosition({ 0.5f, 0.05f });
            auto ppos = lavaPos;
            auto psize = glm::vec2(PARTICLES_DX * .5f, PARTICLES_DY * .5f);
            ppos.y = ppos.y - (psize.y + 0.08f);

            //DebugDraw::DrawLine(ppos - psize, ppos + psize, { 1, 1, 0, 1 });
            DebugDraw::DrawLineRect(ppos - psize, ppos + psize, { 1, 1, 0, 1 });
            //DebugDraw::DrawLine(lavaPos, lavaPos + lava->GetSize() * .5f, { 1, 0, 1, 1 });


            DebugDraw::DrawLine(lava->GetPosition({ 0.0f, 0.0f }), lava->GetPosition({ 1.f, 1.f }), { 1, 0, 1, 1 });
            DebugDraw::DrawLine(lava->GetPosition(), lava->GetPosition({ 0.0f, 0.0f }), { 0, 1, 0, 1 });

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

#define ARG_VEC2(v) v.x, v.y

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
        if (!startedMove) runSeconds = 0;
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
    SetWindowPos(3);
    ImGui::SetNextWindowBgAlpha(0.5f);
    auto window_flags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_AlwaysAutoResize |
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
        ImGui::Text("  pos: %.1f, %.1f", ARG_VEC2(player->GetPosition()));
        ImGui::Text("  velocity: %.1f, %.1f", ARG_VEC2(player->GetBody()->GetLinearVelocity()));
        ImGui::Text("  speed: %.2f", (player->speed * 10));
        ImGui::Text("  grounded: %d", (player->grounded));
        ImGui::Text("  wallLeft: %d", (player->wallLeft));
        ImGui::Text("  wallRight: %d", (player->wallRight));
        ImGui::Text("  ceiling: %d", (player->ceiling));
        ImGui::Text("  inside: %d", (player->inside));
        ImGui::Text("  damping: %.1f", player->GetBody()->GetLinearDamping());

        ImGui::Separator();

        ImGui::Text("Misc:");
        glm::vec3 ctr = m_CameraController.GetCameraPosition();
        auto zoom = m_CameraController.GetZoomLevel();
        ImGui::Text("  campos: %.1f, %.1f", ARG_VEC2(ctr));
        ImGui::Text("  zoom: %.1f", (zoom));
        auto lavaPos = lava->GetPosition({ 0.5f, 0.05f });
        ImGui::Text("  lava.y: %.1f", (lavaPos.y));

        ImGui::Text("  oneWayPlatforms: %d", (objectManager->GetPhysicsMgr()->oneWayPlatforms));

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

