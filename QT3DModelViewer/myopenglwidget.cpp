#include "myopenglwidget.h"
#include "gameobject.h"
#include "scene.h"
#include "resources.h"
#include "mesh.h"
#include "transform.h"

#include <QtWidgets>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QOpenGLShaderProgram>
#include <QTimer>
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
    setFocusPolicy(Qt::ClickFocus);
    cam.transform = new Transform(nullptr, true, {0,0,1});

    for (int i = 0; i < 6; i++) cam_dir[i] = false;


    // Tick Widget
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MyOpenGLWidget::Tick);
    timer->start(static_cast <int>(tick_period));
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
    tick_count += tick_period;

    if (cam_dir[0]) cam.transform->TranslateForward(-0.01f * tick_period);
    if (cam_dir[1]) cam.transform->TranslateForward(0.01f * tick_period);
    if (cam_dir[2]) cam.transform->TranslateLeft(-0.01f * tick_period);
    if (cam_dir[3]) cam.transform->TranslateLeft(0.01f * tick_period);
    if (cam_dir[4]) cam.transform->TranslateUp(0.01f * tick_period);
    if (cam_dir[5]) cam.transform->TranslateUp(-0.01f * tick_period);

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
    cam.m_projMatrixLoc = resources->programs[program_index]->uniformLocation("projMatrix");
    m_mvMatrixLoc = resources->programs[program_index]->uniformLocation("mvMatrix");
    m_normalMatrixLoc = resources->programs[program_index]->uniformLocation("normalMatrix");
    m_lightPosLoc = resources->programs[program_index]->uniformLocation("lightPos");
    resources->programs[program_index]->release();

    scene->InitDemo(this);
}

void MyOpenGLWidget::paintGL()
{
    // RESET
    glClearColor(0, 0, 0, 1);
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

    //mesh->gameobject->transform->RotateY(1);
    QMatrix4x4 m_world = mesh->gameobject->transform->GetWorldMatrix();

    QOpenGLVertexArrayObject::Binder vaoBinder(&mesh->vao);

    resources->programs[program_index]->bind();
    resources->programs[program_index]->setUniformValue(cam.m_projMatrixLoc, cam.m_proj);
    resources->programs[program_index]->setUniformValue(m_mvMatrixLoc, cam.transform->GetWorldMatrix().inverted() * m_world);
    resources->programs[program_index]->setUniformValue(m_normalMatrixLoc, m_world.normalMatrix());
    resources->programs[program_index]->setUniformValue(m_lightPosLoc, QVector3D((tick_count%120) - 60, 0, (tick_count%120) - 60));

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
    mesh->vbo.allocate(mesh->constData(), mesh->count() * static_cast<int>(sizeof(GLfloat)));

    // Store the vertex attribute bindings for the program.
    mesh->vbo.bind();
    QOpenGLFunctions *f = QOpenGLContext::currentContext()->functions();
    f->glEnableVertexAttribArray(PROGRAM_VERTEX_ATTRIBUTE);
    f->glVertexAttribPointer(PROGRAM_VERTEX_ATTRIBUTE, 3, GL_FLOAT, GL_FALSE, 6 * static_cast<int>(sizeof(GLfloat)), nullptr);
    f->glEnableVertexAttribArray(PROGRAM_NORMAL_ATTRIBUTE);
    f->glVertexAttribPointer(PROGRAM_NORMAL_ATTRIBUTE, 3, GL_FLOAT, GL_FALSE, 6 * static_cast<int>(sizeof(GLfloat)), reinterpret_cast<void *>(3 * sizeof(GLfloat)));
    mesh->vbo.release();


    /*glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int),
                 &indices[0], GL_STATIC_DRAW);

    // vertex positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    // vertex normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
    // vertex texture coords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));

    glBindVertexArray(0);*/
}

void MyOpenGLWidget::resizeGL(int width, int height)
{
    cam.m_proj.setToIdentity();
    cam.m_proj.perspective(45.0f, GLfloat(width) / height, 0.01f, 100.0f);
}

void MyOpenGLWidget::mousePressEvent(QMouseEvent *event)
{
    mouse_pos = event->pos();
}

void MyOpenGLWidget::mouseMoveEvent(QMouseEvent *event)
{
    cam.transform->RotateAxisUp(static_cast<float>((mouse_pos - event->localPos()).x()));
    cam.transform->RotateAxisLeft(static_cast<float>((mouse_pos - event->localPos()).y()));
    mouse_pos = event->localPos();

    // correct roll
    cam.transform->RotateAxisForward(-cam.transform->GetRot().z());
}

void MyOpenGLWidget::keyPressEvent(QKeyEvent *event)
{
    switch (event->key())
    {
    case Qt::Key_W: cam_dir[0] = true; break;
    case Qt::Key_S: cam_dir[1] = true; break;
    case Qt::Key_A: cam_dir[2] = true; break;
    case Qt::Key_D: cam_dir[3] = true; break;
    case Qt::Key_Space: cam_dir[4] = true; break;
    case Qt::Key_E: cam_dir[5] = true; break;
    default: break;
    }
}

void MyOpenGLWidget::keyReleaseEvent(QKeyEvent *event)
{
    switch (event->key())
    {
    case Qt::Key_W: cam_dir[0] = false; break;
    case Qt::Key_S: cam_dir[1] = false; break;
    case Qt::Key_A: cam_dir[2] = false; break;
    case Qt::Key_D: cam_dir[3] = false; break;
    case Qt::Key_Space: cam_dir[4] = false; break;
    case Qt::Key_E: cam_dir[5] = false; break;
    default: break;
    }
}
