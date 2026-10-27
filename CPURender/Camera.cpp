#include "Camera.h"

#include <SDL3/SDL_keyboard.h>
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

    glm::mat4 rot = glm::rotate(glm::mat4(1.0f), m_camYaw, glm::vec3(0, 1, 0))
             * glm::rotate(glm::mat4(1.0f), m_camPitch, glm::vec3(1, 0, 0));

    glm::vec3 right   = glm::vec3(rot[0]);
    glm::vec3 forward = -glm::vec3(rot[2]);
    glm::vec3 up      = glm::vec3(rot[1]);

    if (keys[SDL_SCANCODE_W])     m_camPos += forward * speed * _dt;
    if (keys[SDL_SCANCODE_S])     m_camPos -= forward * speed * _dt;
    if (keys[SDL_SCANCODE_D])     m_camPos += right   * speed * _dt;
    if (keys[SDL_SCANCODE_A])     m_camPos -= right   * speed * _dt;
    if (keys[SDL_SCANCODE_SPACE]) m_camPos += up      * speed * _dt;
    if (keys[SDL_SCANCODE_LCTRL]) m_camPos -= up      * speed * _dt;

    glm::mat4 world = glm::translate(glm::mat4(1.0f), m_camPos) * rot;
    SetMatrix(world);
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
