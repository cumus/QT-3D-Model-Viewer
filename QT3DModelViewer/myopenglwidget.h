#ifndef MYOPENGLWIDGET_H
#define MYOPENGLWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QMatrix4x4>
#include <QPoint>

class Scene;
class Resources;
class Mesh;
class Transform;
class QTimer;

struct Camera
{
    Transform* transform;
    QMatrix4x4 m_proj;
    int m_projMatrixLoc;
};

class MyOpenGLWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT
public:
    explicit MyOpenGLWidget(QWidget *parent = nullptr);
    ~MyOpenGLWidget() override;

    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;

    void Tick();

    void DrawMesh(Mesh* mesh = nullptr);
    void LoadMesh(Mesh* mesh = nullptr);

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
    Resources* resources = nullptr;

private:

    QTimer *timer = nullptr;
    int tick_count = 0;
    float tick_period = 3.0f;
    int program_index = -1;

    Camera cam;
    QPointF mouse_pos;
    bool cam_dir[6];

    int m_mvMatrixLoc;
    int m_normalMatrixLoc;
    int m_lightPosLoc;
};

#endif // MYOPENGLWIDGET_H
