#ifndef CAMERA_H_INCLUDED
#define CAMERA_H_INCLUDED

#include "Transform.h"

enum CameraType
{
    ORTHOGRAPHIC,
    PERSPECTIVE
};

class Camera
{
public:
    Camera(glm::vec3 const& pos, Transform* parent = nullptr);
    void HandleEvent(float _dt);
    void SetOrthographic(glm::float32 left, glm::float32 right, glm::float32 bottom, glm::float32 top, glm::float32 nearZ, glm::float32 farZ);
    void SetPerspective(glm::float32 fovAngleY, glm::float32 aspectRatio, glm::float32 nearZ, glm::float32 farZ);

    glm::mat4 const& GetMatrix()     const { return transform.GetWorldMatrix(); }
    glm::mat4 const& GetViewMatrix() const { return m_view; }
    glm::mat4 const& GetProjMatrix() const { return m_proj; }
    glm::mat4 const& GetVPMatrix()   const { return m_viewProj; }
    // Par valeur : GetWorldPosition / GetWorldRotation renvoient un temporaire (une reference serait pendante).
    glm::vec3 GetPosition()          const { return transform.GetWorldPosition(); }
    glm::quat GetRotation()          const { return transform.GetWorldRotation(); }
    
    CameraType GetType() const { return m_type; }

    Transform transform;
private:
    void UpdateViewMatrix();
    void UpdateViewProj() { m_viewProj = m_proj * m_view; }

    glm::mat4 m_view     = glm::mat4(1.0f);
    glm::mat4 m_proj     = glm::mat4(1.0f);
    glm::mat4 m_viewProj = glm::mat4(1.0f);
    CameraType m_type = PERSPECTIVE;
};


#endif
