#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include <QString>
#include <QVector>
#include <QVector3D>
#include <qtreewidget.h>
#include <QTreeWidgetItem>

#include "mesh.h"

class Component;
class Transform;
class MyOpenGLWidget;

class GameObject
{
public:
    GameObject(QString name = "untitled_go",
               GameObject* parent = nullptr,
               bool isActive = true,
               QVector3D pos = QVector3D(0,0,0),
               QVector3D rot = QVector3D(0,0,0),
               QVector3D scale = QVector3D(1,1,1));

    ~GameObject();

    void Draw(MyOpenGLWidget* renderer = nullptr);
    void Save(QDataStream& stream);
    void Load(QDataStream& stream);
    void CleanUp();


public:
    QString name = "untitled_go";
    int id = -1;
    QTreeWidgetItem *hierarchyItem;

    GameObject* parent = nullptr;
    QVector<GameObject*> childs;

    QVector<Component*> components;
    Transform* transform = nullptr;
};

#endif // GAMEOBJECT_H
