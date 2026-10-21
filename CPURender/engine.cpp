#include "engine.h"
#include "Shaders.h"
#include <stdio.h>
#include <SDL3/SDL_events.h>
#include <glm/gtc/matrix_transform.hpp>
#include "Chrono.h"
#include "GeometryFactory.hpp"

void Engine::init()
{
	printf("Init engine \n");

	m_triangle = 
	{
		{ {-5.0f, -1.0f, 0.0f}, {255, 0, 0} },
		{ { -3.0f, -1.0f, 0.0f}, {0, 255, 0}},
		{ { -4.0f,  1.0f, 0.0f}, {0, 0, 255} },
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

	
	// m_obj = GeometryFactory::BuildTriangle();
	// m_obj = GeometryFactory::BuildPyramid();
	// m_obj = GeometryFactory::BuildCube();
	// m_obj = GeometryFactory::BuildIcosphere(3);         
	// m_obj = GeometryFactory::BuildIcosphere(4, 1.0f);
	// m_obj = GeometryFactory::BuildUVSphere(32, 16);        
	// m_obj = GeometryFactory::BuildUVSphere(64, 32, 2.0f);
	// m_obj = GeometryFactory::BuildCircle(32, 0.0f, true);
	// m_obj = GeometryFactory::BuildCircle(32, 0.0f, false);
	// m_obj = GeometryFactory::BuildCylinder(32);
	// m_obj = GeometryFactory::BuildDonut(0.5f, 0.2f, 48, 24);
	
	m_obj = GeometryFactory::LoadGeometry("assets/tuture.obj");
	
	
	m_texture.Load("Assets/mael.png");

	m_triangleBounds = ComputeAABB(m_triangle);
	m_cubeBounds = ComputeAABB(m_cube);

	m_pipeline.BindShader(BasicShader);

	float aspect = float(WIDTH) / float(HEIGHT);
	
	// m_camera.SetOrthographic(10.0f * aspect, 10.0f, 0.1f, 100.0f);
	m_camera.SetPerspective(radians(45.0f), aspect, 0.1f, 100.0f);
	m_camera.SetMatrix(translate(mat4(1.0f), vec3(100.0f, 100.0f, 150.0f)));
	
	m_emitter = new Emitter({0.0f, 0.0f, 0.0f}, 1000);
	m_emitter->Init();
}

void Engine::Update()
{
	float dt = static_cast<float>(CHRONO->GetDeltaTime());
	m_camera.HandleEvent(dt);
	if (m_emitter != nullptr)
		m_emitter->Update();
	
	// m_duration_flag -= dt;
	// if ( m_duration_flag <= 0.0f )
	// {
	// 	if ( m_currentFlag == m_flags.size() - 1)
	// 		m_currentFlag = 0;
	// 	else
	// 		m_currentFlag++;
	// 	m_obj = m_flags[m_currentFlag];
	// 	m_duration_flag = m_time_flag;
	// }
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
	uniforms.mvp = m_camera.GetVPMatrix();
	
	Frustum frustum = Frustum::FromMatrix(m_camera.GetVPMatrix());


	if (frustum.Intersects(m_triangleBounds))
	{
		m_pipeline.BindShader(BasicShader);
		m_pipeline.BindVertexBuffer(&m_triangle);
		m_pipeline.BindUniforms(uniforms);
		m_pipeline.Draw();
	}

	/*// Cube
	if (frustum.Intersects(m_cubeBounds))
	{
		uniforms.texture = &m_texture;
		m_pipeline.BindShader(TextureShader);
		m_pipeline.BindVertexBuffer(&m_cube);
		m_pipeline.BindUniforms(uniforms);
		m_pipeline.Draw();
		uniforms.texture = nullptr;
	}*/

	m_pipeline.BindShader(BasicShader);

	m_particleVertices.clear();
	if (m_emitter)
		m_emitter->Render(m_particleVertices);

	uniforms.mvp = m_camera.GetVPMatrix();
	m_pipeline.BindVertexBuffer(&m_particleVertices);
	m_pipeline.BindUniforms(uniforms);
	m_pipeline.Draw();
	
	uniforms.texture = &m_texture;
	m_pipeline.BindShader(TextureShader);
	m_pipeline.BindVertexBuffer(&m_obj);
	m_pipeline.BindUniforms(uniforms);
	m_pipeline.Draw();
}

void Engine::destroy()
{
	delete m_emitter;
	printf("Killing engine \n");
}
