#include "resources.h"

#include <QOpenGLBuffer>
#include <QOpenGLTexture>
#include <QOpenGLShaderProgram>

Resources::Resources()
{
    vbos.clear();
    //vaos.clear();
    textures.clear();
    programs.clear();
}

void Resources::Clear()
{
    foreach(QOpenGLBuffer vbo, vbos)
        vbo.destroy();

    vbos.clear();
    textures.clear();
    programs.clear();
}

int Resources::AddVBO(QVector<GLfloat>* vertData)
{
    int ret = -1;

    if (vertData != nullptr)
    {
        int count = vertData->count();
        if (count > 0)
        {
            QOpenGLBuffer* vbo = new QOpenGLBuffer();
            vbo->create();
            vbo->bind();
            vbo->allocate(vertData->constData(), count * sizeof(GLfloat));

            vbos.push_back(*vbo);
            ret = vbos.count() - 1;
        }
    }

    return ret;
}

int Resources::AddTex(QString path) /*":/icons/Resources/icons/Folder.png"*/
{
    int ret = -1;

    if (path != nullptr)
    {
        textures.push_back(new QOpenGLTexture(QImage(path).mirrored()));
        ret = textures.count() - 1;
    }

    return ret;
}

int Resources::AddShader()
{
    int ret = -1;

    programs.push_back(new QOpenGLShaderProgram);
    ret = programs.count() - 1;

    return ret;
}

