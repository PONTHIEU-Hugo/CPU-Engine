#ifndef CAMERA_CPP_INCLUDED
#define CAMERA_CPP_INCLUDED

#include "Camera.h"

#include <glm/ext/matrix_clip_space.hpp>

void Camera::UpdateViewMatrix()
{
    m_view = inverse(m_world);
    UpdateViewProj();
}

void Camera::HandleEvent(float _dt)
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
	vec3 up = vec3(rot[1]); 

    if (keys[SDL_SCANCODE_W]) m_camPos += forward * speed * _dt;
    if (keys[SDL_SCANCODE_S]) m_camPos -= forward * speed * _dt;
    if (keys[SDL_SCANCODE_D]) m_camPos += right   * speed * _dt;
    if (keys[SDL_SCANCODE_A]) m_camPos -= right   * speed * _dt;
	if (keys[SDL_SCANCODE_SPACE]) m_camPos += up * speed * _dt;
	if (keys[SDL_SCANCODE_LCTRL]) m_camPos -= up * speed * _dt;

    mat4 world = translate(mat4(1.0f), m_camPos) * rot;
    SetMatrix(world);   
}

void Camera::SetOrthographic(float32 w, float32 h, float32 nearZ, float32 farZ)
{
    m_type = ORTHOGRAPHIC;
    m_proj = ortho(-w * 0.5f, w * 0.5f, -h * 0.5f, h * 0.5f, nearZ, farZ);
    UpdateViewProj();
}

void Camera::SetPerspective(float32 fovY, float32 aspect, float32 nearZ, float32 farZ)
{
    m_type = PERSPECTIVE;
    m_proj = perspective(fovY, aspect, nearZ, farZ);
    UpdateViewProj();
}


#endif
