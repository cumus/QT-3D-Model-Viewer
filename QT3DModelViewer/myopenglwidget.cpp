#include "myopenglwidget.h"
#include "gameobject.h"
#include "scene.h"
#include "resources.h"
#include "mesh.h"
#include "transform.h"

#include <QtWidgets>
//#include <QMouseEvent>
#include <QOpenGLShaderProgram>
//#include <QCoreApplication>
//#include <QOpenGLTexture>

#define PROGRAM_VERTEX_ATTRIBUTE 0
#define PROGRAM_NORMAL_ATTRIBUTE 1

static const char *vertexShaderSource =
    "attribute vec4 vertex;\n"
    "attribute vec3 normal;\n"
    "varying vec3 vert;\n"
    "varying vec3 vertNormal;\n"
    "uniform mat4 projMatrix;\n"
    "uniform mat4 mvMatrix;\n"
    "uniform mat3 normalMatrix;\n"
    "void main() {\n"
    "   vert = vertex.xyz;\n"
    "   vertNormal = normalMatrix * normal;\n"
    "   gl_Position = projMatrix * mvMatrix * vertex;\n"
    "}\n";

static const char *fragmentShaderSource =
    "varying highp vec3 vert;\n"
    "varying highp vec3 vertNormal;\n"
    "uniform highp vec3 lightPos;\n"
    "void main() {\n"
    "   highp vec3 L = normalize(lightPos - vert);\n"
    "   highp float NL = max(dot(normalize(vertNormal), L), 0.0);\n"
    "   highp vec3 color = vec3(0.39, 1.0, 0.0);\n"
    "   highp vec3 col = clamp(color * 0.2 + color * 0.8 * NL, 0.0, 1.0);\n"
    "   gl_FragColor = vec4(col, 1.0);\n"
    "}\n";

MyOpenGLWidget::MyOpenGLWidget(QWidget *parent)
    : QOpenGLWidget(parent), tick_count(0), program_index(-1)
{
    m_camera.setToIdentity();
    m_camera.translate(0, 0, -1);
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
    tick_count++;
    update();
}

void MyOpenGLWidget::initializeGL()
{
    initializeOpenGLFunctions();

    program_index = resources->AddShader();
    resources->programs[program_index]->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderSource);
    resources->programs[program_index]->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSource);
    resources->programs[program_index]->bindAttributeLocation("vertex", PROGRAM_VERTEX_ATTRIBUTE);
    resources->programs[program_index]->bindAttributeLocation("normal", PROGRAM_NORMAL_ATTRIBUTE);
    resources->programs[program_index]->link();

    resources->programs[program_index]->bind();
    m_projMatrixLoc = resources->programs[program_index]->uniformLocation("projMatrix");
    m_mvMatrixLoc = resources->programs[program_index]->uniformLocation("mvMatrix");
    m_normalMatrixLoc = resources->programs[program_index]->uniformLocation("normalMatrix");
    m_lightPosLoc = resources->programs[program_index]->uniformLocation("lightPos");
    resources->programs[program_index]->release();

    scene->InitDemo(this);
}

void MyOpenGLWidget::paintGL()
{
    // RESET
    glClearColor(1, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST); // Enable depth buffer
    glEnable(GL_CULL_FACE); // Enable back face culling

    if (scene != nullptr)
        scene->Draw(this);
}

void MyOpenGLWidget::DrawMesh(Mesh* mesh)
{
    if (mesh == nullptr || mesh->vertexCount() <= 0)
        return;

    QMatrix4x4 m_world = mesh->gameobject->transform->GetWorldMatrix();



    QOpenGLVertexArrayObject::Binder vaoBinder(&mesh->vao);

    resources->programs[program_index]->bind();
    resources->programs[program_index]->setUniformValue(m_projMatrixLoc, m_proj);
    resources->programs[program_index]->setUniformValue(m_mvMatrixLoc, m_camera * m_world);
    resources->programs[program_index]->setUniformValue(m_normalMatrixLoc, m_world.normalMatrix());
    resources->programs[program_index]->setUniformValue(m_lightPosLoc, QVector3D((tick_count%40) - 20, 0, (tick_count%60) - 30));

    glDrawArrays(GL_TRIANGLES, 0, mesh->vertexCount());

    resources->programs[program_index]->release();
}

void MyOpenGLWidget::LoadMesh(Mesh *mesh)
{
    if (mesh == nullptr || mesh->vertexCount() <= 0)
        return;

    qDebug() << "MyOpenGLWidget::LoadMesh";

    mesh->vao.create();
    QOpenGLVertexArrayObject::Binder vaoBinder(&mesh->vao);

    // Setup our vertex buffer object.
    mesh->vbo.create();
    mesh->vbo.bind();
    mesh->vbo.allocate(mesh->constData(), mesh->count() * sizeof(GLfloat));


    // Store the vertex attribute bindings for the program.
    mesh->vbo.bind();
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    f->glEnableVertexAttribArray(PROGRAM_VERTEX_ATTRIBUTE);
    f->glEnableVertexAttribArray(PROGRAM_NORMAL_ATTRIBUTE);
    f->glVertexAttribPointer(PROGRAM_VERTEX_ATTRIBUTE, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), 0);
    f->glVertexAttribPointer(PROGRAM_NORMAL_ATTRIBUTE, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), reinterpret_cast<void *>(3 * sizeof(GLfloat)));
    mesh->vbo.release();
}

void MyOpenGLWidget::resizeGL(int width, int height)
{
    m_proj.setToIdentity();
    m_proj.perspective(45.0f, GLfloat(width) / height, 0.01f, 100.0f);
}
