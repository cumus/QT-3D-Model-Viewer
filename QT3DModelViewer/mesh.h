#ifndef MESH_H
#define MESH_H

#include "component.h"
#include <qopengl.h>
#include <QVector>
#include <QVector3D>
#include <QVector2D>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>

QT_FORWARD_DECLARE_CLASS(QOpenGLBuffer)

struct Vertex {
    QVector3D Position;
    QVector3D Normal;
    QVector2D TexCoords;
};

struct Texture {
    unsigned int id;
    QString type;
};

class MyOpenGLWidget;
//class Resources;

class Mesh : public Component
{
public:
    Mesh(GameObject* go = nullptr, bool isActive = true);
    ~Mesh() override;

    void Draw(MyOpenGLWidget* renderer) override;

    void Save(QDataStream& stream) override;
    void Load(QDataStream& stream) override;
    void CleanUp() override;

public:
    QVector<Vertex> vertices;
    QVector<unsigned int> indices;
    QVector<Texture> textures;

    QOpenGLVertexArrayObject vao;
    QOpenGLBuffer vbo;
    QOpenGLBuffer nbo;
    QOpenGLBuffer tbo;
    QOpenGLBuffer ibo;

    QVector<GLfloat> vertex_data;
    QVector<GLfloat> normal_data;
    QVector<GLint> texcoord_data;
    QVector<GLint> index_data;

    int num_vertices;
    int num_faces;
};

#endif // MODEL_H
