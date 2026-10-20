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
    void SetOrthographic(float32 viewWidth, float32 viewHeight, float32 nearZ, float32 farZ);
    void SetPerspective(float32 fovAngleY, float32 aspectRatio, float32 nearZ, float32 farZ);

    void SetMatrix(mat4 const& world) { m_world = world; UpdateViewMatrix(); }

    mat4 const& GetMatrix()     const { return m_world; }
    mat4 const& GetViewMatrix() const { return m_view; }
    mat4 const& GetProjMatrix() const { return m_proj; }
    mat4 const& GetVPMatrix()   const { return m_viewProj; }

    CameraType GetType() const { return m_type; }

private:
    void UpdateViewMatrix();
    void UpdateViewProj() { m_viewProj = m_proj * m_view; }

    mat4 m_world    = mat4(1.0f);
    mat4 m_view     = mat4(1.0f);
    mat4 m_proj     = mat4(1.0f);
    mat4 m_viewProj = mat4(1.0f);
    CameraType m_type = PERSPECTIVE;
};


#endif
