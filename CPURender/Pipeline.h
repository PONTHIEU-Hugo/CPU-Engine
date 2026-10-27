#ifndef PIPELINE_H_INCLUDED
#define PIPELINE_H_INCLUDED

#include <functional>
#include <vector>
#include <glm/glm.hpp>
#include "main.h"
#include "Vertex.h"
#include "RenderTarget.h"

struct Texture;
struct Mesh;

// out du vertex shader / in du pixel shader (position clipSpace).
struct Varyings
{
	glm::vec4 position = { 0.0f, 0.0f, 0.0f, 1.0f };
	glm::vec3 normal = { 0.0f, 0.0f, 0.0f };
	glm::vec3 color = { 255.0f, 0.0f, 255.0f };
	glm::vec2 uv = { 0.0f, 0.0f };
	glm::vec3 worldPos = { 0.0f, 0.0f, 0.0f };   // position du point en espace monde (specular)
	glm::vec3 lightPos = { 0.0f, 0.0f, 0.0f };
};

struct Uniforms
{
	glm::mat4 vp{ 1.0f };
	const Texture* texture = nullptr;
	glm::vec3 baseColor = { 255.0f, 255.0f, 255.0f };   // teinte du materiau (blanc = aucune)
	glm::vec3 lightDirection = { 0.0f, -1.0f, 0.0f };
	float ambient = 0.2f;

	// Reflet speculaire (Phong)
	glm::vec3 cameraPos = { 0.0f, 0.0f, 0.0f };
	float specularStrength = 0.0f;   // 0 = pas de reflet
	float shininess = 32.0f;         // exposant de Phong, derive de la roughness du materiau

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

	void BindVertexBuffer(const Mesh* mesh) { m_mesh = mesh; }
	void BindVertexShader(VertexShader vs) { m_vertexShader = std::move(vs); }
	void BindPixelShader(PixelShader ps) { m_pixelShader = std::move(ps); }
	void BindShader(const Shader& shader) { m_vertexShader = shader.vs; m_pixelShader = shader.ps; }
	void BindUniforms(const Uniforms& uniforms) { m_uniforms = uniforms; }
	void BindFrameBuffer(FinalFrameBuffer* fb) { m_screen.color = fb; m_target = &m_screen; }
	void BindTarget(RenderTarget* target) { m_target = target; }
	void SetCullMode(CullMode mode) { m_cull = mode; }

	void ClearDepth();

	void Draw(glm::mat4 const& model);

private:
	void DrawTriangle(const Varyings& vertexA, const Varyings& vertexB, const Varyings& vertexC);

	const Mesh* m_mesh = nullptr;

	VertexShader m_vertexShader;
	PixelShader m_pixelShader;

	Uniforms m_uniforms;

	RenderTarget  m_screen;
	RenderTarget* m_target = nullptr;

	CullMode      m_cull = CullMode::Back;

	std::vector<Varyings> m_varyings;   // reutilise d'un Draw a l'autre (pas d'allocation par draw)
};

#endif // !PIPELINE_H_INCLUDED