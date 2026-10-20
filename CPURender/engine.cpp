#include "engine.h"
#include "Shaders.h"
#include <stdio.h>
#include <SDL3/SDL_events.h>
#include <glm/gtc/matrix_transform.hpp>
#include "Chrono.h"

void Engine::init()
{
	printf("Init engine \n");

	m_triangle = 
	{
		{ {-1.0f, -1.0f, 0.0f}, {255, 0, 0} },
		{ { 1.0f, -1.0f, 0.0f}, {0, 255, 0} },
		{ { 0.0f,  1.0f, 0.0f}, {0, 0, 255} },
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
		{255, 0, 0},   // 1 rouge
		{0, 255, 0},   // 2 vert
		{255, 255, 0},   // 3 jaune
		{0, 0, 255},   // 4 bleu
		{255, 0, 255},   // 5 magenta
		{0, 255, 255},   // 6 cyan
		{255, 255, 255},   // 7 blanc
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
		const vec3& p = pos[idx[i]];

		float u = 0.0f, v = 0.0f;

		switch (i / 6)
		{
		case 0: u =  p.x; v =  p.y; break; // avant  (+Z)
		case 1: u = -p.x; v =  p.y; break; // arriere (-Z)
		case 2: u = -p.z; v =  p.y; break; // droite (+X)
		case 3: u =  p.z; v =  p.y; break; // gauche (-X)
		case 4: u =  p.x; v = -p.z; break; // haut   (+Y)
		case 5: u =  p.x; v =  p.z; break; // bas    (-Y)
		}

		Vertex vertex{ p, col[idx[i]] };
		vertex.uv = vec2(u * 0.5f + 0.5f, 0.5f - v * 0.5f);

		m_cube.push_back(vertex);
	}

	m_texture.Load("Assets/doomguy.png");

	m_pipeline.BindShader(BasicShader);

	float aspect = float(WIDTH) / float(HEIGHT);
	
	// m_camera.SetOrthographic(10.0f * aspect, 10.0f, 0.1f, 100.0f);
	m_camera.SetPerspective(radians(45.0f), aspect, 0.1f, 100.0f);
	m_camera.SetMatrix(translate(mat4(1.0f), vec3(0.0f, 0.0f, 10.0f)));
	
	m_emitter = new Emitter({0.0f, 0.0f, 0.0f}, 10);
	m_emitter->Init();
}

void Engine::Update()
{
	const bool* keys = SDL_GetKeyboardState(nullptr);

	const float speed = 3.0f;
	const float turn  = 1.5f;

	float dt = CHRONO->GetDeltaTime();
	if (keys[SDL_SCANCODE_LEFT])  m_camYaw   += turn * dt;
	if (keys[SDL_SCANCODE_RIGHT]) m_camYaw   -= turn * dt;
	if (keys[SDL_SCANCODE_UP])    m_camPitch += turn * dt;
	if (keys[SDL_SCANCODE_DOWN])  m_camPitch -= turn * dt;

	mat4 rot = rotate(mat4(1.0f), m_camYaw,   vec3(0, 1, 0))
			 * rotate(mat4(1.0f), m_camPitch, vec3(1, 0, 0));

	vec3 right   = vec3(rot[0]);
	vec3 forward = -vec3(rot[2]); 

	if (keys[SDL_SCANCODE_W]) m_camPos += forward * speed * dt;
	if (keys[SDL_SCANCODE_S]) m_camPos -= forward * speed * dt;
	if (keys[SDL_SCANCODE_D]) m_camPos += right   * speed * dt;
	if (keys[SDL_SCANCODE_A]) m_camPos -= right   * speed * dt;

	mat4 world = translate(mat4(1.0f), m_camPos) * rot;
	m_camera.SetMatrix(world);            

	if (m_emitter != nullptr)
		m_emitter->Update();
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
	frameBufferOut.clear(0x000000);
	m_pipeline.ClearDepth();
	m_pipeline.BindFrameBuffer(&frameBufferOut);

	Uniforms uniforms;
	mat4 model = mat4(1.0f);
	uniforms.mvp = m_camera.GetVPMatrix() * model;
	
	m_pipeline.BindShader(BasicShader);
	m_pipeline.BindVertexBuffer(&m_triangle);
	m_pipeline.BindUniforms(uniforms);
	m_pipeline.Draw();

	// Cube
	uniforms.texture = &m_texture;
	m_pipeline.BindShader(TextureShader);
	m_pipeline.BindVertexBuffer(&m_cube);
	m_pipeline.BindUniforms(uniforms);
	m_pipeline.Draw();
	uniforms.texture = nullptr;

	m_pipeline.BindShader(BasicShader);

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
	delete m_emitter;
	printf("Killing engine \n");
}
