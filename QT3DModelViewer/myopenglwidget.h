#ifndef MYOPENGLWIDGET_H
#define MYOPENGLWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QMatrix4x4>

class Scene;
class Resources;
class Mesh;

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

protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int width, int height) override;
    /*void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;*/

public:
    // Scene
    Scene* scene = nullptr;
    Resources* resources = nullptr;

private:

    bool m_core;

    // camera
    QMatrix4x4 m_proj;
    QMatrix4x4 m_camera;

    int program_index = -1;
};

#endif // MYOPENGLWIDGET_H
