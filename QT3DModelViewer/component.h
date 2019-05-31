#ifndef COMPONENT_H
#define COMPONENT_H

#include <QDataStream>

class GameObject;

enum ComponentTYPE
{
    EMPTY,
    TRANSFORM,
    MESH,
    LIGHT
};

class Component
{
public:
    Component(ComponentTYPE type = EMPTY, GameObject* gameobject = nullptr, bool isActive = true);
    virtual ~Component();

    virtual void Save(QDataStream& stream) = 0;
    virtual void Load(QDataStream& stream) = 0;
    virtual void CleanUp() = 0;

public:

    ComponentTYPE type = EMPTY;
    GameObject* gameobject = nullptr;
    bool isActive = true;
};

#endif // COMPONENT_H
