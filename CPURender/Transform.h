#ifndef TRANSFORM_H_INCLUDED
#define TRANSFORM_H_INCLUDED

#include <cstdint>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

// Position / rotation / echelle d'un objet. Ne depend d'aucun systeme de rendu :
// ceux qui ont besoin de savoir si l'objet a bouge comparent Version().
struct Transform
{
public:
    Transform() = default;
    Transform(glm::vec3 _position, glm::quat _rotation, glm::vec3 _scale)
        : position(_position), scale(_scale), rotation(_rotation) {}

    const glm::vec3& GetPosition() const { return position; }
    const glm::vec3& GetScale()    const { return scale; }
    const glm::quat& GetRotation() const { return rotation; }

    glm::vec3 GetForward() const { return rotation * glm::vec3(0, 0, 1); }
    glm::vec3 GetRight()   const { return rotation * glm::vec3(1, 0, 0); }
    glm::vec3 GetUp()      const { return rotation * glm::vec3(0, 1, 0); }

    const glm::mat4& GetWorldMatrix() const;
    // Pas de hierarchie pour l'instant : local == monde.
    const glm::mat4& GetLocalMatrix() const { return GetWorldMatrix(); }

    // Incremente a chaque modification. Permet a un systeme (ex. ombres) de
    // detecter un changement sans que Transform le connaisse.
    uint32_t Version() const { return m_version; }

    void SetPosition(glm::vec3 _position);
    void SetScale(glm::vec3 _scale);
    void SetRotation(glm::quat _rotation);
    void SetYawPitchRoll(glm::vec3 _euler);

    void Move(glm::vec3 _dir);
    void LookAt(glm::vec3 _target);
    void OrbitAroundAxis(glm::vec3 _axis, float _angle, float _radius);

private:
    void MarkDirty() { m_dirty = true; ++m_version; }

    glm::vec3 position{ 0.0f };
    glm::vec3 scale{ 1.0f };
    glm::quat rotation{ 1.0f, 0.0f, 0.0f, 0.0f };

    mutable glm::mat4 m_world{ 1.0f };
    mutable bool      m_dirty   = true;
    uint32_t          m_version = 0;
};

#endif
