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
    Camera(vec3 const& pos, Transform* parent = nullptr);
    void HandleEvent(float _dt);
    void SetOrthographic(float32 viewWidth, float32 viewHeight, float32 nearZ, float32 farZ);
    void SetPerspective(float32 fovAngleY, float32 aspectRatio, float32 nearZ, float32 farZ);

    mat4 const& GetMatrix()     const { return transform.GetWorldMatrix(); }
    mat4 const& GetViewMatrix() const { return m_view; }
    mat4 const& GetProjMatrix() const { return m_proj; }
    mat4 const& GetVPMatrix()   const { return m_viewProj; }
    vec3 const& GetPosition()   const { return transform.GetWorldPosition(); }
    glm::quat const& GetRotation()   const { return transform.GetWorldRotation(); }
    
    CameraType GetType() const { return m_type; }

    Transform transform;
private:
    void UpdateViewMatrix();
    void UpdateViewProj() { m_viewProj = m_proj * m_view; }

    mat4 m_view     = mat4(1.0f);
    mat4 m_proj     = mat4(1.0f);
    mat4 m_viewProj = mat4(1.0f);
    CameraType m_type = PERSPECTIVE;
};


#endif
