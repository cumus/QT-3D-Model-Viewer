#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <iostream>

namespace Ui {
class MainWindow;
// class Hierarchy;
// class Rendering;
}

// class Inspector;
// class Scene;
class MyOpenGLWidget;


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    
protected:
    //void paintEvent(QPaintEvent* event) override;

private:
    Ui::MainWindow *ui;
    /*Ui::Hierarchy *uiHierarchy;
    Ui::Rendering *uiRendering;
    Inspector* inspector;

    Scene* scene = nullptr;*/

    MyOpenGLWidget* myOpenGLWidget = nullptr;

public slots:
    /*void openScene();
    void saveScene();
    void newScene();
    void openReadme();

    void changeSelectedGameObject();

    void addEntityButtonClicked();
    void removeEntityButtonClicked();

    void changeBackGroundColor();*/
};

#endif // MAINWINDOW_H
