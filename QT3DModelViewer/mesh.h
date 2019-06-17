#ifndef MESH_H
#define MESH_H

#include "component.h"
#include <qopengl.h>
#include <QVector>
#include <QVector3D>
#include <QVector2D>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QOpenGLFunctions>
#include <QOpenGLTexture>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

struct Vertex {
    QVector3D Position;
    QVector3D Normal;
    QVector2D TexCoords;
    QVector3D Tangent;
    QVector3D Bitangent;
};

struct Texture {
    QOpenGLTexture* glTexture;
    QString type;
    QString path;
};

class SubMesh
{
public:
    SubMesh():
    vbo(QOpenGLBuffer::VertexBuffer),
    nbo(QOpenGLBuffer::VertexBuffer),
    tbo(QOpenGLBuffer::VertexBuffer),
    tnbo(QOpenGLBuffer::VertexBuffer),
    btnbo(QOpenGLBuffer::VertexBuffer),
    ibo(QOpenGLBuffer::IndexBuffer){}

    int num_vertices = 0;
    int num_faces = 0;
    QVector<Vertex> vertices;
    QVector<unsigned int> indices;
    QVector<Texture> textures;

    QVector<GLfloat> vertex_data;
    QVector<GLfloat> normal_data;
    QVector<GLfloat> texcoord_data;
    QVector<GLfloat> tangent_data;
    QVector<GLfloat> bitangent_data;
    QVector<GLuint> index_data;

    QOpenGLVertexArrayObject vao;
    QOpenGLBuffer vbo;
    QOpenGLBuffer nbo;
    QOpenGLBuffer tbo;
    QOpenGLBuffer tnbo;
    QOpenGLBuffer btnbo;
    QOpenGLBuffer ibo;
};

class MyOpenGLWidget;
class QOpenGLShaderProgram;

class Mesh : public Component
{
public:
    Mesh(GameObject* go = nullptr, bool isActive = true);
    ~Mesh() override;

    void Draw(MyOpenGLWidget* renderer) override;

    void Save(QDataStream& stream) override;
    void Load(QDataStream& stream) override;
    void CleanUp() override;

    void importModel(QString path, MyOpenGLWidget* renderer = nullptr);
    void processNode(aiNode *node, const aiScene *scene, MyOpenGLWidget* renderer = nullptr);
    SubMesh* processMesh(aiMesh *aimesh, const aiScene *scene, MyOpenGLWidget* renderer = nullptr);
    QVector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, QString typeName);

public:

    QVector<SubMesh*> sub_meshes;
    QVector<Texture> texturesLoaded;

    bool draw_border = false;
    float refraction_index = 1.52f;

private:

    QString directory;
};

#endif // MODEL_H
