#include "Shaders.h"
#include "Texture.h"
#include <cmath>
#include <SDL3/SDL_init.h>
#include "CubeMap.h"

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
	
	// color = glm::vec4((glm::vec3(in.normal.x, in.normal.y, in.normal.z) * 0.5f + 0.5f) * 255.0f, 128.0f);
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

static glm::vec3 SurfaceARM(const Varyings& in, const Uniforms& u)
{
	if (!u.armMap || !u.armMap->IsValid())
		return in.arm;
	float lod = u.armMap->ComputeLod(in.duvdx, in.duvdy);
	glm::vec4 s = u.armMap->Sample(in.uv, lod);
	return glm::vec3(s) / 255.0f;
}

static float SurfaceAO(const Varyings& in, const Uniforms& u)
{
	if (u.aoMap && u.aoMap->IsValid())
	{
		float lod = u.aoMap->ComputeLod(in.duvdx, in.duvdy);
		return u.aoMap->Sample(in.uv, lod).r / 255.0f;
	}
	return SurfaceARM(in, u).x;
}

static float SurfaceRoughness(const Varyings& in, const Uniforms& u)
{
	if (u.roughnessMap && u.roughnessMap->IsValid())
	{
		float lod = u.roughnessMap->ComputeLod(in.duvdx, in.duvdy);
		return u.roughnessMap->Sample(in.uv, lod).g / 255.0f;
	}
	return SurfaceARM(in, u).y;
}

static float SurfaceMetalness(const Varyings& in, const Uniforms& u)
{
	if (u.metalnessMap && u.metalnessMap->IsValid())
	{
		float lod = u.metalnessMap->ComputeLod(in.duvdx, in.duvdy);
		return u.metalnessMap->Sample(in.uv, lod).b / 255.0f;
	}
	return SurfaceARM(in, u).z;
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
	
	float r;
	if (u.armMap)
		r = glm::clamp(SurfaceARM(in, u).g, 0.05f, 1.0f);
	else
		r = glm::clamp(SurfaceRoughness(in, u), 0.05f, 1.0f);
	
	float calculatedShininess = glm::max(2.0f / (r * r) - 2.0f, 1.0f);
	
	return u.specularStrength * std::pow(glm::max(glm::dot(reflected, toCamera), 0.0f), calculatedShininess);
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
	return SurfaceColor(in, u);
}

const Shader BasicShader = { BasicVS, BasicPS };

static glm::vec4 TexturePS(const Varyings& in, const Uniforms& u)
{
	return SurfaceColor(in, u);
}

const Shader TextureShader = { BasicVS, TexturePS };

static glm::vec4 ParticlePS(const Varyings& in, const Uniforms& u)
{
	glm::vec2 fromCenter = in.uv - glm::vec2(0.5f, 0.5f);
	float transparency = 4.0f * glm::dot(fromCenter, fromCenter);
	glm::vec4 color = in.color * (u.baseColor / 255.0f);
	color.a -= 255.0f * transparency;
	return color;
}

const Shader ParticleShader = { BasicVS, ParticlePS };

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
	
	specular = std::clamp(specular, 0.0f, 255.0f);
	
	float ao    = SurfaceAO(in, u);
	float metal = SurfaceMetalness(in, u);

	glm::vec3 base = glm::vec3(baseColor);
	glm::vec3 F0 = glm::mix(glm::vec3(0.04f), base / 255.0f, metal);
	glm::vec3 diffuseColor = base * (1.0f - metal);

	glm::vec3 lighting = glm::vec3(u.ambient * ao) + (1.0f - u.ambient) * diffuse;
	glm::vec3 rgb = diffuseColor * lighting + 255.0f * F0 * specular;

	return glm::vec4(rgb, surface.a);
}

const Shader LambertShader = { BasicVS, LambertPS };

static Varyings SkyboxVS(const Vertex& v, const Uniforms& u)
{
	Varyings o;
	o.worldPos = v.position;                       
	o.position = u.vp * glm::vec4(v.position, 1.0f);
	o.position.z = o.position.w * 0.99999f;        
	return o;
}

static glm::vec4 SkyboxPS(const Varyings& in, const Uniforms& u)
{
	if (!u.cubeMap || !u.cubeMap->enabled)
		return glm::vec4(0.0f, 0.0f, 0.0f, 255.0f);

	const glm::vec3 dir = in.worldPos - u.cameraPos;

	return glm::vec4(u.cubeMap->Sample(dir), 255.0f);
}

const Shader SkyboxShader = { SkyboxVS, SkyboxPS };

// ---------------------------------------------------------------------------
// Ombres 
// ---------------------------------------------------------------------------

static glm::vec4 TintPS(const Varyings&, const Uniforms& u)
{
	const float density = 0.8f;                      
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

	assert(tint->depth.size() == map->depth.size());
	
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

	float cosT = glm::clamp(glm::dot(-v, n), 0.0f, 1.0f);
	float F0 = 0.04f;
	float fresnel = F0 + (1.0f - F0) * std::pow(1.0f - cosT, 5.0f);

	glm::vec3 reflColor(200.0f, 210.0f, 230.0f);
	glm::vec3 refl = glm::reflect(v, n);

	glm::vec3 pointFond = in.worldPos + refl * u.thickness;
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
			float terminator = glm::smoothstep(0.0f, 0.1f, geoCos);
			visibility = ShadowLight(in, u, geoCos) * terminator;
			specular = PhongSpecular(in, u, normal) * terminator;
		}
	}

	float ao    = SurfaceAO(in, u);
	float metal = SurfaceMetalness(in, u);

	glm::vec3 base = glm::vec3(baseColor);
	glm::vec3 F0 = glm::mix(glm::vec3(0.04f), base / 255.0f, metal);
	glm::vec3 diffuseColor = base * (1.0f - metal);

	glm::vec3 lighting = glm::vec3(u.ambient * ao) + (1.0f - u.ambient) * diffuse * visibility;
	glm::vec3 rgb = diffuseColor * lighting + 255.0f * F0 * specular * visibility;

	return glm::vec4(rgb, baseColor.a);
}

const Shader ShadowLambertShader = { ShadowVS, ShadowLambertPS };
