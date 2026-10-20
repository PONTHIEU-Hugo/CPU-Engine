#pragma once

#include <functional>
#include <vector>
#include <glm/glm.hpp>
#include "main.h"

struct Texture;

struct Vertex
{
	glm::vec3 position = { 0.0f, 0.0f, 0.0f };
	glm::vec3 color = { 255.0f, 0.0f, 255.0f };
	glm::vec2 uv = { 0.0f, 0.0f };
};

// out du vertex shader / in du pixel shader (position clipSpace).
struct Varyings
{
	glm::vec4 position = { 0.0f, 0.0f, 0.0f, 1.0f };
	glm::vec3 color = { 255.0f, 0.0f, 255.0f };
	glm::vec2 uv = { 0.0f, 0.0f };
};

struct Uniforms
{
	glm::mat4 mvp{ 1.0f };
	const Texture* texture = nullptr;
};

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

	void BindVertexBuffer(const std::vector<Vertex>* vertices) { m_vertices = vertices; }
	void BindVertexShader(VertexShader vs) { m_vertexShader = std::move(vs); }
	void BindPixelShader(PixelShader ps) { m_pixelShader = std::move(ps); }
	void BindShader(const Shader& shader) { m_vertexShader = shader.vs; m_pixelShader = shader.ps; }
	void BindUniforms(const Uniforms& uniforms) { m_uniforms = uniforms; }
	void BindFrameBuffer(FinalFrameBuffer* fb) { m_frameBuffer = fb; }

	void ClearDepth();

	void Draw();

private:
	void DrawTriangle(const Varyings& vertexA, const Varyings& vertexB, const Varyings& vertexC);

	const std::vector<Vertex>* m_vertices = nullptr;

	VertexShader m_vertexShader;
	PixelShader m_pixelShader;

	Uniforms m_uniforms;

	FinalFrameBuffer* m_frameBuffer = nullptr;

	std::vector<float> m_depth; // z-buffer, 1 = loin
};
