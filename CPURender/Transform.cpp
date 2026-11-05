#include "Transform.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include "ShadowMap.h"

void Transform::UpdateMatrix() const
{
    bool changed = isDirty;

    if (isDirty)
    {
        local = glm::translate(glm::mat4(1.0f), position)
              * glm::mat4_cast(rotation)
              * glm::scale(glm::mat4(1.0f), scale);
        isDirty = false;
    }

    const glm::mat4 newWorld = parent ? parent->GetWorldMatrix() * local : local;

    if (changed || newWorld != world)
    {
        world = newWorld;
    }
}

void Transform::Update()
{
    UpdateMatrix();
}

glm::quat Transform::GetWorldRotation() const
{
    const glm::mat3 m(GetRight(), GetUp(), -GetForward());
    return glm::normalize(glm::quat_cast(m));
}

void Transform::SetPosition(glm::vec3 _position)
{
    position = _position;
    MakeDirty();
}

void Transform::SetScale(glm::vec3 _scale)
{
    scale = _scale;
    MakeDirty();
}

void Transform::SetRotation(glm::quat _rotation)
{
    rotation = glm::normalize(_rotation);
    MakeDirty();
}

void Transform::SetParent(Transform* _parent)
{
    if (_parent == this) return;

    parent = _parent;
    MakeDirty();
}

void Transform::SetYawPitchRoll(glm::vec3 _ypr)
{
    const glm::vec3 axisY(0.0f, 1.0f, 0.0f);
    const glm::vec3 axisX(1.0f, 0.0f, 0.0f);
    const glm::vec3 axisZ(0.0f, 0.0f, 1.0f);

    rotation = glm::angleAxis(_ypr.x, axisY)
             * glm::angleAxis(_ypr.y, axisX)
             * glm::angleAxis(_ypr.z, axisZ);
    rotation = glm::normalize(rotation);
    MakeDirty();
}

void Transform::AddYPR(const glm::vec3& ypr)
{
    // ypr.x = yaw (axe Y monde), ypr.y = pitch (axe X local), ypr.z = roll (axe Z local)
    const glm::vec3 axisY(0.0f, 1.0f, 0.0f);
    const glm::vec3 axisX(1.0f, 0.0f, 0.0f);
    const glm::vec3 axisZ(0.0f, 0.0f, 1.0f);

    if (ypr.x != 0.0f)
        rotation = glm::angleAxis(ypr.x, axisY) * rotation;

    if (ypr.y != 0.0f) // axe local
        rotation = rotation * glm::angleAxis(ypr.y, axisX);

    if (ypr.z != 0.0f) // axe local
        rotation = rotation * glm::angleAxis(ypr.z, axisZ);

    rotation = glm::normalize(rotation);
    MakeDirty();
}

void Transform::AddScale(const glm::vec3& _scale)
{
    scale += _scale;
    MakeDirty();
}

void Transform::AddScale(const float& _scale)
{
    scale += _scale;
    MakeDirty();
}

void Transform::Move(glm::vec3 _dir)
{
    position += _dir;
    MakeDirty();
}

void Transform::LookAt(glm::vec3 _target)
{
    glm::vec3 f = _target - GetWorldPosition();
    if (glm::dot(f, f) < 1e-12f) return;
    f = glm::normalize(f);

    const glm::vec3 worldUp = (glm::abs(f.y) > 0.999f) ? glm::vec3(0, 0, 1) : glm::vec3(0, 1, 0);

    const glm::vec3 r = glm::normalize(glm::cross(f, worldUp));
    const glm::vec3 u = glm::cross(r, f);

    glm::quat worldRot = glm::quat_cast(glm::mat3(r, u, -f));

    if (parent)
        rotation = glm::normalize(glm::inverse(parent->GetWorldRotation()) * worldRot);
    else
        rotation = glm::normalize(worldRot);

    MakeDirty();
}

void Transform::OrbitAroundAxis(glm::vec3 _axis, float _angle, float _radius)
{
    if (glm::dot(_axis, _axis) < 1e-12f) return;
    glm::vec3 axis = glm::normalize(_axis);

    if (parent)
        axis = glm::normalize(glm::inverse(parent->GetWorldRotation()) * axis);

    const glm::vec3 along  = axis * glm::dot(position, axis);
    glm::vec3       radial = position - along;

    if (_radius > 0.0f)
    {
        if (glm::dot(radial, radial) < 1e-12f)
        {
            const glm::vec3 ref = (glm::abs(axis.y) < 0.99f) ? glm::vec3(0, 1, 0)
                                                             : glm::vec3(1, 0, 0);
            radial = glm::cross(axis, ref);
        }
        radial = glm::normalize(radial) * _radius;
    }

    const glm::quat q = glm::angleAxis(_angle, axis);
    position = along + q * radial;
    rotation = glm::normalize(q * rotation);

    MakeDirty();
}