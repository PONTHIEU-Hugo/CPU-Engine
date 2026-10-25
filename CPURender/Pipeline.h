#pragma once

#include <functional>
#include <vector>
#include <glm/glm.hpp>
#include "main.h"
#include "RenderTarget.h"
#include "Transform.h"

struct Texture;

struct Vertex
{
	glm::vec3 position = { 0.0f, 0.0f, 0.0f };
	glm::vec3 normal = { 0.0f, 0.0f, 0.0f };
	glm::vec3 color = { 255.0f, 0.0f, 255.0f };
	glm::vec2 uv = { 0.0f, 0.0f };
};

struct Geometry
{
	std::vector<Vertex> vertices;
	Transform transform;
};


// out du vertex shader / in du pixel shader (position clipSpace).
struct Varyings
{
	glm::vec4 position = { 0.0f, 0.0f, 0.0f, 1.0f };
	glm::vec3 normal = { 0.0f, 0.0f, 0.0f };
	glm::vec3 color = { 255.0f, 0.0f, 255.0f };
	glm::vec2 uv = { 0.0f, 0.0f };
	glm::vec3 lightPos = { 0.0f, 0.0f, 0.0f };
};

struct Uniforms
{
	glm::mat4 mvp{ 1.0f };
	const Texture* texture = nullptr;
	glm::vec3 lightDirection = { 0.0f, -1.0f, 0.0f }; 
	float ambient = 0.2f; 
	
	// Ombres
	glm::mat4 lightVP{ 1.0f };
	const RenderTarget* shadowMap = nullptr;  
	float shadowBiasConst = 0.0f;        
	float shadowBiasSlope = 0.0f;
};

enum class CullMode { None, Back, Front };

using VertexShader = std::function<Varyings(const Vertex&, const Uniforms&)>;
using PixelShader = std::function<glm::vec3(const Varyings&, const Uniforms&)>;

// Un shader = vertex shader + pixel shader
struct Shader
{
	VertexShader vs;
	PixelShader ps;
};

class Pipeline 
{
public:
	Pipeline();
	Pipeline(const Pipeline&) = delete;
	Pipeline& operator=(const Pipeline&) = delete;
	
	void BindVertexBuffer(const Geometry* geometries) { m_geometry = geometries; }
	void BindVertexShader(VertexShader vs) { m_vertexShader = std::move(vs); }
	void BindPixelShader(PixelShader ps) { m_pixelShader = std::move(ps); }
	void BindShader(const Shader& shader) { m_vertexShader = shader.vs; m_pixelShader = shader.ps; }
	void BindUniforms(const Uniforms& uniforms) { m_uniforms = uniforms; }
	void BindFrameBuffer(FinalFrameBuffer* fb) { m_screen.color = fb; m_target = &m_screen; }
	void BindTarget(RenderTarget* target)      { m_target = target; }
	void SetCullMode(CullMode mode)            { m_cull = mode; }

	void ClearDepth();

	void Draw(glm::mat4 const& mat);

private:
	void DrawTriangle(const Varyings& vertexA, const Varyings& vertexB, const Varyings& vertexC);

	const Geometry* m_geometry = nullptr;

	VertexShader m_vertexShader;
	PixelShader m_pixelShader;

	Uniforms m_uniforms;
	
	RenderTarget  m_screen;               
	RenderTarget* m_target = nullptr;
	
	CullMode      m_cull   = CullMode::Back;
};
