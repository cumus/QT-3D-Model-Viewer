#ifndef RESOURCES_H
#define RESOURCES_H

#include <QVector>
#include <QOpenGLBuffer>
//#include <QOpenGLVertexArrayObject>

QT_FORWARD_DECLARE_CLASS(QOpenGLBuffer)
QT_FORWARD_DECLARE_CLASS(QOpenGLShaderProgram)

class QOpenGLTexture;

class Resources
{
public:
    Resources();

    void Clear();

    int AddVBO(QVector<GLfloat>* vertData);
    int AddTex(QString path = nullptr);
    int AddShader();

public:
    QVector<QOpenGLBuffer> vbos;
    //QVector<QOpenGLVertexArrayObject*> vaos;
    QVector<QOpenGLTexture*> textures;
    QVector<QOpenGLShaderProgram*> programs;
};

#endif // RESOURCES_H
