#ifndef MYOPENGLWIDGET_H
#define MYOPENGLWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QMatrix4x4>
#include <QPoint>
#include <QVector3D>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QOpenGLFunctions>
QT_FORWARD_DECLARE_CLASS(QOpenGLShaderProgram)

class Scene;
class Resources;
class Mesh;
class Transform;
class QTimer;
class QOpenGLTexture;
class SubMesh;

struct Camera
{
    Transform* transform;
    QMatrix4x4 m_proj;
    int m_projMatrixLoc;
};

struct Light
{
    QVector3D Position;
    QVector3D Color;
    float Intensity;
    float Radius;
    int TypeLight; // 0 Directional, 1 PointLight
};

class MyOpenGLWidget : public QOpenGLWidget, protected QOpenGLFunctions_3_3_Core
{
    Q_OBJECT
public:
    explicit MyOpenGLWidget(QWidget *parent = nullptr);
    ~MyOpenGLWidget() override;

    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;

    void Tick();

    void DrawMesh(Mesh* mesh = nullptr);
    void LoadMesh(SubMesh* mesh = nullptr);

protected:

    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int width, int height) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

public:

    Scene* scene = nullptr;

private:

    // Time control
    QTimer *timer = nullptr;
    int tick_count = 0;
    float tick_period = 3.0f;

    int width, height;

    // Camera
    Camera cam;
    QPointF mouse_pos;
    bool cam_dir[6];

    QVector3D lightPos;
    QVector3D lightColor;

    // Shaders
    QVector<QOpenGLShaderProgram*> programs;

    // Shader uniforms
    int m_mvMatrixLoc;
    int m_normalMatrixLoc;
    int m_lightPosLoc;
    int m_lightIntensityLoc;
    int m_textureLoc;
    int m_modeLoc;

    float mode = 0;

    // Lights
    QList<Light> lights;

public:

    void InitDeferredRenderer();
    void DeleteBuffers();
    void ResizeS(int width,int height);

    void Render();
    void RenderQuad();

    int renderView = 0;
    unsigned int gBuffer; //fbo
    unsigned int gPosition, gNormal, gAlbedoSpec;
    unsigned int rboDepth; //rbo

    unsigned int attachments[3];

    unsigned int quadVAO = 0;
    unsigned int quadVBO;
};

#endif // MYOPENGLWIDGET_H
