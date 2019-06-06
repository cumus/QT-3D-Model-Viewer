#include "mesh.h"
#include "gameobject.h"
#include "transform.h"
#include "myopenglwidget.h"
#include <qmath.h>
#include <qfile.h>

Mesh::Mesh(GameObject* go, bool isActive) :
    Component(MESH, go, isActive)
{
    if (go != nullptr)
        go->components.push_back(this);
}

Mesh::~Mesh()
{
    CleanUp();
}

void Mesh::Draw(MyOpenGLWidget* renderer)
{
    renderer->DrawMesh(this);
}

void Mesh::Save(QDataStream &stream) {}
void Mesh::Load(QDataStream &stream) {}

void Mesh::CleanUp()
{

}

void Mesh::importModel(QString path, MyOpenGLWidget* renderer)
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
                data.data(), static_cast<size_t>(data.size()),
                aiProcess_Triangulate |
                aiProcess_FlipUVs |
                aiProcess_GenSmoothNormals |
                aiProcess_OptimizeMeshes |
                aiProcess_PreTransformVertices |
                aiProcess_ImproveCacheLocality,
                ".obj");

    //##################

    if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        qDebug() << "ERROR::ASSIMP::" << import.GetErrorString() << endl;
        return;
    }

    directory = path;
    for (int i = directory.length() - 1; i > 0; i--)
    {
        if(directory[i] != "/") directory.remove(i,1);
        else break;
    }

    qDebug() << "Mesh at;" << directory;

    processNode(scene->mRootNode, scene, renderer);

    file.close();
}

void Mesh::processNode(aiNode *node, const aiScene *scene, MyOpenGLWidget* renderer)
{
    aiMatrix4x4 t = node->mTransformation;
    aiVector3D pos, scale;
    aiQuaternion rot;
    t.Decompose(scale, rot, pos);

    gameobject->transform->SetPos({pos.x, pos.y, pos.z});
    gameobject->transform->SetRotQ({rot.x, rot.y, rot.z, rot.w});
    gameobject->transform->SetScale({scale.x, scale.y, scale.z});

    for(unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        sub_meshes.push_back(processMesh(mesh, scene, renderer));
    }

    for(unsigned int i = 0; i < node->mNumChildren; i++)
    {
        processNode(node->mChildren[i], scene, renderer);
    }
}

SubMesh* Mesh::processMesh(aiMesh *aimesh, const aiScene *scene, MyOpenGLWidget* renderer)
{
    SubMesh *sub_mesh = new SubMesh();

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

        sub_mesh->vertices.push_back(vertex);
    }

    sub_mesh->num_vertices = static_cast<int>(aimesh->mNumVertices);
    sub_mesh->vertex_data.resize(sub_mesh->num_vertices * 3);
    sub_mesh->normal_data.resize(sub_mesh->num_vertices * 3);
    sub_mesh->texcoord_data.resize(sub_mesh->num_vertices * 2);

    for (int i = 0; i < sub_mesh->num_vertices; i++)
    {
        GLfloat* float_p = sub_mesh->vertex_data.data() + (3 * i);
        *float_p++ = sub_mesh->vertices[i].Position.x();
        *float_p++ = sub_mesh->vertices[i].Position.y();
        *float_p++ = sub_mesh->vertices[i].Position.z();

        float_p = sub_mesh->normal_data.data() + (3 * i);
        *float_p++ = sub_mesh->vertices[i].Normal.x();
        *float_p++ = sub_mesh->vertices[i].Normal.y();
        *float_p++ = sub_mesh->vertices[i].Normal.z();

        float_p = sub_mesh->texcoord_data.data() + (2 * i);
        *float_p++ = sub_mesh->vertices[i].TexCoords.x();
        *float_p++ = sub_mesh->vertices[i].TexCoords.y();
    }

    //Process each MESH FACE, 3 INDEX each
    sub_mesh->num_faces = static_cast<int>(aimesh->mNumFaces);
    sub_mesh->index_data.resize(sub_mesh->num_faces * 3);

    for(int i = 0; i < sub_mesh->num_faces; i++)
    {
        aiFace face = aimesh->mFaces[i];
        GLuint* int_p = sub_mesh->index_data.data() + (3 * i);
        *int_p++ = static_cast<GLuint>(face.mIndices[0]);
        *int_p++ = static_cast<GLuint>(face.mIndices[1]);
        *int_p++ = static_cast<GLuint>(face.mIndices[2]);
    }

    //Proces MATERIALS
    if(scene->HasMaterials())
    {
        aiString path;
        aiMaterial* mat = scene->mMaterials[aimesh->mMaterialIndex];

        if(aiReturn_SUCCESS == aiGetMaterialTexture(mat, aiTextureType_DIFFUSE, 0, &path))
        {
            QString file = directory;
            file += path.C_Str();
            qDebug() << path.C_Str();

            QImage image(file);
            if (!image.isNull())
                sub_mesh->texture = new QOpenGLTexture(image);
        }
        else
        {
            qDebug() << "ERROR::ASSIMP:: could not load material " << aimesh->mMaterialIndex;

            QString folder = "/";
            for (int i = directory.length() - 2; i > 0; i--)
            {
                if(directory[i] != "/") folder = directory[i] + folder;
                else break;
            }

            // hardcoded loading textures
            if (folder == "Patrick/")
            {
                switch (aimesh->mMaterialIndex)
                {
                case 1: sub_mesh->texture = new QOpenGLTexture(QImage(directory + "Color.png")); break; // brows
                case 2: sub_mesh->texture = new QOpenGLTexture(QImage(directory + "Color.png")); break; // eyes
                case 3: sub_mesh->texture = new QOpenGLTexture(QImage(directory + "Color.png")); break; // pupils
                case 4: sub_mesh->texture = new QOpenGLTexture(QImage(directory + "Flowers.png")); break; // pants
                case 5: sub_mesh->texture = new QOpenGLTexture(QImage(directory + "Skin_Patrick.png")); break; //body
                case 6: sub_mesh->texture = new QOpenGLTexture(QImage(directory + "Color.png")); break; // mouth
                default: break;
                }
            }
        }
    }

    qDebug() << " - Mesh Loading: " << aimesh->mName.C_Str() << " with " << sub_mesh->vertices.count()<< " vertices";

    renderer->LoadMesh(sub_mesh);

    return sub_mesh;
}
