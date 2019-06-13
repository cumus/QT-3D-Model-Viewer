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
    goPatrick->transform->SetPos({-2,0,0});
    Mesh* mesh = new Mesh(goPatrick);
    mesh->importModel(qApp->applicationDirPath() + "/Models/Patrick/Patrick.obj", renderer);
    mesh->draw_border = true;



    //mesh2->draw_border = true;

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

    if(loadNewModel)
    {
        GameObject* loadedModelGO = AddGameObject("NoName");
        Mesh* loadedModelMESH = new Mesh(loadedModelGO);
        loadedModelMESH->importModel(newModelPath, renderer);
        //loadedModelMESH->draw_border = true;

        loadNewModel = false;
    }
}

GameObject *Scene::AddGameObject(QString name, GameObject *parent)
{
    if (parent == nullptr)
        parent = root;

    GameObject* ret = new GameObject(name, parent);
    return ret;
}

void Scene::RemoveEntity(int id)
{

}
