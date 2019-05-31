#include "resources.h"
#include "myopenglwidget.h"

#include <QOpenGLBuffer>
#include <QOpenGLTexture>
#include <QOpenGLShaderProgram>
#include <QOpenGLFunctions>

Resources::Resources()
{
    textures.clear();
    programs.clear();
}

void Resources::Clear()
{
    textures.clear();
    programs.clear();
}

int Resources::AddTex(QString path) /*":/icons/Resources/icons/Folder.png"*/
{
    int ret = -1;

    if (path != nullptr)
    {
        textures.push_back(new QOpenGLTexture(QImage(path).mirrored()));
        ret += textures.count();
    }

    return ret;
}

int Resources::AddShader()
{
    int ret = -1;

    programs.push_back(new QOpenGLShaderProgram);
    ret += programs.count();

    return ret;
}
