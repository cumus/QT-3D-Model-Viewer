#include "myopenglwidget.h"
#include "gameobject.h"
#include "scene.h"
#include "resources.h"
#include "mesh.h"
#include "transform.h"

#include <QtWidgets>
#include <QMouseEvent>
#include <QOpenGLShaderProgram>
#include <QCoreApplication>
#include <QOpenGLTexture>
#include <math.h>

MyOpenGLWidget::MyOpenGLWidget(QWidget *parent)
    : QOpenGLWidget(parent)
{
    // QSurfaceFormat::CoreProfile -> Functionality deprecated in OpenGL version 3.0 is not available.
    m_core = QSurfaceFormat::defaultFormat().profile() == QSurfaceFormat::CoreProfile;
    program_index = -1;
}

MyOpenGLWidget::~MyOpenGLWidget()
{
    makeCurrent();

    if (resources != nullptr)
        resources->Clear();

    if (scene != nullptr)
        scene->Clear();

    doneCurrent();
}

QSize MyOpenGLWidget::minimumSizeHint() const
{
    return QSize(100, 100);
}

QSize MyOpenGLWidget::sizeHint() const
{
    return QSize(563, 453);
}

void MyOpenGLWidget::Tick()
{
    /*if (xRot++ >= 360) xRot = 0;
    if (yRot++ >= 360) yRot = 0;
    if (zRot++ >= 360) zRot = 0;*/

    update();
}

void MyOpenGLWidget::initializeGL()
{
    initializeOpenGLFunctions();

    glEnable(GL_DEPTH_TEST); // Enable depth buffer
    glEnable(GL_CULL_FACE); // Enable back face culling

    // Load Shaders
    #define PROGRAM_VERTEX_ATTRIBUTE 0
    #define PROGRAM_TEXCOORD_ATTRIBUTE 1

    QOpenGLShader *vshader = new QOpenGLShader(QOpenGLShader::Vertex, this);
    const char *vsrc =
        "attribute highp vec4 vertex;\n"
        "attribute mediump vec4 texCoord;\n"
        "varying mediump vec4 texc;\n"
        "uniform mediump mat4 matrix;\n"
        "void main(void)\n"
        "{\n"
        "    gl_Position = matrix * vertex;\n"
        "    texc = texCoord;\n"
        "}\n";
    vshader->compileSourceCode(vsrc);

    QOpenGLShader *fshader = new QOpenGLShader(QOpenGLShader::Fragment, this);
    const char *fsrc =
        "uniform sampler2D texture;\n"
        "varying mediump vec4 texc;\n"
        "void main(void)\n"
        "{\n"
        "    gl_FragColor = texture2D(texture, texc.st);\n"
        "}\n";
    fshader->compileSourceCode(fsrc);

    program_index = resources->AddShader();
    if(program_index >= 0)
    {
        resources->programs[program_index]->addShader(vshader);
        resources->programs[program_index]->addShader(fshader);
        resources->programs[program_index]->bindAttributeLocation("vertex", PROGRAM_VERTEX_ATTRIBUTE);
        resources->programs[program_index]->bindAttributeLocation("texCoord", PROGRAM_TEXCOORD_ATTRIBUTE);
        resources->programs[program_index]->link();
        resources->programs[program_index]->bind();
        resources->programs[program_index]->setUniformValue("texture", 0);
    }
}

void MyOpenGLWidget::paintGL()
{
    // RESET
    glClearColor(1, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    /*if (scene != nullptr && scene->root != nullptr)
        scene->root->Draw(this);*/
}

void MyOpenGLWidget::DrawMesh(Mesh* mesh)
{
    if (mesh == nullptr
            || mesh->vbo_index < 0
            || mesh->texture_index < 0
            || program_index < 0)
        return;

    QMatrix4x4 m;

    //CAMERA
    m.ortho(-0.5f, +0.5f, +0.5f, -0.5f, 4.0f, 15.0f);
    //m.perspective(fov, aspect_ratio, near_plane, far_plane);

    //m *= mesh->gameobject->transform->GetWorldMatrix();

    m.translate(mesh->gameobject->transform->local_pos);
    m.rotate(mesh->gameobject->transform->local_rot.x(), 1.0f, 0.0f, 0.0f);
    m.rotate(mesh->gameobject->transform->local_rot.y(), 0.0f, 1.0f, 0.0f);
    m.rotate(mesh->gameobject->transform->local_rot.z(), 0.0f, 0.0f, 1.0f);
    //m.scale(mesh->gameobject->transform->local_scale);

    // SHADERS
    resources->programs[program_index]->setUniformValue("matrix", m);
    resources->programs[program_index]->enableAttributeArray(PROGRAM_VERTEX_ATTRIBUTE);
    resources->programs[program_index]->enableAttributeArray(PROGRAM_TEXCOORD_ATTRIBUTE);
    resources->programs[program_index]->setAttributeBuffer(PROGRAM_VERTEX_ATTRIBUTE, GL_FLOAT, 0, 3, 5 * sizeof(GLfloat));
    resources->programs[program_index]->setAttributeBuffer(PROGRAM_TEXCOORD_ATTRIBUTE, GL_FLOAT, 3 * sizeof(GLfloat), 2, 5 * sizeof(GLfloat));

    // TEXTURE
    resources->textures[mesh->texture_index]->bind();

    // DRAW
    for (int i = 0; i < 6; ++i)
        glDrawArrays(GL_TRIANGLE_FAN, i * 4, 4);
}

void MyOpenGLWidget::resizeGL(int width, int height)
{
    m_proj.setToIdentity();
    m_proj.perspective(45.0f, GLfloat(width) / height, 0.01f, 100.0f);
}



