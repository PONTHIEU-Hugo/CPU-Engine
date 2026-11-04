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
struct WaveData;
struct CubeMap;

// out du vertex shader / in du pixel shader (position clipSpace).
struct Varyings
{
    glm::vec4 position	= { 0.0f, 0.0f, 0.0f, 1.0f };
    glm::vec3 normal	= { 0.0f, 0.0f, 0.0f };
    glm::vec4 color		= { 255.0f, 255.0f, 255.0f, 255.0f };
    glm::vec2 uv		= { 0.0f, 0.0f };
	glm::vec3 arm		= { 1.0f, 0.5f, 0.0f };
	glm::vec3 tangent	= glm::vec3(1, 0, 0);
	glm::vec2 duvdx		= glm::vec2(0.0f);
	glm::vec2 duvdy		= glm::vec2(0.0f);
	glm::vec3 dpdx		= glm::vec3(0.0f);
	glm::vec3 dpdy		= glm::vec3(0.0f);
    glm::vec3 worldPos	= { 0.0f, 0.0f, 0.0f }; 
    glm::vec3 lightPos	= { 0.0f, 0.0f, 0.0f };
};
 
struct Uniforms
{
    glm::mat4 vp{ 1.0f };
    const Texture* texture = nullptr;
    glm::vec4 baseColor = { 255.0f, 255.0f, 255.0f, 255.0f };
    glm::vec3 lightDirection = { 0.0f, -1.0f, 0.0f };
	glm::vec3 lightColor	 = { 255.0f, 255.0f, 255.0f};
    float ambient = 0.2f;

    // Reflet speculaire (Phong)
    glm::vec3 cameraPos = { 0.0f, 0.0f, 0.0f };
    float specularStrength = 0.0f;   // 0 = pas de reflet
    float shininess = 32.0f;         // exposant de Phong
	float thickness = 1.0f;
	float eta = 0.66f;
	const FinalFrameBuffer*   sceneColor = nullptr;
	const std::vector<float>* sceneDepth = nullptr;

	// NormalMap
	const Texture* normalMap	= nullptr;
	float normalStrength		= 1.0f;
	bool  normalFlipY			= false;
	
	// armMap
	const Texture* aoMap = nullptr;
	const Texture* roughnessMap	= nullptr;
	const Texture* metalnessMap = nullptr;
	const Texture* armMap = nullptr;

    const CubeMap* cubeMap = nullptr;
	
	// displacement
	const Texture* displacementMap = nullptr;
	float displacementScale = 0.0f;
	float displacementMidpoint = 0.5f;
	float displacementPixelEdge = 8.0f;
	float tessPixelEdge = 0.0f;
	
    // Ombres
    glm::mat4 lightVP{ 1.0f };
    const RenderTarget* shadowMap = nullptr;
	const RenderTarget* shadowTint = nullptr;
    float shadowBiasConst = 0.0f;
    float shadowBiasSlope = 0.0f;

    // Vagues
    double time = 0.0;
    glm::vec3 scale{ 1, 1, 1 }; 
    glm::vec3 divScale{ 1, 1, 1 }; // 1/scale to multpily instead of divide
    glm::vec3 translation{ 0, 0, 0 };
    uint8_t waveNumber = 0;
    WaveData* waveDatas = nullptr;
};

enum class CullMode { None, Back, Front };

using VertexShader = Varyings(*)(const Vertex&, const Uniforms&);
using PixelShader = glm::vec4(*)(const Varyings&, const Uniforms&);

// Un shader = vertex shader + pixel shader
struct Shader
{
    VertexShader vs;
    PixelShader ps;
};

class Pipeline
{
public:
	// Clusters 
	static constexpr int TILES_X = 16;
	static constexpr int TILES_Y = 9;
	static constexpr int TILE_COUNT = TILES_X * TILES_Y;

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

    // Etats du pipeline (persistants : a restaurer apres usage !)
    // Ecriture dans le depth buffer (le depth TEST reste toujours actif).
    // Mettre a false pour les transparents.
    void SetDepthWrite(bool enabled) { m_depthWrite = enabled; }
	
    void SetBlend(bool enabled) { m_blend = enabled; }

    // true : seuls la couleur et les uv sont interpoles par pixel (normale, tangente, worldPos et lightPos restent a zero).
    // Pour les shaders qui n'utilisent que ceux-la, comme ParticleShader.
    void SetLiteVaryings(bool enabled) { m_lite = enabled; }

    // Avec le blend actif : false = SrcAlpha / InvSrcAlpha, true = SrcAlpha / One.
    void SetAdditive(bool enabled) { m_additive = enabled; }

    // Alpha test (hors blend) : un pixel dont l'alpha (0-255) est < cutoff est rejete.
    // 0 = desactive.
    void SetAlphaCutoff(float cutoff) { m_alphaCutoff = cutoff; }

    bool GetDepthWrite() const { return m_depthWrite; }
    bool GetBlend() const { return m_blend; }
	const std::vector<float>& GetScreenDepth() const { return m_screen.depth; }

    void ClearDepth();

    void Draw(glm::mat4 const& model);

private:

	struct ScreenTriangle
	{
		Varyings  varyings[3];        
		glm::vec3 screenPos[3] = { glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(0.0f) };
        float invW[3] = { 0.0f, 0.0f, 0.0f };
		float invArea = 1;     
        int minX = 0;
        int maxX = 0;
        int minY = 0;
        int maxY = 0;
	};

	struct ThreadBins
	{
		std::vector<ScreenTriangle> triangles;
		std::vector<int>            bins[TILE_COUNT];  
	};

	bool SetupTriangle(const Varyings& a, const Varyings& b, const Varyings& c, ScreenTriangle& out) const;
	void BinTriangle(const ScreenTriangle& tri, int index, ThreadBins& local) const;
	void RasterizeInTile(const ScreenTriangle& tri, int tileX0, int tileY0, int tileX1, int tileY1);
	int  ComputeTessLevel(const std::vector<Vertex>& world) const;
	void TessellateTriangle(const Vertex& a, const Vertex& b, const Vertex& c, int N, std::vector<Vertex>& out) const;
	
	std::vector<Vertex> m_worldVertices, m_tessVertices;
    const Mesh* m_mesh = nullptr;

    VertexShader m_vertexShader;
    PixelShader m_pixelShader;

    Uniforms m_uniforms;

    RenderTarget  m_screen;
    RenderTarget* m_target = nullptr;

    CullMode      m_cull = CullMode::Back;

    bool          m_depthWrite  = true;
    bool          m_blend       = false;
    bool          m_additive    = false;
    bool          m_lite        = false;
    float         m_alphaCutoff = 50.0f;

    std::vector<Varyings> m_varyings;
	
	std::vector<ThreadBins> m_threadBins;   // reutilise d'un Draw a l'autre
};

#endif