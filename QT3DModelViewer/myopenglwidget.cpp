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
#include <iostream>

MyOpenGLWidget::MyOpenGLWidget(QWidget *parent)
    : QOpenGLWidget(parent), tick_count(0)
{
    setFocusPolicy(Qt::ClickFocus);

    cam.transform = new Transform(nullptr, true, {0,0,5},{0,0,0});
    lightPos = {0,0,2};
    lightColor = {1, 1, 1};
    for (int i = 0; i < 6; i++) cam_dir[i] = false;

    // Tick Widget
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MyOpenGLWidget::Tick);
    timer->start(static_cast <int>(tick_period));
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

    // First Light
    Light base;
    base.Position = {5, 10, 5};
    base.Color = {0.8f, 0.8f, 0.8f};
    base.Intensity = 10.0;
    base.Radius = 0.0;
    base.TypeLight = 0;
    lights.push_back(base);

    // Default shader
    programs.push_back(new QOpenGLShaderProgram);
    if (!programs[0]->addShaderFromSourceFile(QOpenGLShader::Vertex, qApp->applicationDirPath() + "/Shaders/default.vert"))
        qDebug() << "Error loading deault.vert shader";
    if (!programs[0]->addShaderFromSourceFile(QOpenGLShader::Fragment, qApp->applicationDirPath() + "/Shaders/default.frag"))
        qDebug() << "Error loading deault.frag shader";

    programs[0]->bindAttributeLocation("vertex", 0);
    programs[0]->bindAttributeLocation("normal", 1);
    programs[0]->bindAttributeLocation("texCoord", 2);
    programs[0]->bindAttributeLocation("tangent", 3);
    programs[0]->bindAttributeLocation("bitangent", 4);

    if (!programs[0]->link())
        qDebug() << "Error Linking Default Shader";
    if (!programs[0]->bind())
        qDebug() << "Error Binding Default Shader";

    cam.m_projMatrixLoc =   programs[0]->uniformLocation("projMatrix");
    m_mvMatrixLoc =         programs[0]->uniformLocation("mvMatrix");
    m_normalMatrixLoc =     programs[0]->uniformLocation("normalMatrix");
    m_lightPosLoc =         programs[0]->uniformLocation("lightPos");
    m_lightIntensityLoc =   programs[0]->uniformLocation("light_intensity");
    m_modeLoc =             programs[0]->uniformLocation("mode");
    m_textureLoc =          programs[0]->uniformLocation("texture");

    programs[0]->release();


    /*/ Shaders
    programs.push_back(new QOpenGLShaderProgram);
    programs[1]->create();
    programs[1]->addShaderFromSourceFile(QOpenGLShader::Vertex, "graphic_buffer.vert");
    programs[1]->addShaderFromSourceFile(QOpenGLShader::Fragment, "graphic_buffer.frag");
    programs[1]->link();

    programs.push_back(new QOpenGLShaderProgram);
    programs[2]->create();
    programs[2]->addShaderFromSourceFile(QOpenGLShader::Vertex, "deferred_shading.vert");
    programs[2]->addShaderFromSourceFile(QOpenGLShader::Fragment, "deferred_shading.frag");
    programs[2]->link();

    programs.push_back(new QOpenGLShaderProgram);
    programs[3]->create();
    programs[3]->addShaderFromSourceFile(QOpenGLShader::Vertex, "deferred_light.vert");
    programs[3]->addShaderFromSourceFile(QOpenGLShader::Fragment, "deferred_light.frag");
    programs[3]->link();*/

    scene->InitDemo(this);
}

void MyOpenGLWidget::paintGL()
{
    // RESET
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST); // Enable depth buffer
    glEnable(GL_CULL_FACE); // Enable back face culling

    //Render();
    if (scene != nullptr)
        scene->Draw(this);
}

void MyOpenGLWidget::DrawMesh(Mesh* mesh)
{
    if (mesh == nullptr)// || !mesh->vao.isCreated())
        return;

    QMatrix4x4 m_world = mesh->gameobject->transform->GetWorldMatrix();
    m_world.rotate(180.f, {0,0,1});

    programs[0]->bind();
    programs[0]->setUniformValue(cam.m_projMatrixLoc, cam.m_proj);
    programs[0]->setUniformValue(m_mvMatrixLoc, cam.transform->GetWorldMatrix().inverted() * m_world);
    programs[0]->setUniformValue(m_normalMatrixLoc, m_world.normalMatrix());
    programs[0]->setUniformValue(m_lightPosLoc, lightPos);
    programs[0]->setUniformValue(m_lightIntensityLoc, lightColor);
    programs[0]->setUniformValue(m_modeLoc, mode);

    for(int i = 0; i < mesh->sub_meshes.size(); i++)
    {
        SubMesh* sub = mesh->sub_meshes[i];

        if(sub->vao.isCreated())
        {
            for (int i = 0; i < sub->textures.size(); i++)
            {
                if(sub->textures[i].glTexture != nullptr && sub->textures[i].glTexture->isCreated())
                {
                    sub->textures[i].glTexture->bind();
                }
            }

            QOpenGLVertexArrayObject::Binder vaoBinder(&sub->vao);

            if(sub->num_faces > 0)
                glDrawElements(GL_TRIANGLES, sub->num_faces * 3, GL_UNSIGNED_INT, nullptr);
            else
                glDrawArrays(GL_TRIANGLES, 0, sub->num_vertices);

            for (int i = 0; i < sub->textures.count(); i++)
            {
                if(sub->textures[i].glTexture != nullptr && sub->textures[i].glTexture->isCreated())
                {
                    sub->textures[i].glTexture->release();
                }
            }
        }
    }

    programs[0]->release();
}

void MyOpenGLWidget::LoadMesh(SubMesh *mesh)
{
    if (mesh == nullptr || mesh->num_vertices <= 0)
        return;

    mesh->vao.create();
    QOpenGLVertexArrayObject::Binder vaoBinder(&mesh->vao);

    // Setup our POSITIONS
    mesh->vbo.create();
    mesh->vbo.setUsagePattern( QOpenGLBuffer::StaticDraw );
    mesh->vbo.bind();
    mesh->vbo.allocate(mesh->vertex_data.constData(), 3 * mesh->num_vertices * static_cast<int>(sizeof(GLfloat)));

    programs[0]->enableAttributeArray(0);
    programs[0]->setAttributeBuffer(0, GL_FLOAT, 0, 3);

    // Setup our NORMALS
    mesh->nbo.create();
    mesh->nbo.setUsagePattern( QOpenGLBuffer::StaticDraw );
    mesh->nbo.bind();
    mesh->nbo.allocate(mesh->normal_data.constData(), 3 * mesh->num_vertices * static_cast<int>(sizeof(GLfloat)));

    programs[0]->enableAttributeArray(1);
    programs[0]->setAttributeBuffer(1, GL_FLOAT, 0, 3);

    // Setup TEXTURE COORDS
    mesh->tbo.create();
    mesh->tbo.setUsagePattern( QOpenGLBuffer::StaticDraw );
    mesh->tbo.bind();
    mesh->tbo.allocate(mesh->texcoord_data.constData(), 2 * mesh->num_vertices * static_cast<int>(sizeof(GLfloat)));

    programs[0]->enableAttributeArray(2);
    programs[0]->setAttributeBuffer(2, GL_FLOAT, 0, 2);

    // Setup our TANGENTS & BITANGENTS
    mesh->tnbo.create();
    mesh->tnbo.setUsagePattern( QOpenGLBuffer::StaticDraw );
    mesh->tnbo.bind();
    mesh->tnbo.allocate(mesh->tangent_data.constData(), 3 * mesh->num_vertices * static_cast<int>(sizeof(GLfloat)));

    programs[0]->enableAttributeArray(3);
    programs[0]->setAttributeBuffer(3, GL_FLOAT, 0, 3);

    mesh->btnbo.create();
    mesh->btnbo.setUsagePattern( QOpenGLBuffer::StaticDraw );
    mesh->btnbo.bind();
    mesh->btnbo.allocate(mesh->bitangent_data.constData(), 3 * mesh->num_vertices * static_cast<int>(sizeof(GLfloat)));

    programs[0]->enableAttributeArray(4);
    programs[0]->setAttributeBuffer(4, GL_FLOAT, 0, 3);

    // Setup INDEXES
    if (mesh->num_faces > 0)
    {
        mesh->ibo.create();
        mesh->ibo.setUsagePattern( QOpenGLBuffer::StaticDraw );
        mesh->ibo.bind();
        mesh->ibo.allocate(mesh->index_data.constData(), 3 * mesh->num_faces * static_cast<int>(sizeof(GLint)));

    }

    mesh->f = QOpenGLContext::currentContext()->functions();
}

void MyOpenGLWidget::resizeGL(int width, int height)
{
    cam.m_proj.setToIdentity();
    cam.m_proj.perspective(45.0f, GLfloat(width) / height, 0.01f, 1000.0f);

    //DeleteBuffers();
    //Resize(this->width = width, this->height = height);
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
    case Qt::Key_F: lightPos = cam.transform->GetPos(); break;
    case Qt::Key_X: mode = mode+1.f>3.f?0:mode+1; break;
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























































void MyOpenGLWidget::DeleteBuffers()
{
    glDeleteTextures(1, &gPosition);
    glDeleteTextures(1, &gNormal);
    glDeleteTextures(1, &gAlbedoSpec);
    /*glDeleteTextures(1, &gDepth);
    glDeleteFramebuffers(1, &gBuffer);

    glDeleteTextures(1, &lighting);
    glDeleteFramebuffers(1, &lightingfbo);
    glDeleteTextures(1, &blurHV);
    glDeleteFramebuffers(1, &blurfbo);
    glDeleteTextures(1, &bloomfbo);
    glDeleteFramebuffers(1, &bloom);
    glDeleteTextures(1, &finalBloom);
    glDeleteFramebuffers(1, &finalBloomfbo);*/
}

void MyOpenGLWidget::ResizeS(int width,int height)
{
    // Frame Buffer
    glGenFramebuffers(1, &gBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);

    // position color buffer
    glGenTextures(1, &gPosition);
    glBindTexture(GL_TEXTURE_2D, gPosition);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);
    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    // normal color buffer
    glGenTextures(1, &gNormal);
    glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);
    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    // color + specular color buffer
    glGenTextures(1, &gAlbedoSpec);
    glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedoSpec, 0);
    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    /*/ Depth
    GL->glGenTextures(1, &depthTexture);
    GL->glBindTexture(GL_TEXTURE_2D, depthTexture);
    GL->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    GL->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    GL->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    GL->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    GL->glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    */

    GLenum buffers[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 , GL_COLOR_ATTACHMENT2};
    static_cast<QOpenGLFunctions_3_3_Core*>(this)->glDrawBuffers(3, buffers);

    srand(13);
    for (unsigned int i = 0; i < 1; i++)
    {
        // calculate slightly random offsets
        float xPos = ((rand() % 100) / 100.0) * 6.0 - 3.0;
        float yPos = ((rand() % 100) / 100.0) * 6.0 - 4.0;
        float zPos = ((rand() % 100) / 100.0) * 6.0 - 3.0;
        lightPos = {xPos, yPos, zPos};

        // also calculate random color
        float rColor = ((rand() % 100) / 200.0f) + 0.5; // between 0.5 and 1.0
        float gColor = ((rand() % 100) / 200.0f) + 0.5; // between 0.5 and 1.0
        float bColor = ((rand() % 100) / 200.0f) + 0.5; // between 0.5 and 1.0
        lightColor = {rColor, gColor, bColor};
    }
/*
    glGenTextures(1, &lighting);
    glBindTexture(GL_TEXTURE_2D, lighting);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
*/
    /*/ Frame Buffer
    glGenFramebuffers(1, &lightingfbo);
    glBindFramebuffer(GL_FRAMEBUFFER, lightingfbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, lighting, 0);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
*/
    /*
    GL->glGenTextures(1, &blurHV);
    GL->glBindTexture(GL_TEXTURE_2D, blurHV);
    GL->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    GL->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    GL->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    GL->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    GL->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    // Frame Buffer
    GL->glGenFramebuffers(1, &blurfbo);
    GL->glBindFramebuffer(GL_FRAMEBUFFER, blurfbo);
    GL->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, blurHV, 0);
    GL->glDrawBuffer(GL_COLOR_ATTACHMENT0);

    GL->glGenTextures(1, &bloom);
    GL->glBindTexture(GL_TEXTURE_2D, bloom);
    GL->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    GL->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    GL->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    GL->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    GL->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    // Bloom
    GL->glGenFramebuffers(1, &bloomfbo);
    GL->glBindFramebuffer(GL_FRAMEBUFFER, bloomfbo);
    GL->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bloom, 0);
    GL->glDrawBuffer(GL_COLOR_ATTACHMENT0);

    GL->glGenTextures(1, &finalBloom);
    GL->glBindTexture(GL_TEXTURE_2D, finalBloom);
    GL->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    GL->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    GL->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    GL->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    GL->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    // Frame Buffer
    GL->glGenFramebuffers(1, &finalBloomfbo);
    GL->glBindFramebuffer(GL_FRAMEBUFFER, finalBloomfbo);
    GL->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, finalBloom, 0);
    GL->glDrawBuffer(GL_COLOR_ATTACHMENT0);*/


    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    switch(status)
    {
    case GL_FRAMEBUFFER_COMPLETE: // Everything's OK
    break;
    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
    qInfo() << "Framebuffer ERROR: GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT"; break;
    case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
    qInfo() << "Framebuffer ERROR: GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT"; break;
    case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
    qInfo() << "Framebuffer ERROR: GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER"; break;
    case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
    qInfo() << "Framebuffer ERROR: GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER"; break;
    case GL_FRAMEBUFFER_UNSUPPORTED:
    qInfo() << "Framebuffer ERROR: GL_FRAMEBUFFER_UNSUPPORTED"; break;
    default:
    qInfo() << "Framebuffer ERROR: Unknown ERROR";
    }


}


void MyOpenGLWidget::Render()
{
    /*glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 1. geometry pass: render scene's geometry/color data into gbuffer
    // -----------------------------------------------------------------------------------------------------------------------

    if(programs[2]->bind())
    {
        glUniformMatrix4fv(programs[2]->uniformLocation("projectionMatrix"), 1, GL_FALSE, cam.m_proj.data());
        foreach (GameObject* go, scene->root->childs)
        {
            if(go->components[1]->type == MESH)
            {
                /*glUniformMatrix4fv(programs[2]->uniformLocation("modelViewMatrix"),
                                       1,
                                       GL_FALSE,
                                       (cam.transform->GetWorldMatrix() * go->transform->GetWorldMatrix()).data());
                glUniformMatrix4fv(programs[2]->uniformLocation("modelWorldMatrix"),
                                       1,
                                       GL_FALSE,
                                       go->transform->GetWorldMatrix().data());

                glActiveTexture(GL_TEXTURE0);*//*

                QVector<SubMesh*> submeshes = static_cast<Mesh*>(go->components[1])->sub_meshes;

                for(unsigned int i = 0; i < submeshes.size(); i++)
                {
                    // bind appropriate textures
                    unsigned int diffuseNr  = 1;
                    unsigned int specularNr = 1;
                    unsigned int normalNr   = 1;
                    unsigned int heightNr   = 1;

                    glActiveTexture(GL_TEXTURE0 + submeshes[i]->texture->textureId()); // active proper texture unit before binding

                    if(submeshes[i]->vao.isCreated())
                    {
                        bool release_texture = false;

                        if(submeshes[i]->texture != nullptr && submeshes[i]->texture->isCreated())
                        {
                            //glUniform1i(glGetUniformLocation(shader.ID, (name + number).c_str()), i);
                            submeshes[i]->texture->bind();
                            release_texture = true;
                        }

                        QOpenGLVertexArrayObject::Binder vaoBinder(&submeshes[i]->vao);

                        if(submeshes[i]->num_faces > 0)
                            glDrawElements(GL_TRIANGLES, submeshes[i]->num_faces * 3, GL_UNSIGNED_INT, nullptr);
                        else
                            glDrawArrays(GL_TRIANGLES, 0, submeshes[i]->num_vertices);

                        if (release_texture)
                            submeshes[i]->texture->release();

                        // always good practice to set everything back to defaults once configured.
                        glActiveTexture(GL_TEXTURE0);
                    }
                }
            }
        }
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);



    // 2. lighting pass: calculate lighting by iterating over a screen filled quad pixel-by-pixel using the gbuffer's content.
    // -----------------------------------------------------------------------------------------------------------------------
    programs[2]->bind();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gPosition);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, gNormal);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);

    // send light relevant uniform
    programs[2]->setUniformValue("light.Position", lightPos);
    programs[2]->setUniformValue("light.Color", lightColor);

    // update attenuation parameters and calculate radius
    const float constant = 1.0f; // note that we don't send this to the shader, we assume it is always 1.0 (in our case)
    const float linear = 0.7f;
    const float quadratic = 1.8f;

    programs[2]->setUniformValue("light.Linear", linear);
    programs[2]->setUniformValue("light.Quadratic", quadratic);

    // then calculate radius of light volume/sphere
    const float maxBrightness = std::fmaxf(std::fmaxf(lightColor.x(), lightColor.y()), lightColor.z());
    float radius = (-linear + std::sqrt(linear * linear - 4 * quadratic * (constant - (256.0f / 5.0f) * maxBrightness))) / (2.0f * quadratic);
    programs[2]->setUniformValue("light.Radius", radius);
    programs[2]->setUniformValue("viewPos", cam.transform->GetPos());
    // finally render quad
    RenderQuad();

    // 2.5. copy content of geometry's depth buffer to default framebuffer's depth buffer
    // -----------------------------------------------------------------------------------------------------------------------
    glBindFramebuffer(GL_READ_FRAMEBUFFER, gBuffer);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); // write to default framebuffer
    // blit to default framebuffer. Note that this may or may not work as the internal formats of both the FBO and default framebuffer have to match.
    // the internal formats are implementation defined. This works on all of my systems, but if it doesn't on yours you'll likely have to write to the
    // depth buffer in another shader stage (or somehow see to match the default framebuffer's internal format with the FBO's internal format).
    glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    programs[2]->setUniformValue("gPosition", 0);
*/}

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
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}
