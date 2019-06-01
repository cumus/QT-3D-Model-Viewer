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

    void LoadFromFile(QString filename, MyOpenGLWidget* renderer = nullptr);

    void Save(QDataStream& stream) override;
    void Load(QDataStream& stream) override;
    void CleanUp() override;

    const GLfloat *constData() const { return m_data.constData(); }
    int count() const { return m_count; }
    int vertexCount() const { return m_count / 6; }

private:
    void quad(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2, GLfloat x3, GLfloat y3, GLfloat x4, GLfloat y4);
    void extrude(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2);

    public:
    void add(const QVector3D &v, const QVector3D &n);

    QVector<Vertex> vertices;
    QVector<unsigned int> indices;
    QVector<Texture> textures;

    QOpenGLVertexArrayObject vao;
    QOpenGLBuffer vbo;

private:
    QVector<GLfloat> m_data;
    int m_count;

};

#endif // MODEL_H
