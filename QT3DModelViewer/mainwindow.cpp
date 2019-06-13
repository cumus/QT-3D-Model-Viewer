#include "mainwindow.h"

#include "ui_mainwindow.h"
#include "ui_inspector.h"
#include "myopenglwidget.h"
#include "scene.h"

#include <QtWidgets>
#include <QtGui>
#include <QFileDialog>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    uiInspector(new Ui::Inspector)
{
    ui->setupUi(this);

    setFocusPolicy(Qt::ClickFocus);

    myOpenGLWidget = new MyOpenGLWidget(ui->openGLWidget);
    myOpenGLWidget->scene = scene = new Scene();
    myOpenGLWidget->scene->mainWindow = this;

    //Inspector

    QWidget *inspectorWidget = new QWidget();
    uiInspector->setupUi(inspectorWidget);
    inspectorWidget->show();

    ui->inspectorDock->setWidget(inspectorWidget);

    //Menu Bar Connexions

    connect(ui->actionLoadModel, SIGNAL(triggered()), this, SLOT(loadModel()));
    connect(ui->actionReadme, SIGNAL(triggered()), this, SLOT(openReadme()));
    connect(ui->actionExit, SIGNAL(triggered()), qApp, SLOT(quit()));
}

MainWindow::~MainWindow()
{
    delete myOpenGLWidget;
    delete resources;
    delete scene;
    delete ui;
}

void MainWindow::reloadHierarchy()
{
    ui->hierarchy->clear();

    for (int i = 0;i < scene->root->childs.size();i++)
    {
        QListWidgetItem *item = new QListWidgetItem(scene->root->childs[i]->name);
        item->setData(Qt::UserRole,scene->root->childs[i]->id);
        ui->hierarchy->addItem(item);
    }
}

void MainWindow::loadModel()
{
    QString file = QFileDialog::getOpenFileName(this,"Select model to load", qApp->applicationDirPath(), "OBJ File (*.obj)");
    qDebug() << file;

    scene->newModelPath = file;
    scene->loadNewModel = true;
}

void MainWindow::openReadme()
{
    QDesktopServices::openUrl(QUrl("https://github.com/cumus/QT-3D-Model-Viewer"));
}

