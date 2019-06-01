#include "gameobject.h"
#include "component.h"
#include "transform.h"
#include "mesh.h"
#include <qfile.h>
#include "myopenglwidget.h"

GameObject::GameObject(QString name,
                       GameObject* parent,
                       bool isActive,
                       QVector3D pos,
                       QVector3D rot,
                       QVector3D scale) :
    name(name),
    id(-1),
    parent(parent)
{
    childs.clear();
    components.clear();

    if (parent!=nullptr) parent->childs.push_back(this);

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

    QVector<Component*>::iterator comp = components.begin();
    for (; comp != components.end(); comp++)
    {
        if ((*comp)->type == MESH && (*comp)->isActive)
            (*comp)->Draw(renderer);
    }

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

void GameObject::importModel(QString path, MyOpenGLWidget* renderer)
{
    Assimp::Importer import;
    //const aiScene *scene = import.ReadFile(path.toStdString(), aiProcess_Triangulate | aiProcess_FlipUVs);

    //##################
    QFile file(path);
    if(!file.open(QIODevice::ReadOnly))
    {
        qDebug() << file.errorString() << path << endl;
        qDebug() << "Could not open file for read: " << path << endl;
        return;
    }

    QByteArray data = file.readAll();

    const aiScene *scene = import.ReadFileFromMemory(
                data.data(), data.size(),
                aiProcess_Triangulate |
                aiProcess_FlipUVs |
                aiProcess_GenSmoothNormals |
                aiProcess_OptimizeMeshes |
                aiProcess_PreTransformVertices |
                aiProcess_ImproveCacheLocality ,
                ".obj");

    //##################

    if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        qDebug() << "ERROR::ASSIMP::" << import.GetErrorString() << endl;
        return;
    }

    processNode(scene->mRootNode, scene, renderer);
}

void GameObject::processNode(aiNode *node, const aiScene *scene, MyOpenGLWidget* renderer)
{
    for(unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        processMesh(scene->mMeshes[node->mMeshes[i]], scene, renderer);
    }
    for(unsigned int i = 0; i < node->mNumChildren; i++)
    {
        GameObject *go = new GameObject(node->mName.C_Str(),this);
        go->processNode(node->mChildren[i], scene, renderer);
    }
}

void GameObject::processMesh(aiMesh *aimesh, const aiScene *scene, MyOpenGLWidget* renderer)
{
    Mesh *mesh = new Mesh(this);

    QVector<Vertex> vertices;
    QVector<unsigned int> indices;
    QVector<Texture> textures;

    //Process MESHES NORMAL & TEXCOORDS
    for (unsigned int i = 0; i < aimesh->mNumVertices; i++)
    {
        Vertex vertex;

        QVector3D vector;
        vector.setX(aimesh->mVertices[i].x);
        vector.setY(aimesh->mVertices[i].y);
        vector.setZ(aimesh->mVertices[i].z);
        vertex.Position = vector;

        vector.setX(aimesh->mNormals[i].x);
        vector.setY(aimesh->mNormals[i].y);
        vector.setZ(aimesh->mNormals[i].z);
        vertex.Normal = vector;

        if(aimesh->mTextureCoords[0])
        {
            QVector2D vec;
            vec.setX(aimesh->mTextureCoords[0][i].x);
            vec.setY(aimesh->mTextureCoords[0][i].y);
            vertex.TexCoords = vec;
        }
        else
            vertex.TexCoords = QVector2D(0.0f, 0.0f);

        vertices.push_back(vertex);
    }

    //Process INDICES
    for(unsigned int i = 0; i < aimesh->mNumFaces; i++)
    {
        aiFace face = aimesh->mFaces[i];
        for(unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }

    //Proces MATERIALS
    if(aimesh->mMaterialIndex >= 0)
    {
        /*aiMaterial *material = scene->mMaterials[aimesh->mMaterialIndex];
        QVector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
        QVector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
    */
    }

    mesh->vertices = vertices;
    mesh->indices = indices;
    mesh->textures = textures;

   /* for (int i = 0; i < vertices.count();i++)
    {
        mesh->add(vertices[i].Position, vertices[i].Normal);
    }

    qDebug() << "MESH::ASSIMP::" << (renderer == nullptr) << mesh->count() << endl;

    renderer->LoadMesh(mesh);*/
}

QVector<Texture> GameObject::loadMaterialTextures(aiMaterial *mat, aiTextureType type, QString typeName)
{
    /*vector<Texture> textures;
    for(unsigned int i = 0; i < mat->GetTextureCount(type); i++)
    {
        aiString str;
        mat->GetTexture(type, i, &str);
        Texture texture;
        texture.id = TextureFromFile(str.C_Str(), directory);
        texture.type = typeName;
        texture.path = str;
        textures.push_back(texture);
    }
    return textures;
    */
    return QVector<Texture>();
}
