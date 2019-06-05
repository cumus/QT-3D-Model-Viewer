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
                data.data(), static_cast<int>(data.size()),
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
    aiMatrix4x4 t = node->mTransformation;
    aiVector3D pos, scale;
    aiQuaternion rot;
    t.Decompose(scale, rot, pos);

    transform->SetPos({pos.x, pos.y, pos.z});
    transform->SetRotQ({rot.x, rot.y, rot.z, rot.w});
    transform->SetScale({scale.x, scale.y, scale.z});

    qDebug() << "Node " << name.toStdString().c_str() << ": " << node->mNumChildren << " childs, " << node->mNumMeshes << " meshes"
             << "at: " << pos.x << pos.y << pos.z << "-" << scale.x << scale.y << scale.z;

    for(unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        if (scene->mMeshes[node->mMeshes[i]]->mNumVertices > 0)
            processMesh(scene->mMeshes[node->mMeshes[i]], scene, renderer);
    }

    for(unsigned int i = 0; i < node->mNumChildren; i++)
    {
        GameObject *go = new GameObject(node->mChildren[i]->mName.C_Str(),this);
        go->processNode(node->mChildren[i], scene, renderer);
    }
}

void GameObject::processMesh(aiMesh *aimesh, const aiScene *scene, MyOpenGLWidget* renderer)
{
    Mesh *mesh = new Mesh(this);

    //Process each MESH VERTEX, NORMAL & TEXCOORDS
    for (unsigned int i = 0; i < aimesh->mNumVertices; i++)
    {
        Vertex vertex;

        vertex.Position.setX(aimesh->mVertices[i].x);
        vertex.Position.setY(aimesh->mVertices[i].y);
        vertex.Position.setZ(aimesh->mVertices[i].z);

        vertex.Normal.setX(aimesh->mNormals[i].x);
        vertex.Normal.setY(aimesh->mNormals[i].y);
        vertex.Normal.setZ(aimesh->mNormals[i].z);

        if(aimesh->HasTextureCoords(0))
        {
            vertex.TexCoords.setX(aimesh->mTextureCoords[0][i].x);
            vertex.TexCoords.setY(aimesh->mTextureCoords[0][i].y);
        }
        else
            vertex.TexCoords = QVector2D(0.0f, 0.0f);

        mesh->vertices.push_back(vertex);
    }

    mesh->num_vertices = static_cast<int>(aimesh->mNumVertices);
    mesh->vertex_data.resize(mesh->num_vertices * 3);
    mesh->normal_data.resize(mesh->num_vertices * 3);
    mesh->texcoord_data.resize(mesh->num_vertices * 2);

    for (int i = 0; i < mesh->num_vertices; i++)
    {
        GLfloat* float_p = mesh->vertex_data.data() + (3 * i);
        *float_p++ = mesh->vertices[i].Position.x();
        *float_p++ = mesh->vertices[i].Position.y();
        *float_p++ = mesh->vertices[i].Position.z();

        float_p = mesh->normal_data.data() + (3 * i);
        *float_p++ = mesh->vertices[i].Normal.x();
        *float_p++ = mesh->vertices[i].Normal.y();
        *float_p++ = mesh->vertices[i].Normal.z();

        float_p = mesh->texcoord_data.data() + (2 * i);
        *float_p++ = mesh->vertices[i].TexCoords.x();
        *float_p++ = mesh->vertices[i].TexCoords.y();
    }

    //Process each MESH FACE, 3 INDEX each
    mesh->num_faces = static_cast<int>(aimesh->mNumFaces);
    mesh->index_data.resize(mesh->num_faces * 3);

    for(int i = 0; i < mesh->num_faces; i++)
    {
        aiFace face = aimesh->mFaces[i];
        GLuint* int_p = mesh->index_data.data() + (3 * i);
        *int_p++ = static_cast<GLuint>(face.mIndices[0]);
        *int_p++ = static_cast<GLuint>(face.mIndices[1]);
        *int_p++ = static_cast<GLuint>(face.mIndices[2]);
    }

    /*/Proces MATERIALS
    if(aimesh->mMaterialIndex > 0)
    {
        aiMaterial *material = scene->mMaterials[aimesh->mMaterialIndex];
        QVector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
        QVector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
    }*/

    qDebug() << " - Mesh Loading: " << aimesh->mName.C_Str() << " with " << mesh->vertices.count()<< " vertices";

    renderer->LoadMesh(mesh);
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
