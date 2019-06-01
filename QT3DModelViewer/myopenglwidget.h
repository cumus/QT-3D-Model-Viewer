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

public:

    Scene* scene = nullptr;
    Resources* resources = nullptr;

private:

    int tick_count = 0;
    int program_index = -1;

    Camera cam;
    QPoint mouse_pos;

    int m_mvMatrixLoc;
    int m_normalMatrixLoc;
    int m_lightPosLoc;
};

#endif // MYOPENGLWIDGET_H
