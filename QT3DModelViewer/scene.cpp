#include "scene.h"
#include "gameobject.h"
#include "component.h"
#include "transform.h"
#include "mesh.h"
#include "resources.h"

#include <QOpenGLBuffer>
#include <QOpenGLTexture>

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

void Scene::Draw(MyOpenGLWidget* renderer)
{
    root->Draw(renderer);
}

void Scene::InitDemo()
{
    GameObject* go = AddGameObject("Demo Cube");
    Mesh* mesh = new Mesh();
    go->components.push_back(mesh);


    static const int coords[6][4][3] = {
            { { +1, -1, -1 }, { -1, -1, -1 }, { -1, +1, -1 }, { +1, +1, -1 } },
            { { +1, +1, -1 }, { -1, +1, -1 }, { -1, +1, +1 }, { +1, +1, +1 } },
            { { +1, -1, +1 }, { +1, -1, -1 }, { +1, +1, -1 }, { +1, +1, +1 } },
            { { -1, -1, -1 }, { -1, -1, +1 }, { -1, +1, +1 }, { -1, +1, -1 } },
            { { +1, -1, +1 }, { -1, -1, +1 }, { -1, -1, -1 }, { +1, -1, -1 } },
            { { -1, -1, +1 }, { +1, -1, +1 }, { +1, +1, +1 }, { -1, +1, +1 } }
    };

    float scale = 0.2f;

    QVector<GLfloat> vertData;
    for (int i = 0; i < 6; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            // vertex position
            vertData.append(scale * coords[i][j][0]);
            vertData.append(scale * coords[i][j][1]);
            vertData.append(scale * coords[i][j][2]);

            // texture coordinate
            vertData.append(j == 0 || j == 3);
            vertData.append(j == 0 || j == 1);
        }
    }

    mesh->vbo_index = resources->AddVBO(&vertData);
    mesh->texture_index = resources->AddTex(":/icons/Resources/icons/Folder.png");
    go->transform->local_pos.setZ(-10);
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
