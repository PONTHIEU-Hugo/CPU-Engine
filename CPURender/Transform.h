#ifndef TRANSFORM_H_INCLUDED
#define TRANSFORM_H_INCLUDED

#include <glm/gtc/quaternion.hpp>

struct Transform
{
public:
    
    Transform() : 
        position(glm::vec3()),
        scale(glm::vec3(1.0f, 1.0f, 1.0f)),
        rotation(glm::quat(1.0f, 0.0f, 0.0f, 0.0f)),
        forward({0.0f, 0.0f, 1.0f}),
        right({1.0f, 0.0f, 0.0f}),
        up({0.0f, 1.0f, 0.0f}),
        world(glm::mat4(1.0f)),
        local(glm::mat4(1.0f)) { UpdateMatrix(); }
    
    Transform(glm::vec3 _position, glm::quat _rotation, glm::vec3 _scale) : 
        position(_position),
        scale(_scale),
        rotation(_rotation),
        forward({0.0f, 0.0f, 1.0f}),
        right({1.0f, 0.0f, 0.0f}),
        up({0.0f, 1.0f, 0.0f}),
        world(glm::mat4(1.0f)),
        local(glm::mat4(1.0f)) { UpdateMatrix(); }
    
    Transform(const Transform&) = default;
    Transform(Transform&&) = default;
    Transform& operator=(const Transform&) = default;
    Transform& operator=(Transform&&) = default;
    
    void UpdateMatrix() const;
    
    const glm::vec3& GetPosition() const { return position; }
    const glm::vec3& GetScale() const { return scale; }
    const glm::quat& GetRotation() const { return rotation; }

    glm::vec3 GetForward() const { return rotation * glm::vec3(0, 0, 1); }
    glm::vec3 GetRight()   const { return rotation * glm::vec3(1, 0, 0); }
    glm::vec3 GetUp()      const { return rotation * glm::vec3(0, 1, 0); }
    
    const glm::mat4& GetWorldMatrix() const { UpdateMatrix(); return world; }
    const glm::mat4& GetLocalMatrix() const { UpdateMatrix(); return local; }
    
    void SetPosition(glm::vec3 _position);
    void SetScale(glm::vec3 _scale);
    void SetRotation(glm::quat _rotation);
    void SetYawPitchRoll(glm::vec3 _euler);
    
    void Move(glm::vec3 _dir);
    void LookAt(glm::vec3 _target);
    void OrbitAroundAxis(glm::vec3 _axis, float _angle, float _radius);
    
private:
    glm::vec3 position;
    glm::vec3 scale;
    glm::quat rotation;
    
    glm::vec3 forward;
    glm::vec3 right;
    glm::vec3 up;
    
    mutable glm::mat4 world{1.0f};
    mutable glm::mat4 local{1.0f};
    mutable bool isDirty = true;
    
};

#endif
