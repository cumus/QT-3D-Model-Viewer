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

int Resources::AddVBO()
{
    int ret = -1;

    static const int coords[6][4][3] = {
            { { +1, -1, -1 }, { -1, -1, -1 }, { -1, +1, -1 }, { +1, +1, -1 } },
            { { +1, +1, -1 }, { -1, +1, -1 }, { -1, +1, +1 }, { +1, +1, +1 } },
            { { +1, -1, +1 }, { +1, -1, -1 }, { +1, +1, -1 }, { +1, +1, +1 } },
            { { -1, -1, -1 }, { -1, -1, +1 }, { -1, +1, +1 }, { -1, +1, -1 } },
            { { +1, -1, +1 }, { -1, -1, +1 }, { -1, -1, -1 }, { +1, -1, -1 } },
            { { -1, -1, +1 }, { +1, -1, +1 }, { +1, +1, +1 }, { -1, +1, +1 } }
    };

    float scale = 0.2f;

    QVector<GLfloat> vertData;
    for (int i = 0; i < 6; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            // vertex position
            vertData.append(scale * coords[i][j][0]);
            vertData.append(scale * coords[i][j][1]);
            vertData.append(scale * coords[i][j][2]);

            // texture coordinate
            vertData.append(j == 0 || j == 3);
            vertData.append(j == 0 || j == 1);
        }
    }

    QOpenGLBuffer* vbo = new QOpenGLBuffer();
    vbo->create();
    vbo->bind();
    vbo->allocate(vertData.constData(), vertData.count() * sizeof(GLfloat));

    vbos.push_back(*vbo);
    ret = vbos.count() - 1;

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

