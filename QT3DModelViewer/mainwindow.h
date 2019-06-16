#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <iostream>
#include <QTreeWidget>
#include "ui_inspector.h"
#include "gameobject.h"

class Resources;
class Scene;

namespace Ui {
class MainWindow;
class Inspector;
}


// class Scene;
class MyOpenGLWidget;


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    void reloadHierarchy();
    
protected:

private:
    Ui::MainWindow *ui;
    Ui::Inspector *uiInspector;

    MyOpenGLWidget* myOpenGLWidget = nullptr;

    Resources* resources= nullptr;
    Scene* scene= nullptr;
    GameObject* selectedGo = nullptr;

public slots:
    void loadModel();

    void shaderDiffuse();
    void shaderVertexPosition();
    void shaderVertexNormal();
    void shaderVertexTextureCoords();
    void shaderBitangents();
    void shaderTangents();
    void shaderDepth();
    void shaderLinearDepth();
    void shaderReflection();
    void shaderRefraction();
    void shaderDeferred();

    void openReadme();
    void reloadInspector();

    void changeActiveGo();

    void changePositionX();
    void changePositionY();
    void changePositionZ();

    void changeRotationX();
    void changeRotationY();
    void changeRotationZ();

    void changeScaleX();
    void changeScaleY();
    void changeScaleZ();
};

#endif // MAINWINDOW_H
