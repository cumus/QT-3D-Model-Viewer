#ifndef RESOURCES_H
#define RESOURCES_H

#include <QVector>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>

QT_FORWARD_DECLARE_CLASS(QOpenGLBuffer)
QT_FORWARD_DECLARE_CLASS(QOpenGLShaderProgram)

class MyOpenGLWidget;
class QOpenGLTexture;

class Resources
{
public:
    Resources();

    void Clear();
    int AddTex(QString path = nullptr);
    int AddShader();

public:

    MyOpenGLWidget* renderer = nullptr;

    QVector<QOpenGLTexture*> textures;
    QVector<QOpenGLShaderProgram*> programs;
};

#endif // RESOURCES_H
