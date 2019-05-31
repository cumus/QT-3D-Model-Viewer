#ifndef TRANSFORM_H
#define TRANSFORM_H

#include "component.h"
#include <QVector3D>
#include <QMatrix4x4>

class Transform : public Component
{
public:
    Transform(GameObject* gameobject = nullptr,
              bool isActive = true,
              QVector3D pos = QVector3D(0,0,0),
              QVector3D rot = QVector3D(0,0,0),
              QVector3D scale = QVector3D(1,1,1));
    ~Transform() override;

    void Save(QDataStream& stream) override;
    void Load(QDataStream& stream) override;
    void CleanUp() override;

    QMatrix4x4 GetWorldMatrix();

public:
    QVector3D local_pos;
    QVector3D local_rot;
    QVector3D local_scale;
    QMatrix4x4 world_m;
};

#endif // TRANSFORM_H
