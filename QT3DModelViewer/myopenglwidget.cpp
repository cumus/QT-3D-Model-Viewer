#include "myopenglwidget.h"
#include "gameobject.h"
#include "scene.h"
#include "mesh.h"
#include "transform.h"

#include <QtWidgets>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QOpenGLShaderProgram>
#include <QTimer>
#include <QOpenGLTexture>
#include <QOpenGLFunctions>
#include <QOpenGLFramebufferObject>
#include <iostream>

MyOpenGLWidget::MyOpenGLWidget(QWidget *parent)
    : QOpenGLWidget(parent), tick_count(0)
{
    setFocusPolicy(Qt::ClickFocus);

    camera = new Transform(nullptr, true, {0,0,5});
    for (int i = 0; i < 6; i++) cam_dir[i] = false;
    cam_focus = {0,0,0};

    border_color = QVector3D(1,0.27f,0);
    border_meshes.clear();

    QSize size = sizeHint();
    width = size.width();
    height = size.height();

    state = WIDGET_CREATED;
}

MyOpenGLWidget::~MyOpenGLWidget()
{
    makeCurrent();

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

    if (cam_dir[0]) camera->TranslateForward(0.01f * tick_period);
    if (cam_dir[1]) camera->TranslateForward(-0.01f * tick_period);
    if (cam_dir[2]) camera->TranslateLeft(0.01f * tick_period);
    if (cam_dir[3]) camera->TranslateLeft(-0.01f * tick_period);
    if (cam_dir[4]) camera->TranslateUp(0.01f * tick_period);
    if (cam_dir[5]) camera->TranslateUp(-0.01f * tick_period);
    if (camera_light_follow) lights[0].Position = camera->GetPos();

    update();
}

void MyOpenGLWidget::initializeGL()
{
    state = INITIALIZING;

    initializeOpenGLFunctions();

    // Shaders
    LoadShaders();

    // Framebuffer
    LoadFramebuffer();

    // Lights
    ResetLights();
    lights[0].isActive = true;
    lights[0].Intensity = 3;

    // Enable depth buffer
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // Enable back face culling
    glEnable(GL_CULL_FACE);

    // Enable transparency blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);

    // Enable stencil
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

    // Try Loading Some Meshes
    scene->InitDemo(this);

    // Tick Widget
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MyOpenGLWidget::Tick);
    timer->start(static_cast <int>(tick_period));

    state = INITIALIZED;
}

void MyOpenGLWidget::paintGL()
{
    state = PREPARING_TO_DRAW;

    // Choose Target Frambuffer
    if (use_deferred)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glDisable(GL_BLEND); // cannot blend with our current deferred lightning
    }
    else
    {
        QOpenGLFramebufferObject::bindDefault();
        glEnable(GL_BLEND);
    }

    // Reset Buffer
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT| GL_STENCIL_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glStencilMask(0x00);
    border_meshes.clear();

    state = MODELS;

    if (scene != nullptr)
    {
        scene->Draw(this);

        if(renderSkybox)
        {
            RenderSkybox();
        }

        DrawBordered();

        PostProcessDeferredLights();
    }

    state = FINISHED;
}

void MyOpenGLWidget::DrawMesh(Mesh* mesh, SHADER_TYPE shader)
{
    if (mesh == nullptr || programs.isEmpty())
    {
        return;
    }
    else if (use_deferred)
    {
        shader = GRAPHIC_BUFFER;
    }
    else if (draw_borders && mesh->draw_border && state == MODELS)
    {
        border_meshes.push_back(mesh);
        return;
    }

    QMatrix4x4 m_world = mesh->gameobject->transform->GetWorldMatrix();

    QOpenGLShaderProgram* program = programs[static_cast<int>(shader)];
    program->bind();

    switch (shader)
    {
    case DEFAULT:
    {
        program->setUniformValue(d_projMatrixLoc, m_proj);
        program->setUniformValue(d_mvMatrixLoc, camera->GetWorldMatrix().inverted() * m_world);
        program->setUniformValue(d_normalMatrixLoc, m_world.normalMatrix());
        program->setUniformValue("modelMatrix", m_world);
        program->setUniformValue("cameraPos", camera->GetPos());
        program->setUniformValue("refraction_index", mesh->refraction_index);
        program->setUniformValue(d_modeLoc, mode);

        for(int i = 0; i < mesh->sub_meshes.size(); i++)
        {
            SubMesh* sub = mesh->sub_meshes[i];

            if(sub->vao.isCreated())
            {
                if (skyboxes[current_skybox] != nullptr)
                    skyboxes[current_skybox]->bind();

                unsigned int diffuseNr  = 1;
                unsigned int specularNr = 1;

                for(int i = 0; i < sub->textures.size(); i++)
                {
                    glActiveTexture(GL_TEXTURE0 + static_cast<unsigned int>(i));
                    QString number;
                    QString name = sub->textures[i].type;
                    if(name == "texture_diffuse") number = QString(diffuseNr++);
                    else if(name == "texture_specular") number = QString(specularNr++);

                    program->setUniformValue((name + number).toStdString().c_str(), static_cast<unsigned int>(i));
                    sub->textures[i].glTexture->bind();
                }

                QOpenGLVertexArrayObject::Binder vaoBinder(&sub->vao);
                if(sub->num_faces > 0) glDrawElements(GL_TRIANGLES, sub->num_faces * 3, GL_UNSIGNED_INT, nullptr);
                else glDrawArrays(GL_TRIANGLES, 0, sub->num_vertices);

                for (int i = 0; i < sub->textures.count(); i++)
                    if(sub->textures[i].glTexture != nullptr && sub->textures[i].glTexture->isCreated())
                        sub->textures[i].glTexture->release();
            }
        }
        break;
    }
    case SINGLE_COLOR:
    {
        m_world.scale(border_scale);
        program->setUniformValue(sc_proj, m_proj);
        program->setUniformValue(sc_modelView, camera->GetWorldMatrix().inverted() * m_world);
        program->setUniformValue(sc_color, border_color);
        program->setUniformValue(sc_alpha, border_alpha);

        for(int i = 0; i < mesh->sub_meshes.size(); i++)
        {
            SubMesh* sub = mesh->sub_meshes[i];
            if(sub->vao.isCreated())
            {
                QOpenGLVertexArrayObject::Binder vaoBinder(&sub->vao);
                if(sub->num_faces > 0) glDrawElements(GL_TRIANGLES, sub->num_faces * 3, GL_UNSIGNED_INT, nullptr);
                else glDrawArrays(GL_TRIANGLES, 0, sub->num_vertices);
            }
        }
        break;
    }
    case GRAPHIC_BUFFER:
    {
        if (state == BORDERS)
            m_world.scale(border_scale);

        program->setUniformValue("use_flat_color", state == BORDERS);
        program->setUniformValue("projection", m_proj);
        program->setUniformValue("view", camera->GetWorldMatrix().inverted());
        program->setUniformValue("model", m_world);
        program->setUniformValue("modelInv", m_world.inverted());

        for(int i = 0; i < mesh->sub_meshes.size(); i++)
        {
            SubMesh* sub = mesh->sub_meshes[i];
            if(sub->vao.isCreated())
            {
                // bind appropriate textures
                unsigned int diffuseNr  = 1;
                unsigned int specularNr = 1;

                for(int i = 0; i < sub->textures.size(); i++)
                {
                    QString number;
                    QString name = sub->textures[i].type;
                    if(name == "texture_diffuse") number = QString(diffuseNr++);
                    else if(name == "texture_specular") number = QString(specularNr++);

                    glActiveTexture(GL_TEXTURE0 + static_cast<unsigned int>(i));
                    program->setUniformValue((name + number).toStdString().c_str(), static_cast<unsigned int>(i));
                    sub->textures[i].glTexture->bind();
                }

                QOpenGLVertexArrayObject::Binder vaoBinder(&sub->vao);
                if(sub->num_faces > 0) glDrawElements(GL_TRIANGLES, sub->num_faces * 3, GL_UNSIGNED_INT, nullptr);
                else glDrawArrays(GL_TRIANGLES, 0, sub->num_vertices);

                for (int i = 0; i < sub->textures.count(); i++)
                    if(sub->textures[i].glTexture != nullptr && sub->textures[i].glTexture->isCreated())
                        sub->textures[i].glTexture->release();
            }
        }
        break;
    }
    default: break;
    }

    program->release();
}

void MyOpenGLWidget::LoadSubMesh(SubMesh *mesh)
{
    if (mesh == nullptr || mesh->num_vertices <= 0)
        return;

    programs[DEFAULT]->bind();
    mesh->vao.create();
    QOpenGLVertexArrayObject::Binder vaoBinder(&mesh->vao);

    // Setup our POSITIONS
    mesh->vbo.create();
    mesh->vbo.setUsagePattern( QOpenGLBuffer::StaticDraw );
    mesh->vbo.bind();
    mesh->vbo.allocate(mesh->vertex_data.constData(), 3 * mesh->num_vertices * static_cast<int>(sizeof(GLfloat)));

    programs[DEFAULT]->enableAttributeArray(0);
    programs[DEFAULT]->setAttributeBuffer(0, GL_FLOAT, 0, 3);

    // Setup our NORMALS
    mesh->nbo.create();
    mesh->nbo.setUsagePattern( QOpenGLBuffer::StaticDraw );
    mesh->nbo.bind();
    mesh->nbo.allocate(mesh->normal_data.constData(), 3 * mesh->num_vertices * static_cast<int>(sizeof(GLfloat)));

    programs[DEFAULT]->enableAttributeArray(1);
    programs[DEFAULT]->setAttributeBuffer(1, GL_FLOAT, 0, 3);

    // Setup TEXTURE COORDS
    mesh->tbo.create();
    mesh->tbo.setUsagePattern( QOpenGLBuffer::StaticDraw );
    mesh->tbo.bind();
    mesh->tbo.allocate(mesh->texcoord_data.constData(), 2 * mesh->num_vertices * static_cast<int>(sizeof(GLfloat)));

    programs[DEFAULT]->enableAttributeArray(2);
    programs[DEFAULT]->setAttributeBuffer(2, GL_FLOAT, 0, 2);

    // Setup our TANGENTS & BITANGENTS
    mesh->tnbo.create();
    mesh->tnbo.setUsagePattern( QOpenGLBuffer::StaticDraw );
    mesh->tnbo.bind();
    mesh->tnbo.allocate(mesh->tangent_data.constData(), 3 * mesh->num_vertices * static_cast<int>(sizeof(GLfloat)));

    programs[DEFAULT]->enableAttributeArray(3);
    programs[DEFAULT]->setAttributeBuffer(3, GL_FLOAT, 0, 3);

    mesh->btnbo.create();
    mesh->btnbo.setUsagePattern( QOpenGLBuffer::StaticDraw );
    mesh->btnbo.bind();
    mesh->btnbo.allocate(mesh->bitangent_data.constData(), 3 * mesh->num_vertices * static_cast<int>(sizeof(GLfloat)));

    programs[DEFAULT]->enableAttributeArray(4);
    programs[DEFAULT]->setAttributeBuffer(4, GL_FLOAT, 0, 3);

    // Setup INDEXES
    if (mesh->num_faces > 0)
    {
        mesh->ibo.create();
        mesh->ibo.setUsagePattern( QOpenGLBuffer::StaticDraw );
        mesh->ibo.bind();
        mesh->ibo.allocate(mesh->index_data.constData(), 3 * mesh->num_faces * static_cast<int>(sizeof(GLint)));
    }

    programs[DEFAULT]->release();
}

void MyOpenGLWidget::ResetLights()
{
    lights.clear();

    for (unsigned int i = 0; i < NR_LIGHTS; i++)
    {
        Light light;
        light.isActive = false;
        light.Position = {0,0,0};
        light.Color = {1,1,1};
        light.Intensity = 1.0f;
        light.MinRange = 1.0f;
        light.Constant = 1.0f;
        light.Linear = 0.7f;
        light.Quadratic = 1.8f;
        light.updated = false;
        lights.push_back(light);
    }
}

void MyOpenGLWidget::RandomizeLights(float range, QVector3D pos_range, QVector3D offset, QVector3D min_color)
{
    srand(13);
    lights.clear();

    for (unsigned int i = 0; i < NR_LIGHTS; i++)
    {
        Light light;
        light.isActive = false;
        light.Position =
        {((rand() % 100) / 100.0f) * pos_range.x() + offset.x(),
         ((rand() % 100) / 100.0f) * pos_range.y() + offset.y(),
         ((rand() % 100) / 100.0f) * pos_range.z() + offset.z()};
        light.Color =
        {((rand() % 100) / 100.0f) * (1.0f - min_color.x())+ min_color.x(),
         ((rand() % 100) / 100.0f) * (1.0f - min_color.y())+ min_color.y(),
         ((rand() % 100) / 100.0f) * (1.0f - min_color.z())+ min_color.z()};
        light.MinRange = ((rand() % 100) / 100.0f) * range;
        light.Intensity = 1.0f;
        light.Constant = 1.0f;
        light.Linear = 0.7f;
        light.Quadratic = 1.8f;
        light.updated = false;
        lights.push_back(light);
    }
}

void MyOpenGLWidget::resizeGL(int width, int height)
{
    m_proj.setToIdentity();
    m_proj.perspective(zoom, GLfloat(width) / height, 0.1f, 100.0f);

    this->width = width;
    this->height = height;

    //DeleteFrameBuffers();
    //CreateFrameBuffers(); with new target size
}

void MyOpenGLWidget::mousePressEvent(QMouseEvent *event)
{
    mouse_pos = event->pos();
}

void MyOpenGLWidget::mouseMoveEvent(QMouseEvent *event)
{

    if (orbiting)
    {
        camera->Orbit(
                    static_cast<float>((mouse_pos - event->localPos()).x()) * 0.5f,
                    static_cast<float>((mouse_pos - event->localPos()).y()) * 0.5f,
                    cam_focus);
        camera->RotateAxisForward(-camera->GetRot().z()); // correct roll
    }
    else
    {
        camera->RotateAxisUp(static_cast<float>((mouse_pos - event->localPos()).x()));
        camera->RotateAxisLeft(static_cast<float>((mouse_pos - event->localPos()).y()));
        camera->RotateAxisForward(-camera->GetRot().z()); // correct roll
    }

    mouse_pos = event->localPos();
}

void MyOpenGLWidget::keyPressEvent(QKeyEvent *event)
{
    switch (event->key())
    {
    case Qt::Key_W: cam_dir[0] = true; break;
    case Qt::Key_S: cam_dir[1] = true; break;
    case Qt::Key_A: cam_dir[2] = true; break;
    case Qt::Key_D: cam_dir[3] = true; break;
    case Qt::Key_E: cam_dir[4] = true; break;
    case Qt::Key_Q: cam_dir[5] = true; break;
    case Qt::Key_Alt: orbiting = true; break;
    case Qt::Key_F: camera->Focus(cam_focus); break;

    case Qt::Key_T: camera_light_follow = !camera_light_follow; break;
    case Qt::Key_R: lights[0].isActive = !lights[0].isActive; break;
    case Qt::Key_C: ResetLights(); break;
    case Qt::Key_Space:
    {
        current_light + 1 == NR_LIGHTS ? current_light = 1 : current_light++;
        lights[current_light].isActive = true;
        lights[current_light].Position = camera->GetPos();
        break;
    }

    case Qt::Key_X: mode = mode+1.f>9.f?0:mode+1; break;
    case Qt::Key_G: use_deferred = !use_deferred; break;

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
    case Qt::Key_E: cam_dir[4] = false; break;
    case Qt::Key_Q: cam_dir[5] = false; break;
    case Qt::Key_Alt: orbiting = false; break;
    default: break;
    }
}

void MyOpenGLWidget::wheelEvent(QWheelEvent *event)
{
    zoom -= event->angleDelta().y() * 0.01f;

    if (zoom < 1) zoom = 1;
    else if (zoom > 160) zoom = 160;

    m_proj.setToIdentity();
    m_proj.perspective(zoom, GLfloat(width) / height, 0.1f, 100.0f);
}

void MyOpenGLWidget::RenderQuad()
{
    if (quadVAO == 0)
    {
        float quadVertices[] = {
            // positions        // texture Coords
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
             1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
             1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        // setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), nullptr);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }

    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

void MyOpenGLWidget::RenderCube()
{
    if (cubeVAO == 0)
    {
        GLfloat vertices[] = {
            // back face
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
             1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
             1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right
             1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
            -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
            // front face
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
             1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
             1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
             1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
            -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
            // left face
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
            -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
            // right face
             1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
             1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
             1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right
             1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
             1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
             1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left
            // bottom face
            -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
             1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
             1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
             1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
            -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
            -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
            // top face
            -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
             1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
             1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right
             1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
            -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
            -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left
        };
        glGenVertexArrays(1, &cubeVAO);
        glGenBuffers(1, &cubeVBO);
        // fill buffer
        glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        // link vertex attributes
        glBindVertexArray(cubeVAO);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), nullptr);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(GLfloat)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(GLfloat)));
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}

void MyOpenGLWidget::RenderSkybox()
{
    if (skyboxVAO == 0)
    {
        GLfloat skyboxVertices[] = {
            // positions
            -1.0f,  1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,
             1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

             1.0f, -1.0f, -1.0f,
             1.0f, -1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            -1.0f,  1.0f, -1.0f,
             1.0f,  1.0f, -1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
             1.0f, -1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
             1.0f, -1.0f,  1.0f
        };

        glGenVertexArrays(1, &skyboxVAO);
        glBindVertexArray(skyboxVAO);

        glGenBuffers(1, &skyboxVBO);
        glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), nullptr);

        QVector<QString> paths;
        paths.push_back(":/skyboxes/sor_land/land_rt.JPG");
        paths.push_back(":/skyboxes/sor_land/land_up.JPG");
        paths.push_back(":/skyboxes/sor_land/land_bk.JPG");
        paths.push_back(":/skyboxes/sor_land/land_lf.JPG");
        paths.push_back(":/skyboxes/sor_land/land_dn.JPG");
        paths.push_back(":/skyboxes/sor_land/land_ft.JPG");
        current_skybox = LoadSkyboxes(paths);
        paths.clear();
        paths.push_back(":/skyboxes/sor_lake1/lake1_rt.JPG");
        paths.push_back(":/skyboxes/sor_lake1/lake1_up.JPG");
        paths.push_back(":/skyboxes/sor_lake1/lake1_bk.JPG");
        paths.push_back(":/skyboxes/sor_lake1/lake1_lf.JPG");
        paths.push_back(":/skyboxes/sor_lake1/lake1_dn.JPG");
        paths.push_back(":/skyboxes/sor_lake1/lake1_ft.JPG");
        current_skybox = LoadSkyboxes(paths);
        paths.clear();

        qDebug() << "Created Skybox at " << skyboxes[current_skybox]->textureId();

        paths.clear();
    }

    glDepthFunc(GL_LEQUAL);
    programs[SKYBOX]->bind();

    QMatrix4x4 view = camera->GetWorldMatrix().inverted();
    view.translate(-camera->GetPos());
    view.scale(300.0f);
    programs[SKYBOX]->setUniformValue("view", view);
    programs[SKYBOX]->setUniformValue("projection", m_proj);
    glBindVertexArray(skyboxVAO);

    skyboxes[current_skybox]->bind();
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
    programs[SKYBOX]->release();
    glDepthFunc(GL_LESS);
}

int MyOpenGLWidget::LoadSkyboxes(const QVector<QString> paths)
{
    QImage posx = QImage(paths[0]).convertToFormat(QImage::Format_RGBA8888);
    QImage posy = QImage(paths[1]).convertToFormat(QImage::Format_RGBA8888);
    QImage posz = QImage(paths[2]).convertToFormat(QImage::Format_RGBA8888);
    QImage negx = QImage(paths[3]).convertToFormat(QImage::Format_RGBA8888);
    QImage negy = QImage(paths[4]).convertToFormat(QImage::Format_RGBA8888);
    QImage negz = QImage(paths[5]).convertToFormat(QImage::Format_RGBA8888);

    if (posx.isNull()) qDebug() << "ERROR::LoadSkyboxes::Invalid posx Path: " << qApp->applicationDirPath() + paths[0] << endl;
    if (posy.isNull()) qDebug() << "ERROR::LoadSkyboxes::Invalid posy Path: " << qApp->applicationDirPath() + paths[1] << endl;
    if (posz.isNull()) qDebug() << "ERROR::LoadSkyboxes::Invalid posz Path: " << qApp->applicationDirPath() + paths[2] << endl;
    if (negx.isNull()) qDebug() << "ERROR::LoadSkyboxes::Invalid negx Path: " << qApp->applicationDirPath() + paths[3] << endl;
    if (negy.isNull()) qDebug() << "ERROR::LoadSkyboxes::Invalid negy Path: " << qApp->applicationDirPath() + paths[4] << endl;
    if (negz.isNull()) qDebug() << "ERROR::LoadSkyboxes::Invalid negz Path: " << qApp->applicationDirPath() + paths[5] << endl;

    QOpenGLTexture* skybox = new QOpenGLTexture(QOpenGLTexture::TargetCubeMap);
    skybox->create();
    skybox->setSize(posx.width(), posx.height(), posx.depth());
    skybox->setFormat(QOpenGLTexture::RGBA8_UNorm);
    skybox->allocateStorage();

    skybox->setData(0, 0, QOpenGLTexture::CubeMapPositiveX, QOpenGLTexture::RGBA, QOpenGLTexture::UInt8, static_cast<const void*>(posx.constBits()), nullptr);
    skybox->setData(0, 0, QOpenGLTexture::CubeMapPositiveY, QOpenGLTexture::RGBA, QOpenGLTexture::UInt8, static_cast<const void*>(posy.constBits()), nullptr);
    skybox->setData(0, 0, QOpenGLTexture::CubeMapPositiveZ, QOpenGLTexture::RGBA, QOpenGLTexture::UInt8, static_cast<const void*>(posz.constBits()), nullptr);
    skybox->setData(0, 0, QOpenGLTexture::CubeMapNegativeX, QOpenGLTexture::RGBA, QOpenGLTexture::UInt8, static_cast<const void*>(negx.constBits()), nullptr);
    skybox->setData(0, 0, QOpenGLTexture::CubeMapNegativeY, QOpenGLTexture::RGBA, QOpenGLTexture::UInt8, static_cast<const void*>(negy.constBits()), nullptr);
    skybox->setData(0, 0, QOpenGLTexture::CubeMapNegativeZ, QOpenGLTexture::RGBA, QOpenGLTexture::UInt8, static_cast<const void*>(negz.constBits()), nullptr);

    skybox->setWrapMode(QOpenGLTexture::ClampToEdge);
    skybox->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
    skybox->setMagnificationFilter(QOpenGLTexture::LinearMipMapLinear);
    skybox->generateMipMaps();

    skyboxes.push_back(skybox);
    return skyboxes.count() - 1;
}

void MyOpenGLWidget::LoadShaders()
{
    // Default shader
    QOpenGLShaderProgram* d_shader = new QOpenGLShaderProgram;
    if (!d_shader->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/default.vert")) qDebug() << "Error loading default.vert shader";
    if (!d_shader->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/default.frag")) qDebug() << "Error loading default.frag shader";
    d_shader->bindAttributeLocation("vertex", 0);
    d_shader->bindAttributeLocation("normal", 1);
    d_shader->bindAttributeLocation("texCoord", 2);
    d_shader->bindAttributeLocation("tangent", 3);
    d_shader->bindAttributeLocation("bitangent", 4);
    if (!d_shader->link()) qDebug() << "Error Linking Default Shader";
    if (!d_shader->bind()) qDebug() << "Error Binding Default Shader";
    d_projMatrixLoc = d_shader->uniformLocation("projMatrix");
    d_mvMatrixLoc = d_shader->uniformLocation("mvMatrix");
    d_normalMatrixLoc = d_shader->uniformLocation("normalMatrix");
    d_lightPosLoc = d_shader->uniformLocation("lightPos");
    d_lightIntensityLoc = d_shader->uniformLocation("light_intensity");
    d_modeLoc = d_shader->uniformLocation("mode");
    d_textureLoc = d_shader->uniformLocation("texture");
    programs.push_back(d_shader);
    d_shader->release();

    // Single Color shader
    QOpenGLShaderProgram* sc_shader = new QOpenGLShaderProgram();
    if (!sc_shader->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/singlecolor.vert")) qDebug() << "Error loading singlecolor.vert shader";
    if (!sc_shader->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/singlecolor.frag")) qDebug() << "Error loading singlecolor.frag shader";
    sc_shader->bindAttributeLocation("aPos", 0);
    sc_shader->bindAttributeLocation("texCoord", 1);
    if (!sc_shader->link()) qDebug() << "Error Linking Single Color Shader";
    if (!sc_shader->bind()) qDebug() << "Error Binding Single Color Shader";
    sc_modelView = sc_shader->uniformLocation("modelview");
    sc_proj = sc_shader->uniformLocation("projection");
    sc_color = sc_shader->uniformLocation("flat_color");
    sc_alpha = sc_shader->uniformLocation("alpha");
    programs.push_back(sc_shader);
    sc_shader->release();

    // Framebuffer to Screen shader
    QOpenGLShaderProgram* fs_shader = new QOpenGLShaderProgram();
    if (!fs_shader->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/framebuffertoscreen.vert")) qDebug() << "Error loading singlecolor.vert shader";
    if (!fs_shader->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/framebuffertoscreen.frag")) qDebug() << "Error loading singlecolor.frag shader";
    fs_shader->bindAttributeLocation("vertex", 0);
    fs_shader->bindAttributeLocation("texCoord", 1);
    if (!fs_shader->link()) qDebug() << "Error Linking Framebuffer to Screen Shader";
    if (!fs_shader->bind()) qDebug() << "Error Binding Framebuffer to Screen Shader";
    fs_screenTexture = fs_shader->uniformLocation("screenTexture");
    fs_shader->setUniformValue(fs_screenTexture, 0);
    programs.push_back(fs_shader);
    fs_shader->release();

    // Graphic Buffer shader
    QOpenGLShaderProgram* gb_shader = new QOpenGLShaderProgram();
    if (!gb_shader->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/graphic_buffer.vert")) qDebug() << "Error loading graphic_buffer.vert shader";
    if (!gb_shader->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/graphic_buffer.frag")) qDebug() << "Error loading graphic_buffer.frag shader";
    gb_shader->bindAttributeLocation("gPosition", 0);
    gb_shader->bindAttributeLocation("gNormal", 1);
    gb_shader->bindAttributeLocation("gAlbedoSpec", 2);
    if (!gb_shader->link()) qDebug() << "Error Linking Graphic Buffer Shader";
    if (!gb_shader->bind()) qDebug() << "Error Binding Graphic Buffer Shader";
    programs.push_back(gb_shader);
    gb_shader->release();

    // Deferred Shading shader
    QOpenGLShaderProgram* def_shader = new QOpenGLShaderProgram();
    if (!def_shader->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/deferred_shading.vert")) qDebug() << "Error loading deferred_shading.vert shader";
    if (!def_shader->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/deferred_shading.frag")) qDebug() << "Error loading deferred_shading.frag shader";
    if (!def_shader->link()) qDebug() << "Error Linking Deferred Shading Shader";
    if (!def_shader->bind()) qDebug() << "Error Binding Deferred Shading Shader";
    def_posLoc = def_shader->uniformLocation("gPosition");
    def_normalLoc = def_shader->uniformLocation("gNormal");
    def_albedospecLoc = def_shader->uniformLocation("gAlbedoSpec");
    def_shader->setUniformValue(def_posLoc, 0);
    def_shader->setUniformValue(def_normalLoc, 1);
    def_shader->setUniformValue(def_albedospecLoc, 2);
    programs.push_back(def_shader);
    def_shader->release();

    // Deferred Light shader
    QOpenGLShaderProgram* dl_shader = new QOpenGLShaderProgram();
    if (!dl_shader->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/deferred_light.vert")) qDebug() << "Error loading deferred_light.vert shader";
    if (!dl_shader->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/deferred_light.frag")) qDebug() << "Error loading deferred_light.frag shader";
    if (!dl_shader->link()) qDebug() << "Error Linking Lightning Pass Shader";
    if (!dl_shader->bind()) qDebug() << "Error Binding Lightning Pass Shader";
    programs.push_back(dl_shader);
    dl_shader->release();

    // Skybox shader
    QOpenGLShaderProgram* sb_shader = new QOpenGLShaderProgram();
    if (!sb_shader->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/skybox.vert")) qDebug() << "Error loading skybox.vert shader";
    if (!sb_shader->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/skybox.frag")) qDebug() << "Error loading skybox.frag shader";
    if (!sb_shader->link()) qDebug() << "Error Linking Skybox Shader";
    if (!sb_shader->bind()) qDebug() << "Error Binding Skybox Shader";
    programs.push_back(sb_shader);
    sb_shader->release();
}

void MyOpenGLWidget::LoadFramebuffer()
{
    // Framebuffer configuration
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // position buffer
    glGenTextures(1, &gPosition);
    glBindTexture(GL_TEXTURE_2D, gPosition);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);

    // normal color buffer
    glGenTextures(1, &gNormal);
    glBindTexture(GL_TEXTURE_2D, gNormal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);

    // color + specular color buffer
    glGenTextures(1, &gAlbedoSpec);
    glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedoSpec, 0);

    // color attachments
    GLenum buffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
    glDrawBuffers(3, buffers);

    // depth attachment
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);

    // check framebuffer status
    switch(glCheckFramebufferStatus(GL_FRAMEBUFFER)){
    case GL_FRAMEBUFFER_COMPLETE: qInfo() << "Framebuffer Complete"; break; // Everything's OK
    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT: qInfo() << "Framebuffer ERROR: GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT"; break;
    case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT: qInfo() << "Framebuffer ERROR: GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT"; break;
    case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER: qInfo() << "Framebuffer ERROR: GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER"; break;
    case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER: qInfo() << "Framebuffer ERROR: GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER"; break;
    case GL_FRAMEBUFFER_UNSUPPORTED: qInfo() << "Framebuffer ERROR: GL_FRAMEBUFFER_UNSUPPORTED"; break;
    default: qInfo() << "Framebuffer ERROR: Unknown ERROR"; }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void MyOpenGLWidget::DrawBordered()
{
    if (border_meshes.isEmpty())
        return;

    state = BORDERED_MODELS;

    // Draw mesh and write to stencil
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glStencilMask(0xFF);

    for (int i = 0; i < border_meshes.count(); i++)
        DrawMesh(border_meshes[i]);

    state = BORDERS;

    // Draw border from stencil
    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    glStencilMask(0x00);

    if (border_over_borderless)
        glDisable(GL_DEPTH_TEST);

    for (int i = 0; i < border_meshes.count(); i++)
        DrawMesh(border_meshes[i], SINGLE_COLOR);

    glStencilMask(0xFF);

    if (border_over_borderless)
        glEnable(GL_DEPTH_TEST);

    border_meshes.clear();
}

void MyOpenGLWidget::PostProcessDeferredLights()
{
    if (!use_deferred)
        return;

    state = POST_PROCESSING;

    QOpenGLFramebufferObject::bindDefault();

    // Lighting pass
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    programs[DEFERRED_SHADING]->bind();
    programs[DEFERRED_SHADING]->setUniformValue("viewPos", camera->GetPos());

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gPosition);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, gNormal);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);

    // Update and set scene lights
    for (int i = 0; i < lights.size(); i++)
    {
        if (!lights[i].updated)
        {
            lights[i].maxBrightness = std::fmaxf(std::fmaxf(lights[i].Color.x(), lights[i].Color.y()), lights[i].Color.z());
            lights[i].radius = (-lights[i].Linear + std::sqrt(lights[i].Linear * lights[i].Linear - 4 * lights[i].Quadratic * (lights[i].Constant - (256.0f / 5.0f) * lights[i].maxBrightness))) / (2.0f * lights[i].Quadratic);
            lights[i].updated = true;
        }
        programs[DEFERRED_SHADING]->setUniformValue(QString("lights[" + QString::number(i) + "].isActive").toStdString().c_str(), lights[i].isActive);
        programs[DEFERRED_SHADING]->setUniformValue(QString("lights[" + QString::number(i) + "].Position").toStdString().c_str(), lights[i].Position);
        programs[DEFERRED_SHADING]->setUniformValue(QString("lights[" + QString::number(i) + "].Color").toStdString().c_str(), lights[i].Color);
        programs[DEFERRED_SHADING]->setUniformValue(QString("lights[" + QString::number(i) + "].Intensity").toStdString().c_str(), lights[i].Intensity);
        programs[DEFERRED_SHADING]->setUniformValue(QString("lights[" + QString::number(i) + "].Range").toStdString().c_str(), lights[i].MinRange);
        programs[DEFERRED_SHADING]->setUniformValue(QString("lights[" + QString::number(i) + "].Linear").toStdString().c_str(), lights[i].Linear);
        programs[DEFERRED_SHADING]->setUniformValue(QString("lights[" + QString::number(i) + "].Quadratic").toStdString().c_str(), lights[i].Quadratic);
        programs[DEFERRED_SHADING]->setUniformValue(QString("lights[" + QString::number(i) + "].Radius").toStdString().c_str(), lights[i].radius);
    }

    RenderQuad();
    programs[DEFERRED_SHADING]->release();

    // copy geometry depth buffer to framebuffer's depth buffer
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // render lights on top of scene (WIP: wont draw those damm cubes)
    programs[DEFERRED_LIGHT]->bind();
    programs[DEFERRED_LIGHT]->setUniformValue("projection", m_proj);

    for (int i = 0; i < lights.count(); i++)
    {
        QMatrix4x4 w;
        w.setToIdentity();
        w.translate(lights[i].Position);
        programs[DEFERRED_LIGHT]->setUniformValue("mv", camera->GetWorldMatrix().inverted() * w);
        programs[DEFERRED_LIGHT]->setUniformValue("lightColor", lights[i].Color);
        RenderCube();
    }
    programs[DEFERRED_LIGHT]->release();
}

