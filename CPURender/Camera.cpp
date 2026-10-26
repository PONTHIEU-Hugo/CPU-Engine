#ifndef CAMERA_CPP_INCLUDED
#define CAMERA_CPP_INCLUDED

#include "Camera.h"

#include <glm/ext/matrix_clip_space.hpp>

void Camera::UpdateViewMatrix()
{
    m_view = inverse(m_transform.GetWorldMatrix());
    UpdateViewProj();
}

Camera::Camera(vec3 const& pos, Transform* parent)
{
    m_transform = Transform(pos, { 1.0f, 0.0f, 0.0f, 0.0f}, vec3(1.0f, 1.0f, 1.0f));
    if ( parent )
        m_transform.SetParent(parent);
}

void Camera::HandleEvent(float _dt)
{
    const bool* keys = SDL_GetKeyboardState(nullptr);

    const float speed = 3.0f;
    const float turn  = 1.5f;

    if (keys[SDL_SCANCODE_LEFT])  
        m_transform.AddYPR({turn * _dt, 0.0f, 0.0f});
    if (keys[SDL_SCANCODE_RIGHT]) 
        m_transform.AddYPR({-turn * _dt, 0.0f, 0.0f});
    if (keys[SDL_SCANCODE_UP])    
        m_transform.AddYPR({0.0f, turn * _dt, 0.0f});
    if (keys[SDL_SCANCODE_DOWN])  
        m_transform.AddYPR({0.0f, -turn * _dt, 0.0f});
    

    if (keys[SDL_SCANCODE_W]) 
        m_transform.Move(m_transform.GetForward() * speed * _dt);
    if (keys[SDL_SCANCODE_S])
        m_transform.Move(m_transform.GetForward() * -speed * _dt);
    if (keys[SDL_SCANCODE_D]) 
        m_transform.Move(m_transform.GetRight() * speed * _dt);
    if (keys[SDL_SCANCODE_A]) 
        m_transform.Move(m_transform.GetRight() * -speed * _dt);
	if (keys[SDL_SCANCODE_SPACE]) 
	    m_transform.Move(m_transform.GetUp() * speed * _dt);
	if (keys[SDL_SCANCODE_LCTRL]) 
	    m_transform.Move(m_transform.GetUp() * -speed * _dt);
    
    if (keys[SDL_SCANCODE_1])
        m_transform.SetPosition({5.0f, 1.0f, 3.0f});
    if (keys[SDL_SCANCODE_2])
        m_transform.SetPosition({0.0f, 4.5f, 3.0f});
    if (keys[SDL_SCANCODE_3])
        m_transform.SetPosition({0.0f, 6.5f, 18.0f});
    
    UpdateViewMatrix();
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
