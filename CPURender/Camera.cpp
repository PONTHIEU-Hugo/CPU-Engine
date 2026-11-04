#include "Camera.h"

#include <SDL3/SDL_keyboard.h>
#include <glm/ext/matrix_clip_space.hpp>

void Camera::UpdateViewMatrix()
{
    m_view = inverse(transform.GetWorldMatrix());
    UpdateViewProj();
}

Camera::Camera(glm::vec3 const& pos, Transform* parent)
{
    transform = Transform(pos, { 1.0f, 0.0f, 0.0f, 0.0f}, glm::vec3(1.0f, 1.0f, 1.0f));
    if ( parent )
        transform.SetParent(parent);
}

void Camera::HandleEvent(float _dt)
{
    const bool* keys = SDL_GetKeyboardState(nullptr);

    const float speed = 10.0f;
    const float turn  = 1.5f;

    if (keys[SDL_SCANCODE_LEFT])  
        transform.AddYPR({turn * _dt, 0.0f, 0.0f});
    if (keys[SDL_SCANCODE_RIGHT]) 
        transform.AddYPR({-turn * _dt, 0.0f, 0.0f});
    if (keys[SDL_SCANCODE_UP])    
        transform.AddYPR({0.0f, turn * _dt, 0.0f});
    if (keys[SDL_SCANCODE_DOWN])  
        transform.AddYPR({0.0f, -turn * _dt, 0.0f});
    

    if (keys[SDL_SCANCODE_W]) 
        transform.Move(transform.GetForward() * speed * _dt);
    if (keys[SDL_SCANCODE_S])
        transform.Move(transform.GetForward() * -speed * _dt);
    if (keys[SDL_SCANCODE_D]) 
        transform.Move(transform.GetRight() * speed * _dt);
    if (keys[SDL_SCANCODE_A]) 
        transform.Move(transform.GetRight() * -speed * _dt);
	if (keys[SDL_SCANCODE_SPACE]) 
	    transform.Move(transform.GetUp() * speed * _dt);
	if (keys[SDL_SCANCODE_LCTRL]) 
	    transform.Move(transform.GetUp() * -speed * _dt);
    
    UpdateViewMatrix();
}

void Camera::SetOrthographic(glm::float32 w, glm::float32 h, glm::float32 nearZ, glm::float32 farZ)
{
    m_type = ORTHOGRAPHIC;
    m_proj = glm::ortho(-w * 0.5f, w * 0.5f, -h * 0.5f, h * 0.5f, nearZ, farZ);
    UpdateViewProj();
}

void Camera::SetPerspective(glm::float32 fovY, glm::float32 aspect, glm::float32 nearZ, glm::float32 farZ)
{
    m_type = PERSPECTIVE;
    m_proj = glm::perspective(fovY, aspect, nearZ, farZ);
    UpdateViewProj();
}
