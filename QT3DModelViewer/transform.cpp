#include "transform.h"
#include "gameobject.h"
//#include <math.h>
#include <QTransform>

Transform::Transform(GameObject* go, bool isActive, QVector3D pos, QVector3D rot, QVector3D scale) :
    Component(TRANSFORM, go, isActive),
    local_pos(pos),
    local_rot(rot),
    local_scale(scale)
{
    local_qrot = local_qrot.fromEulerAngles(local_rot);
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

    local_pos += world_left * dist;
    isUpdated = false;
}

void Transform::TranslateUp(float dist)
{
    if (!isUpdated)
        GetWorldMatrix();

    local_pos += world_up * dist;
    isUpdated = false;
}

void Transform::TranslateForward(float dist)
{
    if (!isUpdated)
        GetWorldMatrix();

    local_pos += world_forward * dist;
    isUpdated = false;
}

void Transform::SetRotXYZ(QVector3D rot)
{
    local_rot = rot;
    local_qrot = local_qrot.fromEulerAngles(local_rot);

    isUpdated = false;
}

void Transform::SetRotQ(QQuaternion rot)
{
    local_qrot = rot;
    float yaw, pitch, roll;
    local_qrot.getEulerAngles(&pitch, &yaw, &roll);
    local_rot = {pitch, yaw, roll};
    isUpdated = false;
}

void Transform::RotateX(float x)
{
    local_rot.setX(local_rot.x() + x);
    local_qrot = local_qrot.fromEulerAngles(local_rot);
    isUpdated = false;
}

void Transform::RotateY(float y)
{
    local_rot.setY(local_rot.y() + y);
    local_qrot = local_qrot.fromEulerAngles(local_rot);
    isUpdated = false;
}

void Transform::RotateZ(float z)
{
    local_rot.setZ(local_rot.z() + z);
    local_qrot = local_qrot.fromEulerAngles(local_rot);
    isUpdated = false;
}

void Transform::RotateQ(QQuaternion rot)
{
    float yaw, pitch, roll;
    local_qrot *= rot;
    local_qrot.getEulerAngles(&pitch, &yaw, &roll);
    local_rot = {pitch, yaw, roll};
    isUpdated = false;
}

void Transform::RotateAngleAxis(float angle, QVector3D axis)
{
    if (!isUpdated)
        GetWorldMatrix();

    float yaw, pitch, roll;
    local_qrot = local_qrot * QQuaternion::fromAxisAndAngle(axis, angle);
    local_qrot.getEulerAngles(&pitch, &yaw, &roll);
    local_rot = {pitch, yaw, roll};
    isUpdated = false;
}

void Transform::RotateAxisLeft(float angle)
{
    if (!isUpdated)
        GetWorldMatrix();

    RotateAngleAxis(angle, {1,0,0});
}

void Transform::RotateAxisUp(float angle)
{
    if (!isUpdated)
        GetWorldMatrix();

    RotateAngleAxis(angle, {0,1,0});
}

void Transform::RotateAxisForward(float angle)
{
    if (!isUpdated)
        GetWorldMatrix();

    RotateAngleAxis(angle, {0,0,1});
}

void Transform::RemoveRoll()
{
    local_rot.setZ(0);
    isUpdated = false;
}

void Transform::SetScale(QVector3D scale)
{
    local_scale = scale;
    isUpdated = false;
}

void Transform::Focus(QVector3D focus)
{
    if (!isUpdated)
        GetWorldMatrix();

    local_qrot = QQuaternion::fromDirection(local_pos - focus, QVector3D::crossProduct(focus - local_pos, world_forward));

    // calc new world_left
    GetWorldMatrix();

    local_qrot = QQuaternion::fromDirection(local_pos - focus, QVector3D::crossProduct(focus - local_pos, world_left));

    float yaw, pitch, roll;
    local_qrot.getEulerAngles(&pitch, &yaw, &roll);
    local_rot = {pitch, yaw, roll};
    isUpdated = false;

    RotateAxisForward(-roll);
}

void Transform::Orbit(float x, float y, QVector3D focus)
{
    if (!isUpdated)
        GetWorldMatrix();

    float dist = local_pos.distanceToPoint(focus);
    local_pos = focus;

    QQuaternion rot;
    float yaw, pitch, roll;
    local_qrot = local_qrot * rot.fromAxisAndAngle({0,1,0}, x);
    local_qrot = local_qrot * rot.fromAxisAndAngle({1,0,0}, y);
    local_qrot.getEulerAngles(&pitch, &yaw, &roll);
    local_rot = {pitch, yaw, roll};

    GetWorldMatrix();

    local_pos -= world_forward * dist;
    isUpdated = false;
}

QVector3D Transform::GetPos() const
{
    return local_pos;
}

QVector3D Transform::GetRot() const
{
    return local_rot;
}

QVector3D Transform::GetScale() const
{
    return local_scale;
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
        world_m.rotate(local_qrot);
        world_m.scale(local_scale);

        world_left = -world_m.column(0).toVector3D().normalized();
        world_up = world_m.column(1).toVector3D().normalized();
        world_forward = -world_m.column(2).toVector3D().normalized();

        isUpdated = true;
    }

    return world_m;
}
