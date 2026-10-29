#ifndef TRANSFORM_H_INCLUDED
#define TRANSFORM_H_INCLUDED

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

struct Transform
{
public:
    Transform() = default;
    Transform(glm::vec3 _position, glm::quat _rotation, glm::vec3 _scale)
        : position(_position), scale(_scale), rotation(_rotation) {}

    Transform(const Transform&) = default;
    Transform(Transform&&) = default;
    Transform& operator=(const Transform&) = default;
    Transform& operator=(Transform&&) = default;
    
    void Update();
    uint32_t Version() const { return m_version; }
    // Local
    const glm::vec3& GetPosition() const { return position; }
    const glm::vec3& GetScale()    const { return scale; }
    const glm::quat& GetRotation() const { return rotation; }

    glm::vec3 GetWorldPosition() const { UpdateMatrix(); return glm::vec3(world[3]); }
    glm::quat GetWorldRotation() const;

    glm::vec3 GetForward() const { UpdateMatrix(); return -glm::normalize(glm::vec3(local[2])); }
    glm::vec3 GetRight()   const { UpdateMatrix(); return  glm::normalize(glm::vec3(local[0])); }
    glm::vec3 GetUp()      const { UpdateMatrix(); return  glm::normalize(glm::vec3(local[1])); }

    const glm::mat4& GetWorldMatrix() const { UpdateMatrix(); return world; }
    const glm::mat4& GetLocalMatrix() const { UpdateMatrix(); return local; }

    void SetPosition(glm::vec3 _position);
    void SetScale(glm::vec3 _scale);
    void SetRotation(glm::quat _rotation);

    // _ypr.x = yaw (axe Y monde), _ypr.y = pitch (axe X local), _ypr.z = roll (axe Z local)
    void SetYawPitchRoll(glm::vec3 _ypr);
    void SetParent(Transform* _parent);

    void AddYPR(const glm::vec3& ypr);
    void AddScale(const glm::vec3& _scale);
    void AddScale(const float& _scale);
    void Move(glm::vec3 _dir);

    void LookAt(glm::vec3 _target);
    void OrbitAroundAxis(glm::vec3 _axis, float _angle, float _radius);

private:
    void UpdateMatrix() const;
    void MakeDirty() { m_version++; isDirty = true;}

    glm::vec3 position{0.0f};
    glm::vec3 scale{1.0f};
    glm::quat rotation{1.0f, 0.0f, 0.0f, 0.0f};

    Transform* parent = nullptr;

    mutable glm::mat4 world{1.0f};
    mutable glm::mat4 local{1.0f};
    mutable bool isDirty = true;
    uint32_t          m_version = 0;
};

#endif