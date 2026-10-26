#ifndef CAMERA_H_INCLUDED
#define CAMERA_H_INCLUDED

#include "core.h"

enum CameraType
{
    ORTHOGRAPHIC,
    PERSPECTIVE
};

class Camera
{
public:
    void HandleEvent(float _dt);
    void SetOrthographic(glm::float32 viewWidth, glm::float32 viewHeight, glm::float32 nearZ, glm::float32 farZ);
    void SetPerspective(glm::float32 fovAngleY, glm::float32 aspectRatio, glm::float32 nearZ, glm::float32 farZ);

    void SetMatrix(glm::mat4 const& world) { m_world = world; UpdateViewMatrix(); }

    glm::mat4 const& GetMatrix()     const { return m_world; }
    glm::mat4 const& GetViewMatrix() const { return m_view; }
    glm::mat4 const& GetProjMatrix() const { return m_proj; }
    glm::mat4 const& GetVPMatrix()   const { return m_viewProj; }
    glm::vec3 const& GetCameraPos()   const { return m_camPos; }

    CameraType GetType() const { return m_type; }

private:
    void UpdateViewMatrix();
    void UpdateViewProj() { m_viewProj = m_proj * m_view; }

    glm::mat4 m_world    = glm::mat4(1.0f);
    glm::mat4 m_view     = glm::mat4(1.0f);
    glm::mat4 m_proj     = glm::mat4(1.0f);
    glm::mat4 m_viewProj = glm::mat4(1.0f);
    CameraType m_type = PERSPECTIVE;
    
    glm::vec3  m_camPos   = { 0.0f, 0.0f, 2.0f };
    float m_camYaw = 0.0f; 
    float m_camPitch = 0.0f;
};


#endif
