#include "engine.h"
#include "Shaders.h"
#include <stdio.h>
#include <SDL3/SDL_events.h>
#include <glm/gtc/matrix_transform.hpp>

void Engine::init()
{
	printf("Init engine \n");

	m_triangle = 
	{
		{ {-1.0f, -1.0f, 0.0f}, {1, 0, 0} },
		{ { 1.0f, -1.0f, 0.0f}, {0, 1, 0} },
		{ { 0.0f,  1.0f, 0.0f}, {0, 0, 1} },
	};
	
	const vec3 pos[8] = {
		{-1, -1, -1},   // 0
		{ 1, -1, -1},   // 1
		{-1,  1, -1},   // 2
		{ 1,  1, -1},   // 3
		{-1, -1,  1},   // 4
		{ 1, -1,  1},   // 5
		{-1,  1,  1},   // 6
		{ 1,  1,  1},   // 7
	};

	const vec3 col[8] = {
		{0, 0, 0},   // 0 noir
		{1, 0, 0},   // 1 rouge
		{0, 1, 0},   // 2 vert
		{1, 1, 0},   // 3 jaune
		{0, 0, 1},   // 4 bleu
		{1, 0, 1},   // 5 magenta
		{0, 1, 1},   // 6 cyan
		{1, 1, 1},   // 7 blanc
	};

	const int idx[36] = {
		4, 5, 6,   6, 5, 7,   // avant  (+Z)
		1, 0, 3,   3, 0, 2,   // arrière (-Z)
		5, 1, 7,   7, 1, 3,   // droite (+X)
		0, 4, 2,   2, 4, 6,   // gauche (-X)
		6, 7, 2,   2, 7, 3,   // haut   (+Y)
		0, 1, 4,   4, 1, 5,   // bas    (-Y)
	};

	m_cube.clear();
	for (int i = 0; i < 36; ++i)
	{
		m_cube.push_back({ pos[idx[i]], col[idx[i]] });
	}

	m_pipeline.BindShader(BasicShader);

	float aspect = float(WIDTH) / float(HEIGHT);
	
	// m_camera.SetOrthographic(10.0f * aspect, 10.0f, 0.1f, 100.0f);
	m_camera.SetPerspective(radians(45.0f), (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);
	m_camera.SetMatrix(translate(mat4(1.0f), vec3(0.0f, 0.0f, 5.0f)));
	
	m_emitter = new Emitter({0.0f, 0.0f, 0.0f}, 1000);
	m_emitter->Init();
}

void Engine::Update(float _dt)
{
	const bool* keys = SDL_GetKeyboardState(nullptr);

	const float speed = 3.0f;
	const float turn  = 1.5f;

	if (keys[SDL_SCANCODE_LEFT])  m_camYaw   += turn * _dt;
	if (keys[SDL_SCANCODE_RIGHT]) m_camYaw   -= turn * _dt;
	if (keys[SDL_SCANCODE_UP])    m_camPitch += turn * _dt;
	if (keys[SDL_SCANCODE_DOWN])  m_camPitch -= turn * _dt;

	mat4 rot = rotate(mat4(1.0f), m_camYaw,   vec3(0, 1, 0))
			 * rotate(mat4(1.0f), m_camPitch, vec3(1, 0, 0));

	vec3 right   = vec3(rot[0]);
	vec3 forward = -vec3(rot[2]); 

	if (keys[SDL_SCANCODE_W]) m_camPos += forward * speed * _dt;
	if (keys[SDL_SCANCODE_S]) m_camPos -= forward * speed * _dt;
	if (keys[SDL_SCANCODE_D]) m_camPos += right   * speed * _dt;
	if (keys[SDL_SCANCODE_A]) m_camPos -= right   * speed * _dt;

	mat4 world = translate(mat4(1.0f), m_camPos) * rot;
	m_camera.SetMatrix(world);            

	if (m_emitter != nullptr)
		m_emitter->Update(_dt);
}

void Engine::handleEvent(const SDL_Event& event)
{
    if (event.type == SDL_EVENT_KEY_DOWN)
    {
        if ( event.key.key == SDLK_P )
        {
            m_emitter->m_active = !m_emitter->m_active;
        }
    }
}

void Engine::renderFrame(FinalFrameBuffer& frameBufferOut)
{
	m_time += 1.0f / 60.0f; // faut faire un vrai delta time

	frameBufferOut.clear(0x000000);
	m_pipeline.ClearDepth();
	m_pipeline.BindFrameBuffer(&frameBufferOut);

	Uniforms uniforms;
	mat4 model = mat4(1.0f);
	uniforms.mvp = m_camera.GetVPMatrix() * model;
	
	m_pipeline.BindVertexBuffer(&m_triangle);
	m_pipeline.BindUniforms(uniforms);
	m_pipeline.Draw();
	
	m_pipeline.BindVertexBuffer(&m_cube);
	m_pipeline.BindUniforms(uniforms);
	m_pipeline.Draw();

	m_particleVertices.clear();
	if (m_emitter)
		m_emitter->Render(m_particleVertices);

	uniforms.mvp = m_camera.GetVPMatrix();
	m_pipeline.BindVertexBuffer(&m_particleVertices);
	m_pipeline.BindUniforms(uniforms);
	m_pipeline.Draw();
}

void Engine::destroy()
{
	printf("Killing engine \n");
}
