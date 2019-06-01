#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include <QString>
#include <QVector>
#include <QVector3D>

#include "mesh.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

class Component;
class Transform;
class MyOpenGLWidget;

class GameObject
{
public:
    GameObject(QString name = "untitled_go",
               GameObject* parent = nullptr,
               bool isActive = true,
               QVector3D pos = QVector3D(0,0,0),
               QVector3D rot = QVector3D(0,0,0),
               QVector3D scale = QVector3D(1,1,1));

    ~GameObject();

    void Draw(MyOpenGLWidget* renderer = nullptr);
    void Save(QDataStream& stream);
    void Load(QDataStream& stream);
    void CleanUp();

    void importModel(QString path);
    void processNode(aiNode *node, const aiScene *scene);
    void processMesh(aiMesh *aimesh, const aiScene *scene);
    QVector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, QString typeName);

public:
    QString name = "untitled_go";
    int id = -1;

    GameObject* parent = nullptr;
    QVector<GameObject*> childs;

    QVector<Component*> components;
    Transform* transform = nullptr;
};

#endif // GAMEOBJECT_H
