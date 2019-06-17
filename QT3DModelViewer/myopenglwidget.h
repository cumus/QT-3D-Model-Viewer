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
    DEFERRED_LIGHT,
    SKYBOX
};

enum RENDER_STATE : int
{
    WIDGET_CREATED,
    INITIALIZING,
    INITIALIZED,
    PREPARING_TO_DRAW,
    MODELS,
    BORDERED_MODELS,
    BORDERS,
    POST_PROCESSING,
    FINISHED
};

struct Light
{
    bool isActive;
    QVector3D Position;
    QVector3D Color;
    float Intensity;
    float MinRange;
    float Constant;
    float Linear;
    float Quadratic;

    // dependent values
    float radius;
    float maxBrightness;
    bool updated;
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
    void AddLightAtCamPos();
    void RandomizeLights(float range, QVector3D pos_range, QVector3D offset = {0,0,0}, QVector3D min_color = {0,0,0});

protected:

    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int width, int height) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private:

    void RenderQuad();
    void RenderCube();
    void RenderSkybox();
    int LoadSkyboxes(const QVector<QString> paths);
    void LoadShaders();
    void LoadFramebuffer();
    void DrawBordered();
    void PostProcessDeferredLights();

public:

    // Scene
    Scene* scene = nullptr;

    // Camera
    Transform* camera = nullptr;
    QVector3D cam_focus;
    float zoom = 45.0f;

    // Light
    QList<Light> lights;
    bool camera_light_follow = true;
    int current_light = 1;

    // Rendering Options
    bool use_deferred = false;
    bool draw_borders = true;
    float mode = 8;
    int current_skybox = 0;

    // Stencil Border
    float border_scale = 1.1f;
    QVector3D border_color;
    float border_alpha = 0.8f;
    bool border_over_borderless = true; // border depth test

    bool renderSkybox = true;

private:

    // Shaders
    QVector<QOpenGLShaderProgram*> programs;

    // Render state
    RENDER_STATE state;

    // Camera
    QPointF mouse_pos;
    bool cam_dir[6];
    bool orbiting = false;

    // Time control
    QTimer *timer = nullptr;
    int tick_count = 0;
    float tick_period = 3.0f;

    // Context dimensions
    int width, height;

    // Framebuffer
    unsigned int fbo;
    unsigned int gPosition, gNormal, gAlbedoSpec;
    unsigned int rboDepth;

    // Screen Quad
    unsigned int quadVAO = 0;
    unsigned int quadVBO;

    // Skybox
    QVector<QOpenGLTexture*> skyboxes;
    unsigned int skyboxVAO = 0;
    unsigned int skyboxVBO;

    // Ligh Cube
    unsigned int cubeVAO = 0;
    unsigned int cubeVBO;

    // Border stack vector
    QVector<Mesh*> border_meshes;

    // Shader uniform Locations
    QMatrix4x4 m_proj;
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
    // Graphic Buffer
    // Deferred Shading
    // Lightning Pass
    int def_posLoc;
    int def_normalLoc;
    int def_albedospecLoc;

    // Max Lights
    const int NR_LIGHTS = 32;
};

#endif // MYOPENGLWIDGET_H
