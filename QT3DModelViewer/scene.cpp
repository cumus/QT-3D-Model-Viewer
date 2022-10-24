#include "scene.h"
#include "gameobject.h"
#include "component.h"
#include "mesh.h"

#include <QOpenGLBuffer>
#include <QOpenGLTexture>
#include <QApplication>
#include <QDebug>

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
    Mesh* mesh = new Mesh(goPatrick);
    mesh->importModel(qApp->applicationDirPath() + "/Models/Patrick/Patrick.obj", renderer);
    //mesh->draw_border = true;

    mainWindow->reloadHierarchy();

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

        mainWindow->reloadHierarchy();
    }
}

GameObject *Scene::AddGameObject(QString name, GameObject *parent)
{
    if (parent == nullptr)
        parent = root;

    GameObject* ret = new GameObject(name, parent);
    ret->id = goId;
    goId++;
    return ret;
}

void Scene::RemoveEntity(int id)
{
    // TODO: Reference-count resources to free unused only
}
