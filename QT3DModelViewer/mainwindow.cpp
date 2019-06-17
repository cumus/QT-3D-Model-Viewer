#include "mainwindow.h"

#include "ui_mainwindow.h"
#include "ui_inspector.h"
#include "myopenglwidget.h"
#include "scene.h"
#include "transform.h"

#include <QtWidgets>
#include <QtGui>
#include <QFileDialog>
#include <QMessageBox>
#include <QVector3D>

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
    ui->inspectorDock->setVisible(false);

    ui->inspectorDock->setWidget(inspectorWidget);

    //Menu Bar Connexions

    connect(ui->actionLoadModel, SIGNAL(triggered()), this, SLOT(loadModel()));
    connect(ui->actionReadme, SIGNAL(triggered()), this, SLOT(openReadme()));
    connect(ui->actionExit, SIGNAL(triggered()), qApp, SLOT(quit()));

    connect(ui->actionDiffuse_Texture, SIGNAL(triggered()), this, SLOT(shaderDiffuse()));
    connect(ui->actionVertex_Position, SIGNAL(triggered()), this, SLOT(shaderVertexPosition()));
    connect(ui->actionVertex_Normal, SIGNAL(triggered()), this, SLOT(shaderVertexNormal()));
    connect(ui->actionVertex_Texture_Coord, SIGNAL(triggered()), this, SLOT(shaderVertexTextureCoords()));
    connect(ui->actionBitangents, SIGNAL(triggered()), this, SLOT(shaderBitangents()));
    connect(ui->actionTangents, SIGNAL(triggered()), this, SLOT(shaderTangents()));
    connect(ui->actionDepth, SIGNAL(triggered()), this, SLOT(shaderDepth()));
    connect(ui->actionLinear_Depth, SIGNAL(triggered()), this, SLOT(shaderLinearDepth()));
    connect(ui->actionReflection, SIGNAL(triggered()), this, SLOT(shaderReflection()));
    connect(ui->actionRefraction, SIGNAL(triggered()), this, SLOT(shaderRefraction()));
    connect(ui->actionDeferred_Shading, SIGNAL(triggered()), this, SLOT(shaderDeferred()));
    connect(ui->actionDraw_Selected_GO_Borders, SIGNAL(triggered()), this, SLOT(drawBorders()));

    connect(ui->actionRender_Skybox, SIGNAL(triggered()), this, SLOT(renderSky()));
    connect(ui->actionRed_Mountain_Sunset, SIGNAL(triggered()), this, SLOT(skybox0()));
    connect(ui->actionClear_Lake, SIGNAL(triggered()), this, SLOT(skybox1()));

    connect(ui->hierarchy, SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem *)),this, SLOT(reloadInspector()));

    //Inspector connections
    connect(uiInspector->activeBox, SIGNAL(clicked()), this, SLOT(changeActiveGo()));

    connect(uiInspector->positionX, SIGNAL(valueChanged(double)), this, SLOT(changePositionX()));
    connect(uiInspector->positionY, SIGNAL(valueChanged(double)), this, SLOT(changePositionY()));
    connect(uiInspector->positionZ, SIGNAL(valueChanged(double)), this, SLOT(changePositionZ()));

    connect(uiInspector->rotationX, SIGNAL(valueChanged(double)), this, SLOT(changeRotationX()));
    connect(uiInspector->rotationY, SIGNAL(valueChanged(double)), this, SLOT(changeRotationY()));
    connect(uiInspector->rotationZ, SIGNAL(valueChanged(double)), this, SLOT(changeRotationZ()));

    connect(uiInspector->scaleX, SIGNAL(valueChanged(double)), this, SLOT(changeScaleX()));
    connect(uiInspector->scaleY, SIGNAL(valueChanged(double)), this, SLOT(changeScaleY()));
    connect(uiInspector->scaleZ, SIGNAL(valueChanged(double)), this, SLOT(changeScaleZ()));

    connect(uiInspector->refractiveIndex, SIGNAL(valueChanged(double)), this, SLOT(changeRefractiveIndex()));
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

    if(selectedGo != nullptr)
    {
        QVector<Component*>::iterator comp = selectedGo->components.begin();
        for (; comp != selectedGo->components.end(); comp++)
        {
            if ((*comp)->type == MESH)
                static_cast<Mesh*>(*comp)->draw_border = false;
        }
    }

    if(file != "")
    {
        scene->newModelPath = file;
        scene->loadNewModel = true;
    }
}

void MainWindow::shaderDiffuse()
{
    myOpenGLWidget->mode = 0;
}

void MainWindow::shaderVertexPosition()
{
    myOpenGLWidget->mode = 1;
}

void MainWindow::shaderVertexNormal()
{
    myOpenGLWidget->mode = 2;
}

void MainWindow::shaderVertexTextureCoords()
{
    myOpenGLWidget->mode = 3;
}

void MainWindow::shaderBitangents()
{
    myOpenGLWidget->mode = 4;
}

void MainWindow::shaderTangents()
{
    myOpenGLWidget->mode = 5;
}

void MainWindow::shaderDepth()
{
    myOpenGLWidget->mode = 6;
}

void MainWindow::shaderLinearDepth()
{
    myOpenGLWidget->mode = 7;
}

void MainWindow::shaderReflection()
{
    myOpenGLWidget->mode = 8;
}

void MainWindow::shaderRefraction()
{
    myOpenGLWidget->mode = 9;
}

void MainWindow::shaderDeferred()
{
    myOpenGLWidget->use_deferred = !myOpenGLWidget->use_deferred;
}

void MainWindow::drawBorders()
{
    myOpenGLWidget->draw_borders = !myOpenGLWidget->draw_borders;
}

void MainWindow::renderSky()
{
    myOpenGLWidget->renderSkybox = !myOpenGLWidget->renderSkybox;
}

void MainWindow::skybox0()
{
    myOpenGLWidget->current_skybox = 0;
}

void MainWindow::skybox1()
{
    myOpenGLWidget->current_skybox = 1;
}

void MainWindow::openReadme()
{
    QDesktopServices::openUrl(QUrl("https://github.com/cumus/QT-3D-Model-Viewer"));
}

void MainWindow::reloadInspector()
{
    if(selectedGo != nullptr)
    {
        QVector<Component*>::iterator comp = selectedGo->components.begin();
        for (; comp != selectedGo->components.end(); comp++)
        {
            if ((*comp)->type == MESH)
                static_cast<Mesh*>(*comp)->draw_border = false;
        }
    }

    if(ui->hierarchy->currentItem() == nullptr)
    {
        ui->inspectorDock->setVisible(false);

        selectedGo = nullptr;
    }
    else
    {
        ui->inspectorDock->setVisible(true);

        QVariant v = ui->hierarchy->currentItem()->data(Qt::UserRole);
        int id = v.value<int>();

        for (int i = 0; i < scene->root->childs.size(); i++)
        {
            if(scene->root->childs[i]->id == id)
            {
                //qDebug()<<"ROLID:" << id;
                selectedGo = scene->root->childs[i];
                myOpenGLWidget->cam_focus = selectedGo->transform->GetPos();

                //GameObject
                uiInspector->activeBox->setChecked(scene->root->childs[i]->transform->isActive);
                uiInspector->goNameLabel->setText(scene->root->childs[i]->name);
                uiInspector->goIdLabel->setText(QString::number(scene->root->childs[i]->id));

                //Position
                uiInspector->positionX->setValue(static_cast<double>(scene->root->childs[i]->transform->GetPos().x()));
                uiInspector->positionY->setValue(static_cast<double>(scene->root->childs[i]->transform->GetPos().y()));
                uiInspector->positionZ->setValue(static_cast<double>(scene->root->childs[i]->transform->GetPos().z()));

                //Rotation
                uiInspector->rotationX->setValue(static_cast<double>(scene->root->childs[i]->transform->GetRot().x()));
                uiInspector->rotationY->setValue(static_cast<double>(scene->root->childs[i]->transform->GetRot().y()));
                uiInspector->rotationZ->setValue(static_cast<double>(scene->root->childs[i]->transform->GetRot().z()));

                //Scale
                uiInspector->scaleX->setValue(static_cast<double>(scene->root->childs[i]->transform->GetScale().x()));
                uiInspector->scaleY->setValue(static_cast<double>(scene->root->childs[i]->transform->GetScale().y()));
                uiInspector->scaleZ->setValue(static_cast<double>(scene->root->childs[i]->transform->GetScale().z()));

                if(selectedGo != nullptr)
                {
                    QVector<Component*>::iterator comp = selectedGo->components.begin();
                    for (; comp != selectedGo->components.end(); comp++)
                    {
                        if ((*comp)->type == MESH)
                        {
                            static_cast<Mesh*>(*comp)->draw_border = true;
                            uiInspector->refractiveIndex->setValue(static_cast<double>(static_cast<Mesh*>(*comp)->refraction_index));
                        }
                    }
                }

                return;
            }
        }
    }
}

void MainWindow::changeActiveGo()
{
    selectedGo->transform->isActive = uiInspector->activeBox->isChecked();
}

void MainWindow::changePositionX()
{
    selectedGo->transform->SetPos(QVector3D(static_cast<float>(uiInspector->positionX->value()),selectedGo->transform->GetPos().y(),selectedGo->transform->GetPos().z()));
}

void MainWindow::changePositionY()
{
    selectedGo->transform->SetPos(QVector3D(selectedGo->transform->GetPos().x(),static_cast<float>(uiInspector->positionY->value()),selectedGo->transform->GetPos().z()));
}

void MainWindow::changePositionZ()
{
    selectedGo->transform->SetPos(QVector3D(selectedGo->transform->GetPos().x(),selectedGo->transform->GetPos().y(),static_cast<float>(uiInspector->positionZ->value())));
}

void MainWindow::changeRotationX()
{
    selectedGo->transform->SetRotXYZ(QVector3D(static_cast<float>(uiInspector->rotationX->value()),selectedGo->transform->GetRot().y(),selectedGo->transform->GetRot().z()));
}

void MainWindow::changeRotationY()
{
    selectedGo->transform->SetRotXYZ(QVector3D(selectedGo->transform->GetRot().x(),static_cast<float>(uiInspector->rotationY->value()),selectedGo->transform->GetRot().z()));
}

void MainWindow::changeRotationZ()
{
    selectedGo->transform->SetRotXYZ(QVector3D(selectedGo->transform->GetRot().x(),selectedGo->transform->GetRot().y(),static_cast<float>(uiInspector->rotationZ->value())));
}

void MainWindow::changeScaleX()
{
    selectedGo->transform->SetScale(QVector3D(static_cast<float>(uiInspector->scaleX->value()),selectedGo->transform->GetScale().y(),selectedGo->transform->GetScale().z()));
}

void MainWindow::changeScaleY()
{
    selectedGo->transform->SetScale(QVector3D(selectedGo->transform->GetScale().x(),static_cast<float>(uiInspector->scaleY->value()),selectedGo->transform->GetScale().z()));
}

void MainWindow::changeScaleZ()
{
    selectedGo->transform->SetScale(QVector3D(selectedGo->transform->GetScale().x(),selectedGo->transform->GetScale().y(),static_cast<float>(uiInspector->scaleZ->value())));
}

void MainWindow::changeRefractiveIndex()
{
    if(selectedGo != nullptr)
    {
        QVector<Component*>::iterator comp = selectedGo->components.begin();
        for (; comp != selectedGo->components.end(); comp++)
        {
            if ((*comp)->type == MESH)
            {
                static_cast<Mesh*>(*comp)->refraction_index = static_cast<float>(uiInspector->refractiveIndex->value());
            }
        }
    }
}
