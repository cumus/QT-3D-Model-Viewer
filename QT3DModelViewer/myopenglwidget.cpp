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
#include <QOpenGLTexture>

static const char *vertexShaderSource =
    "attribute vec4 vertex;\n"
    "attribute vec3 normal;\n"
    "attribute mediump vec4 texCoord;\n"
    "varying vec3 vert;\n"
    "varying vec3 vertNormal;\n"
    "varying mediump vec4 texc;\n"
    "uniform mat4 projMatrix;\n"
    "uniform mat4 mvMatrix;\n"
    "uniform mat3 normalMatrix;\n"
    "void main() {\n"
    "   texc = texCoord;\n"
    "   vert = vertex.xyz;\n"
    "   vertNormal = normalMatrix * normal;\n"
    "   gl_Position = projMatrix * mvMatrix * vertex;\n"
    "}\n";

static const char *fragmentShaderSource =
    "varying highp vec3 vert;\n"
    "varying highp vec3 vertNormal;\n"
    "varying mediump vec4 texc;\n"
    "uniform highp vec3 lightPos;\n"
    "uniform highp vec3 light_intensity;\n"
    "uniform sampler2D texture;\n"
    "void main() {\n"
    "   highp vec3 L = normalize(lightPos - vert);\n"
    "   highp float NL = max(dot(normalize(vertNormal), L), 0.0);\n"
    "   highp vec3 color = vec3(0.39, 1.0, 0.0);\n"
    "   highp vec3 col = clamp(color * 0.2 + color * 0.8 * NL, 0.0, 1.0);\n"
    //"   gl_FragColor = vec4(col, 1.0);\n"
    "   gl_FragColor = texture2D(texture, texc.st);\n"
    "}\n";

/*/000000000000000000000000000000000000000

"attribute mediump vec4 texCoord;\n"
"varying mediump vec4 texc;\n"

"    texc = texCoord;\n"

//000000000000000000000000000000000000000

"uniform sampler2D texture;\n"
"varying mediump vec4 texc;\n"

"   gl_FragColor = texture2D(texture, texc.st);\n"


    "   gl_FragColor = vec4(light_intensity, 1.0) * vec4(texture2D(texture, texc.st));\n"
*/

MyOpenGLWidget::MyOpenGLWidget(QWidget *parent)
    : QOpenGLWidget(parent), tick_count(0), program_index(-1)
{
    setFocusPolicy(Qt::ClickFocus);
    cam.transform = new Transform(nullptr, true, {0,0,2});

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
    resources->programs[program_index]->bindAttributeLocation("vertex", 0);
    resources->programs[program_index]->bindAttributeLocation("normal", 1);
    resources->programs[program_index]->bindAttributeLocation("texCoord", 2);
    resources->programs[program_index]->link();

    resources->programs[program_index]->bind();

    cam.m_projMatrixLoc =   resources->programs[program_index]->uniformLocation("projMatrix");
    m_mvMatrixLoc =         resources->programs[program_index]->uniformLocation("mvMatrix");
    m_normalMatrixLoc =     resources->programs[program_index]->uniformLocation("normalMatrix");
    m_lightPosLoc =         resources->programs[program_index]->uniformLocation("lightPos");
    m_lightIntensityLoc =   resources->programs[program_index]->uniformLocation("light_intensity");
    m_textureLoc =          resources->programs[program_index]->uniformLocation("texture");

    resources->programs[program_index]->release();

    scene->InitDemo(this);
}

void MyOpenGLWidget::paintGL()
{
    // RESET
    glClearColor(0.1f, 0.1f, 0.1f, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST); // Enable depth buffer
    glEnable(GL_CULL_FACE); // Enable back face culling

    if (scene != nullptr)
        scene->Draw(this);
}

void MyOpenGLWidget::DrawMesh(Mesh* mesh)
{
    if (mesh == nullptr)// || !mesh->vao.isCreated())
        return;

    QMatrix4x4 m_world = mesh->gameobject->transform->GetWorldMatrix();
    m_world.rotate(180.f, {0,0,1});

    resources->programs[program_index]->bind();
    resources->programs[program_index]->setUniformValue(cam.m_projMatrixLoc, cam.m_proj);
    resources->programs[program_index]->setUniformValue(m_mvMatrixLoc, cam.transform->GetWorldMatrix().inverted() * m_world.inverted());
    resources->programs[program_index]->setUniformValue(m_normalMatrixLoc, m_world.normalMatrix());
    resources->programs[program_index]->setUniformValue(m_lightPosLoc, QVector3D(0, 1, 0));
    resources->programs[program_index]->setUniformValue(m_lightIntensityLoc, QVector3D(1, 1, 1));

    for(int i = 0; i < mesh->sub_meshes.size(); i++)
    {
        SubMesh* sub = mesh->sub_meshes[i];

        if(sub->vao.isCreated())
        {
            QOpenGLVertexArrayObject::Binder vaoBinder(&sub->vao);

            if(sub->texture != nullptr && sub->texture->isCreated())
                sub->texture->bind();

            if(sub->num_faces > 0)
                glDrawElements(GL_TRIANGLES, sub->num_faces * 3, GL_UNSIGNED_INT, nullptr);
            else
                glDrawArrays(GL_TRIANGLES, 0, sub->num_vertices);
        }
    }

    resources->programs[program_index]->release();
}

void MyOpenGLWidget::LoadMesh(SubMesh *mesh)
{
    if (mesh == nullptr || mesh->num_vertices <= 0 || mesh->num_faces <= 0)
        return;

    mesh->vao.create();
    QOpenGLVertexArrayObject::Binder vaoBinder(&mesh->vao);

    // Setup our POSITIONS
    mesh->vbo.create();
    mesh->vbo.setUsagePattern( QOpenGLBuffer::StaticDraw );
    mesh->vbo.bind();
    mesh->vbo.allocate(mesh->vertex_data.constData(), 3 * mesh->num_vertices * static_cast<int>(sizeof(GLfloat)));

    resources->programs[program_index]->enableAttributeArray(0);
    resources->programs[program_index]->setAttributeBuffer(0, GL_FLOAT, 0, 3);

    // Setup our NORMALS
    mesh->nbo.create();
    mesh->nbo.setUsagePattern( QOpenGLBuffer::StaticDraw );
    mesh->nbo.bind();
    mesh->nbo.allocate(mesh->normal_data.constData(), 3 * mesh->num_vertices * static_cast<int>(sizeof(GLfloat)));

    resources->programs[program_index]->enableAttributeArray(1);
    resources->programs[program_index]->setAttributeBuffer(1, GL_FLOAT, 0, 3);

    // Setup TEXTURE COORDS
    mesh->tbo.create();
    mesh->tbo.setUsagePattern( QOpenGLBuffer::StaticDraw );
    mesh->tbo.bind();
    mesh->tbo.allocate(mesh->texcoord_data.constData(), 2 * mesh->num_vertices * static_cast<int>(sizeof(GLfloat)));

    resources->programs[program_index]->enableAttributeArray(2);
    resources->programs[program_index]->setAttributeBuffer(2, GL_FLOAT, 0, 2);

    // Setup INDEXES
    mesh->ibo.create();
    mesh->ibo.setUsagePattern( QOpenGLBuffer::StaticDraw );
    mesh->ibo.bind();
    mesh->ibo.allocate(mesh->index_data.constData(), 3 * mesh->num_faces * static_cast<int>(sizeof(GLint)));

    mesh->f = QOpenGLContext::currentContext()->functions();
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
