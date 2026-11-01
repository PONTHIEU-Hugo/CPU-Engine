#include "Shaders.h"
#include "Texture.h"
#include <cmath>

#include "core.h"

// ---------------------------------------------------------------------------
// Basic / Texture / Lambert
// ---------------------------------------------------------------------------

static glm::vec4 SurfaceColor(const Varyings& in, const Uniforms& u)
{
	glm::vec4 color = in.color;
	if (u.texture)
	{
		float lod = u.texture->ComputeLod(in.duvdx, in.duvdy);
		color = u.texture->Sample(in.uv, lod);
	}
	return color * (u.baseColor / 255.0f);
}

static glm::vec3 SurfaceNormal(const Varyings& in, const Uniforms& u)
{
	float len2 = glm::dot(in.normal, in.normal);
	if (len2 <= 1e-8f)
		return glm::vec3(0.0f);

	glm::vec3 N = in.normal / std::sqrt(len2);
	if (!u.normalMap || !u.normalMap->IsValid())
		return N;

	glm::vec3 dp2perp = glm::cross(N, in.dpdy);
	glm::vec3 dp1perp = glm::cross(in.dpdx, N);
	glm::vec3 T = dp2perp * in.duvdx.x + dp1perp * in.duvdy.x;
	glm::vec3 B = dp2perp * in.duvdx.y + dp1perp * in.duvdy.y;

	float m = std::max(glm::dot(T, T), glm::dot(B, B));
	if (m < 1e-20f)
		return N;                      
	float invMax = 1.0f / std::sqrt(m);
	T *= invMax;
	B *= invMax;

	float lod = u.normalMap->ComputeLod(in.duvdx, in.duvdy);
	glm::vec4 s = u.normalMap->Sample(in.uv, lod);
	glm::vec3 t = glm::vec3(s) / 255.0f * 2.0f - 1.0f;

	if (u.normalFlipY)
		t.y = -t.y;
	t.x *= u.normalStrength;
	t.y *= u.normalStrength;

	return glm::normalize(T * t.x + B * t.y + N * t.z);
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
	out.tangent = v.tangent;

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

static glm::vec4 LambertPS(const Varyings& in, const Uniforms& u)
{
	glm::vec4 surface   = SurfaceColor(in, u);
	glm::vec3 baseColor = surface;

	glm::vec3 normal = SurfaceNormal(in, u);
	float diffuse = 1.0f;
	float specular = 0.0f;

	if (glm::dot(normal, normal) > 0.5f) 
	{
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

static glm::vec4 TintPS(const Varyings&, const Uniforms& u)
{
	const float density = 0.8f;                       // 0 = ombre claire, 1 = tres sombre
	float     a     = glm::clamp(u.baseColor.a / 255.0f, 0.0f, 1.0f);
	glm::vec3 color = glm::vec3(u.baseColor) / 255.0f;

	glm::vec3 t = glm::mix(glm::vec3(1.0f), color, a) * (1.0f - a * density);
	return glm::vec4(t * 255.0f, 255.0f);
}

static Varyings DepthVS(const Vertex& v, const Uniforms& u)
{
	Varyings out;
	out.position = u.vp * glm::vec4(v.position, 1.0f);
	return out;
}

const Shader TintShader = { DepthVS, TintPS };

const Shader DepthShader = { DepthVS, nullptr };

static glm::vec3 ShadowLight(const Varyings& in, const Uniforms& u, float cosTheta)
{
	const RenderTarget* map = u.shadowMap;
	if (!map) return glm::vec3(1.0f);

	if (in.lightPos.z >= 1.0f || in.lightPos.z <= -1.0f)
		return glm::vec3(1.0f);

	float sx = (in.lightPos.x * 0.5f + 0.5f) * (float)map->width;
	float sy = (1.0f - (in.lightPos.y * 0.5f + 0.5f)) * (float)map->height;
	int x0 = (int)std::floor(sx);
	int y0 = (int)std::floor(sy);
	if (x0 < 0 || x0 >= map->width || y0 < 0 || y0 >= map->height)
		return glm::vec3(1.0f);

	float c = glm::max(cosTheta, 0.05f);
	float tanTheta = glm::min(std::sqrt(1.0f - c * c) / c, 10.0f);
	float bias = u.shadowBiasConst + u.shadowBiasSlope * tanTheta;
	float z = in.lightPos.z - bias;

	const RenderTarget* tint = u.shadowTint;

	glm::vec3 sum(0.0f);
	for (int dy = -1; dy <= 1; ++dy)
		for (int dx = -1; dx <= 1; ++dx)
		{
			int x = glm::clamp(x0 + dx, 0, map->width  - 1);
			int y = glm::clamp(y0 + dy, 0, map->height - 1);
			size_t i = (size_t)y * map->width + x;

			if (z > map->depth[i])
				continue;                                 

			glm::vec3 t(1.0f);
			if (tint && !tint->tint.empty() && z > tint->depth[i])
			{
				uint32_t p = tint->tint[i];
				t = glm::vec3((p >> 16) & 255, (p >> 8) & 255, p & 255) / 255.0f;
			}

			sum += t;
		}
	return sum / 9.0f;
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

static glm::vec4 RefractionPS(const Varyings& in, const Uniforms& u)
{
	if (!u.sceneColor || !u.sceneDepth)
		return glm::vec4(glm::vec3(u.baseColor), 255.0f);

	glm::vec3 v = glm::normalize(in.worldPos - u.cameraPos);
	glm::vec3 n = glm::normalize(in.normal);

	float px = in.position.x;
	float py = in.position.y;

	glm::vec3 r = glm::refract(v, n, u.eta);
	if (glm::dot(r, r) < 1e-8f)   
		r = glm::reflect(v, n);

	glm::vec3 pointFond = in.worldPos + r * u.thickness;
	glm::vec4 clip = u.vp * glm::vec4(pointFond, 1.0f);

	float sx = px;
	float sy = py;
	if (clip.w > 1e-6f)
	{
		sx = (clip.x / clip.w * 0.5f + 0.5f) * WIDTH;
		sy = (1.0f - (clip.y / clip.w * 0.5f + 0.5f)) * HEIGHT;
	}

	sx = glm::clamp(sx, 0.0f, WIDTH  - 1.0f);
	sy = glm::clamp(sy, 0.0f, HEIGHT - 1.0f);

	int ix = (int)sx;
	int iy = (int)sy;

	if ((*u.sceneDepth)[(size_t)iy * WIDTH + ix] < in.position.z)
	{
		ix = glm::clamp((int)px, 0, WIDTH  - 1);
		iy = glm::clamp((int)py, 0, HEIGHT - 1);
	}

	uint32_t p = u.sceneColor->colorData[(size_t)iy * WIDTH + ix];
	glm::vec3 fond((p >> 16) & 255, (p >> 8) & 255, p & 255);

	glm::vec3 rgb = fond * (glm::vec3(u.baseColor) / 255.0f);
	return glm::vec4(rgb, 255.0f);
}

const Shader RefractionShader = { ShadowVS, RefractionPS };

static glm::vec4 ShadowLambertPS(const Varyings& in, const Uniforms& u)
{
	if (in.color.g < 10.0f)
		int i = 0;
	glm::vec4 baseColor = SurfaceColor(in, u);

	float diffuse = 1.0f;
	glm::vec3 visibility(1.0f);
	float specular = 0.0f;

	glm::vec3 normal = SurfaceNormal(in, u);
	if (glm::dot(normal, normal) > 0.5f)
	{
		glm::vec3 L = -glm::normalize(u.lightDirection);
		diffuse = glm::max(glm::dot(normal, L), 0.0f);
		if (diffuse > 0.0f)
		{
			float geoCos = glm::max(glm::dot(glm::normalize(in.normal), L), 0.0f);
			visibility = ShadowLight(in, u, geoCos);
			specular = PhongSpecular(in, u, normal);
		}
	}

	glm::vec3 lighting = glm::vec3(u.ambient) + (1.0f - u.ambient) * diffuse * visibility;
	glm::vec3 rgb = glm::vec3(baseColor) * lighting + 255.0f * specular * visibility;
	return glm::vec4(rgb, baseColor.a);
}

const Shader ShadowLambertShader = { ShadowVS, ShadowLambertPS };
