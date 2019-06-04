#ifndef SCENE_H
#define SCENE_H

#include <QString>

class MyOpenGLWidget;
//class Resources;
class GameObject;
class Mesh;

class Scene
{
public:
    Scene();
    ~Scene();

    Mesh* InitDemo(MyOpenGLWidget* renderer);

    void Clear();
    void Draw(MyOpenGLWidget* renderer);

    GameObject* AddGameObject(QString name, GameObject* parent = nullptr);
    void RemoveEntity(int id);

public:
    GameObject* root = nullptr;
    GameObject* goPatrick = nullptr;
    //Resources* resources = nullptr;
};

#endif // SCENE_H
