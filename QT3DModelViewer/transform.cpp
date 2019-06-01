#include "transform.h"
#include "gameobject.h"
//#include <math.h>

Transform::Transform(GameObject* go, bool isActive, QVector3D pos, QVector3D rot, QVector3D scale) :
    Component(TRANSFORM, go, isActive),
    local_pos(pos),
    local_rot(rot),
    local_scale(scale)
{
    GetWorldMatrix();
}

Transform::~Transform()
{}

void Transform::Reset()
{
    local_pos *= 0;
    local_rot *= 0;
    local_scale = {1,1,1};
    isUpdated = false;
}

void Transform::SetPos(QVector3D pos)
{
    local_pos = pos;
    isUpdated = false;
}

void Transform::Translate(QVector3D pos)
{
    local_pos += pos;
    isUpdated = false;
}

void Transform::TranslateX(float x)
{
    local_pos.setX(local_pos.x() + x);
    isUpdated = false;
}

void Transform::TranslateY(float y)
{
    local_pos.setY(local_pos.y() + y);
    isUpdated = false;
}

void Transform::TranslateZ(float z)
{
    local_pos.setZ(local_pos.z() + z);
    isUpdated = false;
}

void Transform::TranslateLeft(float dist)
{
    if (!isUpdated)
        GetWorldMatrix();

    local_pos += local_left * dist;
    isUpdated = false;
}

void Transform::TranslateUp(float dist)
{
    if (!isUpdated)
        GetWorldMatrix();

    local_pos += local_up * dist;
    isUpdated = false;
}

void Transform::TranslateForward(float dist)
{
    if (!isUpdated)
        GetWorldMatrix();

    local_pos += local_forward * dist;
    isUpdated = false;
}

void Transform::SetRotXYZ(QVector3D rot)
{
    local_rot = rot;
    isUpdated = false;
}

void Transform::RotateX(float x)
{
    local_rot.setX(local_rot.x() + x);
    isUpdated = false;
}

void Transform::RotateY(float y)
{
    local_rot.setY(local_rot.y() + y);
    isUpdated = false;
}

void Transform::RotateZ(float z)
{
    local_rot.setZ(local_rot.z() + z);
    isUpdated = false;
}

void Transform::SetScale(QVector3D scale)
{
    local_scale = scale;
    isUpdated = false;
}

void Transform::Save(QDataStream &stream) {}
void Transform::Load(QDataStream &stream) {}
void Transform::CleanUp() {}

QMatrix4x4 Transform::GetWorldMatrix()
{
    if (!isUpdated)
    {
        world_m.setToIdentity();

        if (gameobject != nullptr && gameobject->parent != nullptr)
            world_m = gameobject->parent->transform->GetWorldMatrix();

        world_m.translate(local_pos);
        world_m.rotate(local_rot.x(), 1.0f, 0.0f, 0.0f);
        world_m.rotate(local_rot.y(), 0.0f, 1.0f, 0.0f);
        world_m.rotate(local_rot.z(), 0.0f, 0.0f, 1.0f);
        world_m.scale(local_scale);

        local_left = world_m.column(0).toVector3D().normalized();
        local_up = world_m.column(1).toVector3D().normalized();
        local_forward = world_m.column(2).toVector3D().normalized();

        isUpdated = true;
    }

    return world_m;
}
