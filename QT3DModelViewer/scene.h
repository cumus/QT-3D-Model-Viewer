#ifndef SCENE_H
#define SCENE_H

#include <QString>

class Resources;
class GameObject;

class Scene
{
public:
    Scene();
    ~Scene();

    void Clear();
    void Draw();

    GameObject* AddGameObject(QString name, GameObject* parent = nullptr);
    void RemoveEntity(int id);

public:
    GameObject* root = nullptr;
    Resources* resources = nullptr;
};

#endif // SCENE_H
