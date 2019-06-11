#include "scene.h"
#include "gameobject.h"
#include "component.h"
#include "transform.h"
#include "mesh.h"

#include <QOpenGLBuffer>
#include <QOpenGLTexture>
#include <QApplication>

Scene::Scene()
{
    root = new GameObject("root");
}

Scene::~Scene()
{
    Clear();
    delete root;
}

Mesh* Scene::InitDemo(MyOpenGLWidget* renderer)
{
    qDebug() << "HERE" << qApp->applicationDirPath();

    goPatrick = AddGameObject("Patrick");
    goPatrick->transform->SetPos({-1,0,0});
    Mesh* mesh = new Mesh(goPatrick);
    mesh->importModel(qApp->applicationDirPath() + "/Models/Patrick/Patrick.obj", renderer);
    mesh->draw_border = true;

    /*GameObject* p2 = AddGameObject("Patrick 2");
    p2->transform->SetPos({1,0,0});
    Mesh* mesh2 = new Mesh(p2);
    mesh2->importModel(qApp->applicationDirPath() + "/Models/Patrick/Patrick.obj", renderer);
    //mesh2->draw_border = true;*/

    return nullptr;
}

void Scene::Clear()
{
    delete root;
    root = new GameObject("root");
}

void Scene::Draw(MyOpenGLWidget* renderer)
{
    root->Draw(renderer);
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
