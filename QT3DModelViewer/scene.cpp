#include "scene.h"
#include "gameobject.h"
#include "resources.h"

Scene::Scene()
{
    root = new GameObject("root");
}

Scene::~Scene()
{
    Clear();
    delete root;
}

void Scene::Clear()
{
    delete root;
    root = new GameObject("root");
}

void Scene::Draw()
{
    root->Draw();
}

GameObject *Scene::AddGameObject(QString name, GameObject *parent)
{
    if (parent == nullptr)
        parent = root;

    GameObject* ret = new GameObject(name, parent);
    parent->childs.push_back(ret);
    return ret;
}

void Scene::RemoveEntity(int id)
{

}
