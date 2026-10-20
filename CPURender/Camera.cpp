#ifndef CAMERA_CPP_INCLUDED
#define CAMERA_CPP_INCLUDED

#include "Camera.h"

#include <glm/ext/matrix_clip_space.hpp>

void Camera::UpdateViewMatrix()
{
    m_view = inverse(m_world);
    UpdateViewProj();
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
