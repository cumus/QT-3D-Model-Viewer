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

void Transform::Draw(MyOpenGLWidget* renderer)
{}

void Transform::Save(QDataStream &stream) {}
void Transform::Load(QDataStream &stream) {}
void Transform::CleanUp() {}

QMatrix4x4 Transform::GetWorldMatrix()
{
    world_m.setToIdentity();

    if (gameobject->parent != nullptr)
        world_m = gameobject->parent->transform->GetWorldMatrix();

    world_m.translate(local_pos);
    world_m.rotate(local_rot.x(), 1.0f, 0.0f, 0.0f);
    world_m.rotate(local_rot.y(), 0.0f, 1.0f, 0.0f);
    world_m.rotate(local_rot.z(), 0.0f, 0.0f, 1.0f);
    world_m.scale(local_scale);

    return world_m;
}
