#include "gameobject.h"
#include "component.h"
#include "transform.h"
#include "mesh.h"
#include "myopenglwidget.h"

GameObject::GameObject(QString name,
                       GameObject* parent,
                       bool isActive,
                       QVector3D pos,
                       QVector3D rot,
                       QVector3D scale) :
    name(name),
    id(-1),
    parent(parent)
{
    childs.clear();
    components.clear();

    if (parent!=nullptr) parent->childs.push_back(this);

    transform = new Transform(this, isActive, pos, rot, scale);
    components.push_back(transform);
}

GameObject::~GameObject()
{
    CleanUp();
}

void GameObject::Draw(MyOpenGLWidget* renderer)
{
    // Check if active
    if (renderer == nullptr || transform == nullptr)
        return;

    QVector<Component*>::iterator comp = components.begin();
    for (; comp != components.end(); comp++)
    {
        if ((*comp)->type == MESH && (*comp)->isActive)
            (*comp)->Draw(renderer);
    }

    // Draw Childs
    QVector<GameObject*>::iterator child = childs.begin();
    for (; child != childs.end(); child++)
        if((*child)->transform->isActive)
            (*child)->Draw(renderer);
}

void GameObject::Save(QDataStream& stream){}
void GameObject::Load(QDataStream& stream){}

void GameObject::CleanUp()
{
    components.clear();
    childs.clear();
}
