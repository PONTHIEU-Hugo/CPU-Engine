#include "ParticleScene.h"

static const float BASE_Y = -2.0f;
static const float BURST_PERIOD = 1.5f;

void ParticleScene::Init()
{
    EnableSkybox(false);

    float aspect = float(WIDTH) / float(HEIGHT);
    camera = new Camera({ 0.0f, 0.0f, 12.0f }, nullptr);
    camera->SetPerspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);
    m_transforms.push_back(&camera->transform);

    sun.shadowFocus = glm::vec3(0.0f);

    // Fumee a gauche
    m_smoke.SetOrigin({ -5.0f, BASE_Y, 0.0f });

    // Neige au centre : colonne de 2.5 de rayon qui tombe de 6 unites au-dessus du sol
    m_snow.settings.origin = { 0.0f, BASE_Y + 6.0f, 0.0f };
    m_snow.settings.radius = 2.5f;
    m_snow.settings.emissionRate = 100.0f;
    m_snow.settings.lifeMin = 4.0f;
    m_snow.settings.lifeMax = 5.0f;
    m_snow.settings.colorStart = { 255.0f, 255.0f, 255.0f, 255.0f };
    m_snow.settings.colorEnd = { 160.0f, 200.0f, 255.0f, 255.0f };
    m_snow.groundHeight = BASE_Y;

    // Etincelles a droite : fontaine continue + rafale periodique
    m_sparkle.SetOrigin({ 5.0f, 0.0f, 0.0f });

    m_smoke.Init();
    m_snow.Init();
    m_sparkle.Init();

    // Alpha d'abord, additif en dernier
    AddEmitter(m_smoke);
    AddEmitter(m_snow);
    AddEmitter(m_sparkle);
}

void ParticleScene::Update(float _dt)
{
    camera->HandleEvent(_dt);
    HandleEvent(_dt);

    m_burstTimer += _dt;
    if (m_burstTimer >= BURST_PERIOD)
    {
        m_burstTimer = 0.0f;
        if (m_sparkle.IsActive())
            m_sparkle.Burst(150);
    }

    m_smoke.Update(_dt);
    m_snow.Update(_dt);
    m_sparkle.Update(_dt);

    Scene::Update(_dt);
}

void ParticleScene::HandleEvent(float /*_dt*/)
{
    const bool* keys = SDL_GetKeyboardState(nullptr);

    // P : pause / reprise de l'emission
    bool pDown = keys[SDL_SCANCODE_P];
    if (pDown && m_pWasDown == false)
    {
        bool active = m_smoke.IsActive() == false;
        m_smoke.SetActive(active);
        m_snow.SetActive(active);
        m_sparkle.SetActive(active);
    }
    m_pWasDown = pDown;

    // B : rafale d'etincelles
    bool bDown = keys[SDL_SCANCODE_B];
    if (bDown && m_bWasDown == false)
        m_sparkle.Burst(200);
    m_bWasDown = bDown;
}
