#ifndef TRANSFORM_H_INCLUDED
#define TRANSFORM_H_INCLUDED

#include "core.h"

struct Transform
{
public:
    Transform(glm::vec3 _position, glm::vec4 _rotation, glm::vec3 _scale) : 
        position(_position),
        rotation(_rotation),
        scale(_scale),
        forward({0.0f, 0.0f, 1.0f}),
        right({1.0f, 0.0f, 0.0f}),
        up({0.0f, 1.0f, 0.0f}),
        wolrd(glm::mat4(1.0f)),
        local(glm::mat4(1.0f)) { UpdateMatrix(); }
    
    
    Transform(Transform& transform) { *this = transform; }
    
    void UpdateMatrix();
    
    void operator=(const Transform& transform);
    
    glm::vec3 const GetPosition() { return position; }
    glm::vec3 const GetScale() { return scale; }
    glm::vec4 const GetRotation() { return rotation; }
    
    glm::mat4 const GetWorldMatrix() { UpdateMatrix(); return wolrd; }
    glm::mat4 const GetLocalMatrix() { UpdateMatrix(); return local; }
    
    void SetPosition(glm::vec3 _position);
    void SetScale(glm::vec3 _scale);
    void SetRotation(glm::vec4 _rotation);
    void SetYawPitchRoll(glm::vec3 _euler);
    
    void Move(glm::vec3 _dir);
    void LookAt(glm::vec3 _target);
    void OrbitAroundAxis(glm::vec3 _axis, float _angle, float _radius);
    
private:
    glm::vec3 position;
    glm::vec3 scale;
    glm::vec4 rotation;
    
    glm::vec3 forward;
    glm::vec3 right;
    glm::vec3 up;
    
    glm::mat4 wolrd;
    glm::mat4 local;
    
    bool isDirty = false;
    
};

#endif
