#include "mainwindow.h"

#include "ui_mainwindow.h"
#include "myopenglwidget.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
    // uiHierarchy(new Ui::Hierarchy)
{
    ui->setupUi(this);
    myOpenGLWidget = new MyOpenGLWidget(ui->openGLWidget);


    /*/ Hierarchy
    QWidget *hierarchyWidget = new QWidget();
    uiHierarchy->setupUi(hierarchyWidget);
    hierarchyWidget->show();
    ui->hierarchyDock->setWidget(hierarchyWidget);*/

    /*/ Render Settings
    QWidget *renderingWidget = new QWidget();
    uiRendering->setupUi(renderingWidget);
    //renderingWidget->show();
    ui->renderDock->setWidget(renderingWidget);*/

    /*inspector = new Inspector(this);
    ui->inspectorDock->setWidget(inspector);*/

    /*/QMainWindow::tabifyDockWidget(ui->inspectorDock,ui->renderDock);
    //ui->inspectorDock->show();
    inspector->show();*/

    /*/ Scene
    ui->openGLWidget->scene = scene = new Scene(this);
    newScene();
    setWindowTitle(scene->name);*/

    /*QColor c = scene->background_color;
    uiHierarchy->backGroundColorButton->setStyleSheet(QString("background-color: rgb(%1, %2, %3)").arg(c.red()).arg(c.green()).arg(c.blue()));*/

    /*/Hierarchy Connexions
    connect(uiHierarchy->addEntityButton, SIGNAL(clicked()), this, SLOT(addEntityButtonClicked()));
    connect(uiHierarchy->removeEntityButton, SIGNAL(clicked()), this, SLOT(removeEntityButtonClicked()));

    connect(uiHierarchy->listWidget, SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem *)), this, SLOT(changeSelectedGemaObject()));*/

    //Menu Bar Connexions
    /*connect(ui->actionNewScene, SIGNAL(triggered()), this, SLOT(newScene()));
    connect(ui->actionOpenScene, SIGNAL(triggered()), this, SLOT(openScene()));
    connect(ui->actionSaveScene, SIGNAL(triggered()), this, SLOT(saveScene()));
    connect(ui->actionReadme, SIGNAL(triggered()), this, SLOT(openReadme()));*/
    connect(ui->actionExit, SIGNAL(triggered()), qApp, SLOT(quit()));

    //connect(uiHierarchy->backGroundColorButton, SIGNAL(clicked()), this, SLOT(changeBackGroundColor()));
}

MainWindow::~MainWindow()
{
    delete ui;
}
