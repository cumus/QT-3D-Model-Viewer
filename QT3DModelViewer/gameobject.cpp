#include "gameobject.h"
#include "component.h"
#include "transform.h"

GameObject::GameObject(QString name,
                       GameObject* parent,
                       bool isActive,
                       QVector3D pos,
                       QVector3D rot,
                       QVector3D scale) :
    name(name),
    parent(parent)
{
    childs.clear();
    components.clear();

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
    if (renderer == nullptr || transform == nullptr || !transform->isActive)
        return;

    // Update Components
    for (QVectorIterator<Component*> comp(components);
         comp.hasNext();
         comp.next())
    {
        if (comp.peekNext()->type == MESH && comp.peekNext()->isActive)
            comp.peekNext()->Draw(renderer);
    }

    // Update Childs
    for (QVectorIterator<GameObject*> child(childs);
         child.hasNext();
         child.next())
        child.peekNext()->Draw(renderer);
}

void GameObject::Save(QDataStream& stream){}
void GameObject::Load(QDataStream& stream){}

void GameObject::CleanUp()
{
    components.clear();
    childs.clear();
}
