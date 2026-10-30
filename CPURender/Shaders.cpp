#include "Shaders.h"
#include "Texture.h"

#include <cmath>

// ---------------------------------------------------------------------------
// Basic / Texture / Lambert
// ---------------------------------------------------------------------------

static glm::vec4 SurfaceColor(const Varyings& in, const Uniforms& u)
{
	glm::vec4 color = u.texture ? u.texture->Sample(in.uv) : in.color;
	return color * (u.baseColor / 255.0f);
}

static float PhongSpecular(const Varyings& in, const Uniforms& u, const glm::vec3& normal)
{
	if (u.specularStrength <= 0.0f)
		return 0.0f;

	glm::vec3 toCamera = u.cameraPos - in.worldPos;
	float toCameraLength2 = glm::dot(toCamera, toCamera);
	if (toCameraLength2 < 1e-12f)
		return 0.0f;

	toCamera /= std::sqrt(toCameraLength2);

	glm::vec3 reflected = glm::reflect(glm::normalize(u.lightDirection), normal);
	return u.specularStrength * std::pow(glm::max(glm::dot(reflected, toCamera), 0.0f), u.shininess);
}

static Varyings BasicVS(const Vertex& v, const Uniforms& u)
{
	Varyings out;
	// v.position et v.normal sont deja en espace monde (Pipeline::Draw applique la matrice modele).
	out.position = u.vp * glm::vec4(v.position, 1.0f);
	out.worldPos = v.position;
	out.normal = v.normal;
	out.color = v.color;
	out.uv = v.uv;

	return out;
}

static glm::vec4 BasicPS(const Varyings& in, const Uniforms& u)
{
	return in.color * (u.baseColor / 255.0f);
}

const Shader BasicShader = { BasicVS, BasicPS };

static glm::vec4 TexturePS(const Varyings& in, const Uniforms& u)
{
	return SurfaceColor(in, u);
}

const Shader TextureShader = { BasicVS, TexturePS };

static glm::vec4 ParticlePS(const Varyings& in, const Uniforms& u)
{
	float transparency = glm::length(in.uv - glm::vec2(0.5f, 0.5f));
	glm::vec4 color = in.color * (u.baseColor / 255.0f);
	color.a -= 255.0f * transparency;
	return color;
}

const Shader ParticleShader = { BasicVS, ParticlePS };

static glm::vec4 LambertPS(const Varyings& in, const Uniforms& u)
{
	glm::vec4 surface   = SurfaceColor(in, u);
	glm::vec3 baseColor = SurfaceColor(in, u);

	float normalLength2 = glm::dot(in.normal, in.normal);
	float diffuse = 1.0f;
	float specular = 0.0f;

	if (normalLength2 > 1e-8f)
	{
		glm::vec3 normal = in.normal / std::sqrt(normalLength2);
		diffuse = glm::max(glm::dot(normal, -glm::normalize(u.lightDirection)), 0.0f);
		if (diffuse > 0.0f)
			specular = PhongSpecular(in, u, normal);
	}

	float lighting = u.ambient + (1.0f - u.ambient) * diffuse;
	glm::vec3 rgb = baseColor * lighting + glm::vec3(255.0f) * specular;
	return glm::vec4(rgb, surface.a);
}

const Shader LambertShader = { BasicVS, LambertPS };

// ---------------------------------------------------------------------------
// Ombres 
// ---------------------------------------------------------------------------

static Varyings DepthVS(const Vertex& v, const Uniforms& u)
{
	Varyings out;
	out.position = u.vp * glm::vec4(v.position, 1.0f);
	return out;
}

const Shader DepthShader = { DepthVS, nullptr };

static float ShadowVisibility(const Varyings& in, const Uniforms& u, float cosTheta)
{
	const RenderTarget* map = u.shadowMap;
	if (!map) return 1.0f;

	if (in.lightPos.z >= 1.0f || in.lightPos.z <= -1.0f)
		return 1.0f;

	float sx = (in.lightPos.x * 0.5f + 0.5f) * (float)map->width;
	float sy = (1.0f - (in.lightPos.y * 0.5f + 0.5f)) * (float)map->height;

	int x0 = (int)std::floor(sx);
	int y0 = (int)std::floor(sy);
	if (x0 < 0 || x0 >= map->width || y0 < 0 || y0 >= map->height)
		return 1.0f;

	float c = glm::max(cosTheta, 0.05f);
	float tanTheta = glm::min(std::sqrt(1.0f - c * c) / c, 10.0f);
	float bias = u.shadowBiasConst + u.shadowBiasSlope * tanTheta;

	// PCF 3x3
	float z = in.lightPos.z - bias;
	int lit = 0;
	for (int dy = -1; dy <= 1; ++dy)
		for (int dx = -1; dx <= 1; ++dx)
		{
			int x = glm::clamp(x0 + dx, 0, map->width  - 1);
			int y = glm::clamp(y0 + dy, 0, map->height - 1);
			if (z <= map->depth[(size_t)y * map->width + x])
				++lit;
		}
	return (float)lit / 9.0f;
}

static Varyings ShadowVS(const Vertex& v, const Uniforms& u)
{
	Varyings out;
	out.position = u.vp * glm::vec4(v.position, 1.0f);
	out.worldPos = v.position;
	out.normal = v.normal;
	out.color  = v.color;
	out.uv     = v.uv;
	out.lightPos = glm::vec3(u.lightVP * glm::vec4(v.position, 1.0f));
	return out;
}

static glm::vec4 ShadowLambertPS(const Varyings& in, const Uniforms& u)
{
	glm::vec4 baseColor = SurfaceColor(in, u);

	float normalLength2 = glm::dot(in.normal, in.normal);
	float diffuse = 1.0f;
	float visibility = 1.0f;
	float specular = 0.0f;

	if (normalLength2 > 1e-8f)
	{
		glm::vec3 normal = in.normal / std::sqrt(normalLength2);
		diffuse = glm::max(glm::dot(normal, -glm::normalize(u.lightDirection)), 0.0f);
		if (diffuse > 0.0f)
		{
			visibility = ShadowVisibility(in, u, diffuse);
			specular = PhongSpecular(in, u, normal);
		}
	}

	float lighting = u.ambient + (1.0f - u.ambient) * diffuse * visibility;
	glm::vec3 rgb = glm::vec3(baseColor.x, baseColor.y, baseColor.z)  * lighting + 255 * specular * visibility;
	return glm::vec4(rgb, baseColor.a);
}

const Shader ShadowLambertShader = { ShadowVS, ShadowLambertPS };
