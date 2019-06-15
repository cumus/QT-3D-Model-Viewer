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

    const aiScene *scene = import.ReadFile(
                    path.toStdString(),
                    aiProcess_CalcTangentSpace |
                    aiProcess_Triangulate |
                    aiProcess_GenSmoothNormals |
                    aiProcess_FixInfacingNormals |
                    aiProcess_JoinIdenticalVertices |
                    aiProcess_PreTransformVertices |
                    aiProcess_FlipUVs |
                    aiProcess_OptimizeMeshes);

    if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        qDebug() << "ERROR::ASSIMP::" << import.GetErrorString() << endl;
        return;
    }
    bool extension=true;
    QString name;
    directory = path;
    for (int i = directory.length() - 1; i > 0; i--)
    {
        if(directory[i] != "/")
        {
            if(!extension)name=directory[i]+name;
            if(directory[i] == ".") extension=false;
            directory.remove(i,1);
        }
        else break;
    }

    qDebug() << "Mesh at;" << directory;

    gameobject->name = name;
    processNode(scene->mRootNode, scene, renderer);

    //file.close();
}

void Mesh::processNode(aiNode *node, const aiScene *scene, MyOpenGLWidget* renderer)
{
    aiMatrix4x4 t = node->mTransformation;
    aiVector3D pos, scale;
    aiQuaternion rot;
    t.Decompose(scale, rot, pos);
    gameobject->transform->Translate({pos.x, pos.y, pos.z});
    gameobject->transform->RotateQ({rot.x, rot.y, rot.z, rot.w});
    gameobject->transform->RotateZ(-180.0f); // flip mesh
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

        if(aimesh->HasTangentsAndBitangents())
        {
            vertex.Tangent.setX(aimesh->mTangents[i].x);
            vertex.Tangent.setY(aimesh->mTangents[i].y);
            vertex.Tangent.setZ(aimesh->mTangents[i].z);

            vertex.Bitangent.setX(aimesh->mBitangents[i].x);
            vertex.Bitangent.setY(aimesh->mBitangents[i].y);
            vertex.Bitangent.setZ(aimesh->mBitangents[i].z);
        }
        else
        {
            vertex.Tangent.setX(1);
            vertex.Tangent.setY(0);
            vertex.Tangent.setZ(0);

            vertex.Bitangent.setX(0);
            vertex.Bitangent.setY(1);
            vertex.Bitangent.setZ(0);
        }

        sub_mesh->vertices.push_back(vertex);
    }

    sub_mesh->num_vertices = static_cast<int>(aimesh->mNumVertices);
    sub_mesh->vertex_data.resize(sub_mesh->num_vertices * 3);
    sub_mesh->normal_data.resize(sub_mesh->num_vertices * 3);
    sub_mesh->texcoord_data.resize(sub_mesh->num_vertices * 2);
    sub_mesh->tangent_data.resize(sub_mesh->num_vertices * 3);
    sub_mesh->bitangent_data.resize(sub_mesh->num_vertices * 3);

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

        float_p = sub_mesh->tangent_data.data() + (3 * i);
        *float_p++ = sub_mesh->vertices[i].Tangent.x();
        *float_p++ = sub_mesh->vertices[i].Tangent.y();
        *float_p++ = sub_mesh->vertices[i].Tangent.z();

        float_p = sub_mesh->bitangent_data.data() + (3 * i);
        *float_p++ = sub_mesh->vertices[i].Bitangent.x();
        *float_p++ = sub_mesh->vertices[i].Bitangent.y();
        *float_p++ = sub_mesh->vertices[i].Bitangent.z();
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
    if(aimesh->mMaterialIndex > 0)
    {
        aiMaterial *material = scene->mMaterials[aimesh->mMaterialIndex];

        QVector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
        sub_mesh->textures << diffuseMaps;

        QVector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
        sub_mesh->textures << specularMaps;
    }

    qDebug() << " - Mesh Loading: " << aimesh->mName.C_Str() << " with " << sub_mesh->vertices.count()<< " vertices";

    renderer->LoadSubMesh(sub_mesh);

    return sub_mesh;
}

QVector<Texture> Mesh::loadMaterialTextures(aiMaterial *mat, aiTextureType type, QString typeName)
{
    //qDebug() << "CARGANDO MATERIAL: " << mat->GetTextureCount(type) << " - " << typeName;
    QVector<Texture> textures;
    for(unsigned int i = 0; i < mat->GetTextureCount(type); i++)
    {
        aiString str;
        mat->GetTexture(type, i, &str);
        bool skip = false;
        for(int j = 0; j < texturesLoaded.size(); j++)
        {
            //qDebug() << texturesLoaded[j].path << "COMPARE" << str.C_Str();
            if(texturesLoaded[j].path.contains(str.C_Str()))
            {
                textures.push_back(texturesLoaded[j]);
                skip = true;
                break;
            }
        }
        if(!skip)
        {   // if texture hasn't been loaded already, load it
            Texture texture;
            texture.glTexture =  new QOpenGLTexture(QImage(directory + str.C_Str()));
            qDebug() << "NEW TEXTURE";
            texture.type = typeName;
            texture.path = str.C_Str();
            textures.push_back(texture);
            texturesLoaded.push_back(texture); // add to loaded textures
        }
    }
    return textures;
}
