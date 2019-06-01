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

    void Reset();
    void SetPos(QVector3D pos);
    void Translate(QVector3D pos);
    void TranslateX(float x);
    void TranslateY(float y);
    void TranslateZ(float z);
    void TranslateLeft(float dist);
    void TranslateUp(float dist);
    void TranslateForward(float dist);
    void SetRotXYZ(QVector3D rot);
    void RotateX(float x);
    void RotateY(float y);
    void RotateZ(float z);
    void SetScale(QVector3D scale);

    void Save(QDataStream& stream) override;
    void Load(QDataStream& stream) override;
    void CleanUp() override;

    QMatrix4x4 GetWorldMatrix();

private:
    QVector3D local_pos;
    QVector3D local_rot;
    QVector3D local_scale;

    QVector3D local_left;
    QVector3D local_up;
    QVector3D local_forward;

    bool isUpdated = false;
    QMatrix4x4 world_m;
};

#endif // TRANSFORM_H
