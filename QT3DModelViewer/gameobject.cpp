#include "gameobject.h"
#include "component.h"
#include "transform.h"
#include "mesh.h"

GameObject::GameObject(QString name,
                       GameObject* parent,
                       bool isActive,
                       QVector3D pos,
                       QVector3D rot,
                       QVector3D scale) :
    name(name),
    id(-1),
    parent(parent),
    mesh(nullptr)
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

/*template <class T>
T* GameObject::AddComponent(int i)
{
    T* ret = nullptr;

    switch(ComponentTYPE(i))
    {
    case MESH:
    {
        return new Mesh(this);
    }
    default: break;

    }

    return ret;
}*/

void GameObject::Draw(MyOpenGLWidget* renderer)
{
    // Check if active
    if (renderer == nullptr || transform == nullptr)
        return;

    if (mesh != nullptr && mesh->isActive)
        mesh->Draw(renderer);


    // Draw Childs
    QVector<GameObject*>::iterator child = childs.begin();
    for (; child != childs.end(); child++)
        (*child)->Draw(renderer);
}

void GameObject::Save(QDataStream& stream){}
void GameObject::Load(QDataStream& stream){}

void GameObject::CleanUp()
{
    components.clear();
    childs.clear();
}
