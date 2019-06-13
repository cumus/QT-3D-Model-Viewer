#include "mainwindow.h"

#include "ui_mainwindow.h"
#include "myopenglwidget.h"
#include "scene.h"

#include <QtWidgets>
#include <QtGui>
#include <QFileDialog>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setFocusPolicy(Qt::ClickFocus);

    myOpenGLWidget = new MyOpenGLWidget(ui->openGLWidget);
    myOpenGLWidget->scene = scene = new Scene();

    //HierarchyButtons

    connect(ui->loadMeshButton, SIGNAL(clicked()), this, SLOT(addNodeHierarchyTree()));

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

void MainWindow::addChild(GameObject *node)
{
    QVector<GameObject*>::iterator child = node->childs.begin();
    for (; child != node->childs.end(); child++)
    {
        QTreeWidgetItem *treeItem = new QTreeWidgetItem();
        treeItem->setText(0, (*child)->name);
        (*child)->hierarchyItem = treeItem;
        node->hierarchyItem->addChild(treeItem);

        addChild((*child));
    }
}

void MainWindow::addNodeHierarchyTree()
{
    ui->hierarchy->clear();

    QTreeWidgetItem *treeItem = new QTreeWidgetItem(ui->hierarchy);
    treeItem->setText(0,scene->root->name);
    scene->root->hierarchyItem = treeItem;
    ui->hierarchy->addTopLevelItem(treeItem);
    addChild(scene->root);
}

