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
#include <QOpenGLTexture>
#include <QOpenGLFunctions>
#include <QWindow>
#include <QVector3D>

QT_FORWARD_DECLARE_CLASS(QOpenGLShaderProgram)
QT_FORWARD_DECLARE_CLASS(QOpenGLContext)
QT_FORWARD_DECLARE_CLASS(QOpenGLFramebufferObject)
QT_FORWARD_DECLARE_CLASS(QOffscreenSurface)

class Scene;
class Resources;
class Mesh;
class Transform;
class QTimer;
class QOpenGLTexture;
class SubMesh;

enum SHADER_TYPE : int
{
    DEFAULT = 0,
    SINGLE_COLOR,
    FRAMEBUFFER_TO_SCREEN,
    GRAPHIC_BUFFER,
    DEFERRED_SHADING,
    DEFERRED_LIGHT
};

enum RENDER_STATE : int
{
    INITIALIZING,
    INITIALIZED,
    MODELS,
    BORDERED_MODELS,
    BORDERS,
    POST_PROCESSING,
    FINISHED
};

struct Camera
{
    Transform* transform;
    QMatrix4x4 m_proj;
};

struct Light
{
    bool isActive;
    QVector3D Position;
    QVector3D Color;
    float range;
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

    void DrawMesh(Mesh* mesh = nullptr, SHADER_TYPE shader = DEFAULT);
    void LoadSubMesh(SubMesh* mesh = nullptr);

    void ResetLights();
    void RandomizeLights(float range, QVector3D pos_range, QVector3D offset = {0,0,0}, QVector3D min_color = {0,0,0});

protected:

    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int width, int height) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;

private:

    void RenderQuad();
    void RenderCube();

public:

    Scene* scene = nullptr;

    Camera cam;

    // Light
    QList<Light> lights;
    bool camera_light_follow = true;

    // Deferred Rendering
    bool use_deferred = false;

    // Stencil Border
    float border_scale = 1.1f;
    QVector3D border_color;
    float border_alpha = 0.8f;
    bool border_over_borderless = true; // border depth test

    // Shaders
    QVector<QOpenGLShaderProgram*> programs;
    float mode = 0;

private:

    RENDER_STATE state;

    // Camera
    QPointF mouse_pos;
    bool cam_dir[6];

    // Time control
    QTimer *timer = nullptr;
    int tick_count = 0;
    float tick_period = 3.0f;

    // Context dimensions
    int width, height;

    // Shader uniforms
    // Default
    int d_mvMatrixLoc;
    int d_normalMatrixLoc;
    int d_projMatrixLoc;
    int d_lightPosLoc;
    int d_lightIntensityLoc;
    int d_textureLoc;
    int d_modeLoc;
    // Single Color
    int sc_modelView;
    int sc_proj;
    int sc_color;
    int sc_alpha;
    // Framebuffer to screen
    int fs_screenTexture;
    /*/ Graphic Buffer
    int gb_model;
    int gb_modelInv;
    int gb_view;
    int gb_projection;*/
    // Deferred Shading
    // Lightning Pass
    int def_posLoc;
    int def_normalLoc;
    int def_albedospecLoc;

    // Framebuffer
    unsigned int fbo;
    //unsigned int textureColorbuffer;
    unsigned int rboDepth;

    // Border stack vector
    QVector<Mesh*> border_meshes;

    unsigned int quadVAO = 0;
    unsigned int quadVBO;
    unsigned int cubeVAO = 0;
    unsigned int cubeVBO;
    unsigned int gPosition, gNormal, gAlbedoSpec;

    const unsigned int NR_LIGHTS = 32;
};

#endif // MYOPENGLWIDGET_H
