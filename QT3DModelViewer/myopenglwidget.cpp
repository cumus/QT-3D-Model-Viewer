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

    cam.transform = new Transform(nullptr, true, {0,0,5},{0,0,0});
    lightPos = {0,0,2};
    lightColor = {1, 1, 1};
    for (int i = 0; i < 6; i++) cam_dir[i] = false;

    border_color = QVector3D(1,0.27f,0);
    border_meshes.clear();

    QSize size = sizeHint();
    width = size.width();
    height = size.height();

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

    if (cam_dir[0]) cam.transform->TranslateForward(0.01f * tick_period);
    if (cam_dir[1]) cam.transform->TranslateForward(-0.01f * tick_period);
    if (cam_dir[2]) cam.transform->TranslateLeft(0.01f * tick_period);
    if (cam_dir[3]) cam.transform->TranslateLeft(-0.01f * tick_period);
    if (cam_dir[4]) cam.transform->TranslateUp(0.01f * tick_period);
    if (cam_dir[5]) cam.transform->TranslateUp(-0.01f * tick_period);

    update();
}

void MyOpenGLWidget::initializeGL()
{
    state = INITIALIZING;
    initializeOpenGLFunctions();

    // Default shader
    QOpenGLShaderProgram* d_shader = new QOpenGLShaderProgram;
    if (!d_shader->addShaderFromSourceFile(QOpenGLShader::Vertex, qApp->applicationDirPath() + "/Shaders/default.vert")) qDebug() << "Error loading default.vert shader";
    if (!d_shader->addShaderFromSourceFile(QOpenGLShader::Fragment, qApp->applicationDirPath() + "/Shaders/default.frag")) qDebug() << "Error loading default.frag shader";
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
    if (!sc_shader->addShaderFromSourceFile(QOpenGLShader::Vertex, qApp->applicationDirPath() + "/Shaders/singlecolor.vert")) qDebug() << "Error loading singlecolor.vert shader";
    if (!sc_shader->addShaderFromSourceFile(QOpenGLShader::Fragment, qApp->applicationDirPath() + "/Shaders/singlecolor.frag")) qDebug() << "Error loading singlecolor.frag shader";
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
    if (!fs_shader->addShaderFromSourceFile(QOpenGLShader::Vertex, qApp->applicationDirPath() + "/Shaders/framebuffertoscreen.vert")) qDebug() << "Error loading singlecolor.vert shader";
    if (!fs_shader->addShaderFromSourceFile(QOpenGLShader::Fragment, qApp->applicationDirPath() + "/Shaders/framebuffertoscreen.frag")) qDebug() << "Error loading singlecolor.frag shader";
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
    if (!gb_shader->addShaderFromSourceFile(QOpenGLShader::Vertex, qApp->applicationDirPath() + "/Shaders/graphic_buffer.vert")) qDebug() << "Error loading graphic_buffer.vert shader";
    if (!gb_shader->addShaderFromSourceFile(QOpenGLShader::Fragment, qApp->applicationDirPath() + "/Shaders/graphic_buffer.frag")) qDebug() << "Error loading graphic_buffer.frag shader";
    gb_shader->bindAttributeLocation("gPosition", 0);
    gb_shader->bindAttributeLocation("gNormal", 1);
    gb_shader->bindAttributeLocation("gAlbedoSpec", 2);
    if (!gb_shader->link()) qDebug() << "Error Linking Graphic Buffer Shader";
    if (!gb_shader->bind()) qDebug() << "Error Binding Graphic Buffer Shader";
    programs.push_back(gb_shader);
    gb_shader->release();

    // Deferred Shading shader
    QOpenGLShaderProgram* def_shader = new QOpenGLShaderProgram();
    if (!def_shader->addShaderFromSourceFile(QOpenGLShader::Vertex, qApp->applicationDirPath() + "/Shaders/deferred_shading.vert")) qDebug() << "Error loading deferred_shading.vert shader";
    if (!def_shader->addShaderFromSourceFile(QOpenGLShader::Fragment, qApp->applicationDirPath() + "/Shaders/deferred_shading.frag")) qDebug() << "Error loading deferred_shading.frag shader";
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
    if (!dl_shader->addShaderFromSourceFile(QOpenGLShader::Vertex, qApp->applicationDirPath() + "/Shaders/deferred_light.vert")) qDebug() << "Error loading deferred_light.vert shader";
    if (!dl_shader->addShaderFromSourceFile(QOpenGLShader::Fragment, qApp->applicationDirPath() + "/Shaders/deferred_light.frag")) qDebug() << "Error loading deferred_light.frag shader";
    if (!dl_shader->link()) qDebug() << "Error Linking Lightning Pass Shader";
    if (!dl_shader->bind()) qDebug() << "Error Binding Lightning Pass Shader";
    programs.push_back(dl_shader);
    dl_shader->release();

    // Framebuffer configuration
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    //glGenTextures(1, &textureColorbuffer);
    //glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);

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

    // tell OpenGL which color attachments we'll use (of this framebuffer) for rendering
    GLenum buffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
    glDrawBuffers(3, buffers);

    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);

    /*glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height); // use a single renderbuffer object for both a depth AND stencil buffer.
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);*/ // now actually attach it

    switch(glCheckFramebufferStatus(GL_FRAMEBUFFER)){
    case GL_FRAMEBUFFER_COMPLETE: qInfo() << "Framebuffer Complete"; break; // Everything's OK
    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT: qInfo() << "Framebuffer ERROR: GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT"; break;
    case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT: qInfo() << "Framebuffer ERROR: GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT"; break;
    case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER: qInfo() << "Framebuffer ERROR: GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER"; break;
    case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER: qInfo() << "Framebuffer ERROR: GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER"; break;
    case GL_FRAMEBUFFER_UNSUPPORTED: qInfo() << "Framebuffer ERROR: GL_FRAMEBUFFER_UNSUPPORTED"; break;
    default: qInfo() << "Framebuffer ERROR: Unknown ERROR"; }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    srand(13);
    for (unsigned int i = 0; i < NR_LIGHTS; i++)
    {
        // calculate slightly random offsets
        float xPos = ((rand() % 100) / 100.0f) * 6.0f - 3.0f;
        float yPos = ((rand() % 100) / 100.0f) * 6.0f - 4.0f;
        float zPos = ((rand() % 100) / 100.0f) * 6.0f - 3.0f;
        lightPositions.push_back({xPos, yPos, zPos});

        // also calculate random color
        float rColor = ((rand() % 100) / 200.0f) + 0.5f; // between 0.5 and 1.0
        float gColor = ((rand() % 100) / 200.0f) + 0.5f; // between 0.5 and 1.0
        float bColor = ((rand() % 100) / 200.0f) + 0.5f; // between 0.5 and 1.0
        lightColors.push_back({rColor, gColor, bColor});
    }

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

    state = INITIALIZED;

    scene->InitDemo(this);
}

void MyOpenGLWidget::paintGL()
{
    if (use_deferred)
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    else
        QOpenGLFramebufferObject::bindDefault();

    // RESET
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT| GL_STENCIL_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glStencilMask(0x00);

    state = MODELS;
    border_meshes.clear();

    // 1. geometry pass: render scene's geometry/color data into gbuffer
    // -----------------------------------------------------------------
    if (scene != nullptr)
        scene->Draw(this);

    if (!border_meshes.isEmpty())
    {
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

        if (border_over_borderless) glDisable(GL_DEPTH_TEST);

        for (int i = 0; i < border_meshes.count(); i++)
            DrawMesh(border_meshes[i], SINGLE_COLOR);

        glStencilMask(0xFF);

        if (border_over_borderless) glEnable(GL_DEPTH_TEST);

        border_meshes.clear();
    }

    state = POST_PROCESSING;

    if (use_deferred)
    {
        QOpenGLFramebufferObject::bindDefault();
        /*glDisable(GL_DEPTH_TEST);
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);*/

        // 2. lighting pass: calculate lighting by iterating over a screen filled quad pixel-by-pixel using the gbuffer's content.
        // -----------------------------------------------------------------------------------------------------------------------
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        programs[DEFERRED_SHADING]->bind();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, gPosition);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, gNormal);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);

        for (int i = 0; i < lightPositions.size(); i++)
        {
            programs[DEFERRED_SHADING]->setUniformValue(QString("lights[" + QString(i) + "].Position").toStdString().c_str(), lightPositions[i]);
            programs[DEFERRED_SHADING]->setUniformValue(QString("lights[" + QString(i) + "].Color").toStdString().c_str(), lightColors[i]);

            // update attenuation parameters and calculate radius
            const float constant = 1.0f; // note that we don't send this to the shader, we assume it is always 1.0 (in our case)
            const float linear = 0.7f;
            const float quadratic = 1.8f;
            programs[DEFERRED_SHADING]->setUniformValue(QString("lights[" + QString(i) + "].Linear").toStdString().c_str(), linear);
            programs[DEFERRED_SHADING]->setUniformValue(QString("lights[" + QString(i) + "].Quadratic").toStdString().c_str(), quadratic);

            // then calculate radius of light volume/sphere
            const float maxBrightness = std::fmaxf(std::fmaxf(lightColors[i].x(), lightColors[i].y()), lightColors[i].z());
            float radius = (-linear + std::sqrt(linear * linear - 4 * quadratic * (constant - (256.0f / 5.0f) * maxBrightness))) / (2.0f * quadratic);
            programs[DEFERRED_SHADING]->setUniformValue(QString("lights[" + QString(i) + "].Radius").toStdString().c_str(), radius);
        }

        programs[DEFERRED_SHADING]->setUniformValue("viewPos", cam.transform->GetPos());
        RenderQuad();
        programs[DEFERRED_SHADING]->release();

        // 2.5. copy content of geometry's depth buffer to default framebuffer's depth buffer
        // ----------------------------------------------------------------------------------
        glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // 3. render lights on top of scene
        // --------------------------------
        QMatrix4x4 w;
        programs[DEFERRED_LIGHT]->bind();
        programs[DEFERRED_LIGHT]->setUniformValue("projection", cam.m_proj);
        for (int i = 0; i < lightPositions.size(); i++)
        {
            w.setToIdentity();
            w.translate(lightPositions[i]);
            w.scale(0.125f);

            programs[DEFERRED_LIGHT]->setUniformValue("mv", cam.transform->GetWorldMatrix().inverted() * w);
            programs[DEFERRED_LIGHT]->setUniformValue("lightColor", lightColors[i]);
            RenderCube();
        }
        programs[DEFERRED_LIGHT]->release();
    }

    state = FINISHED;
}

void MyOpenGLWidget::DrawMesh(Mesh* mesh, SHADER_TYPE shader)
{
    if (mesh == nullptr || shader >= programs.count())
        return;
    else if (mesh->draw_border && state == MODELS)
    {
        border_meshes.push_back(mesh);
        return;
    }

    if (use_deferred) shader = GRAPHIC_BUFFER;

    QMatrix4x4 m_world = mesh->gameobject->transform->GetWorldMatrix();
    m_world.rotate(180.f, {0,0,1});

    QOpenGLShaderProgram* program = programs[static_cast<int>(shader)];
    program->bind();

    switch (shader)
    {
    case DEFAULT:
    {
        program->setUniformValue(d_projMatrixLoc, cam.m_proj);
        program->setUniformValue(d_mvMatrixLoc, cam.transform->GetWorldMatrix().inverted() * m_world);
        program->setUniformValue(d_normalMatrixLoc, m_world.normalMatrix());
        program->setUniformValue(d_lightPosLoc, lightPos);
        program->setUniformValue(d_lightIntensityLoc, lightColor);
        program->setUniformValue(d_modeLoc, mode);

        for(int i = 0; i < mesh->sub_meshes.size(); i++)
        {
            SubMesh* sub = mesh->sub_meshes[i];

            if(sub->vao.isCreated())
            {
                // bind appropriate textures
                unsigned int diffuseNr  = 1;
                unsigned int specularNr = 1;
                unsigned int normalNr   = 1;
                unsigned int heightNr   = 1;

                for(int i = 0; i < sub->textures.size(); i++)
                {
                    glActiveTexture(GL_TEXTURE0 + static_cast<unsigned int>(i)); // active proper texture unit before binding
                    // retrieve texture number (the N in diffuse_textureN)
                    QString number;
                    QString name = sub->textures[i].type;
                    if(name == "texture_diffuse") number = QString(diffuseNr++);
                    else if(name == "texture_specular") number = QString(specularNr++);
                    else if(name == "texture_normal") number = QString(normalNr++);
                    else if(name == "texture_height") number = QString(heightNr++);

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
        program->setUniformValue(sc_proj, cam.m_proj);
        program->setUniformValue(sc_modelView, cam.transform->GetWorldMatrix().inverted() * m_world);
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
        program->setUniformValue("projection", cam.m_proj);
        program->setUniformValue("view", cam.transform->GetWorldMatrix().inverted());
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
                unsigned int normalNr   = 1;
                unsigned int heightNr   = 1;

                for(int i = 0; i < sub->textures.size(); i++)
                {
                    glActiveTexture(GL_TEXTURE0 + static_cast<unsigned int>(i)); // active proper texture unit before binding
                    // retrieve texture number (the N in diffuse_textureN)
                    QString number;
                    QString name = sub->textures[i].type;
                    if(name == "texture_diffuse") number = QString(diffuseNr++);
                    else if(name == "texture_specular") number = QString(specularNr++);
                    else if(name == "texture_normal") number = QString(normalNr++);
                    else if(name == "texture_height") number = QString(heightNr++);

                    program->setUniformValue((name + number).toStdString().c_str(), static_cast<unsigned int>(i));
                    sub->textures[i].glTexture->bind();
                }

                QOpenGLVertexArrayObject::Binder vaoBinder(&sub->vao);
                if(sub->num_faces > 0) glDrawElements(GL_TRIANGLES, sub->num_faces * 3, GL_UNSIGNED_INT, nullptr);
                else glDrawArrays(GL_TRIANGLES, 0, sub->num_vertices);

                for (int i = 0; i < sub->textures.count(); i++)
                    if(sub->textures[i].glTexture != nullptr && sub->textures[i].glTexture->isCreated())
                        sub->textures[i].glTexture->release();

                glActiveTexture(GL_TEXTURE0);
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

    programs[0]->release();
}

void MyOpenGLWidget::resizeGL(int width, int height)
{
    cam.m_proj.setToIdentity();
    cam.m_proj.perspective(45.0f, GLfloat(width) / height, 0.1f, 100.0f);

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
    case Qt::Key_X: mode = mode+1.f>7.f?0:mode+1; break;
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
    case Qt::Key_Space: cam_dir[4] = false; break;
    case Qt::Key_E: cam_dir[5] = false; break;
    default: break;
    }
}




/*void MyOpenGLWidget::DeleteBuffers()
{
    glDeleteTextures(1, &gPosition);
    glDeleteTextures(1, &gNormal);
    glDeleteTextures(1, &gAlbedoSpec);
    glDeleteTextures(1, &gDepth);
    glDeleteFramebuffers(1, &gBuffer);

    glDeleteTextures(1, &lighting);
    glDeleteFramebuffers(1, &lightingfbo);
    glDeleteTextures(1, &blurHV);
    glDeleteFramebuffers(1, &blurfbo);
    glDeleteTextures(1, &bloomfbo);
    glDeleteFramebuffers(1, &bloom);
    glDeleteTextures(1, &finalBloom);
    glDeleteFramebuffers(1, &finalBloomfbo);
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

    // Depth
    GL->glGenTextures(1, &depthTexture);
    GL->glBindTexture(GL_TEXTURE_2D, depthTexture);
    GL->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    GL->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    GL->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    GL->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    GL->glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);


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

    glGenTextures(1, &lighting);
    glBindTexture(GL_TEXTURE_2D, lighting);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    // Frame Buffer
    glGenFramebuffers(1, &lightingfbo);
    glBindFramebuffer(GL_FRAMEBUFFER, lightingfbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, lighting, 0);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);


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
    GL->glDrawBuffer(GL_COLOR_ATTACHMENT0);


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
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
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

                glActiveTexture(GL_TEXTURE0);

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
}*/

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

void MyOpenGLWidget::RenderCube()
{
    if (cubeVAO == 0)
    {
        float vertices[] = {
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
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}

