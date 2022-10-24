#-------------------------------------------------
#
# Project created by QtCreator 2019-05-29T18:58:22
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = QT3DModelViewer
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

SOURCES += \
        component.cpp \
        gameobject.cpp \
        main.cpp \
        mainwindow.cpp \
        mesh.cpp \
        myopenglwidget.cpp \
        scene.cpp \
        transform.cpp

HEADERS += \
        component.h \
        gameobject.h \
        mainwindow.h \
        mesh.h \
        myopenglwidget.h \
        scene.h \
        transform.h

FORMS += \
        inspector.ui \
        mainwindow.ui

RESOURCES += \
        Resources/Skyboxes/skyboxes.qrc \
        Resources/Shaders/shaders.qrc \
        Resources/Icons/icons.qrc

win32 {
    contains(QT_ARCH, i386) {
        CONFIG(debug, debug|release) {
            message("Building: Windows 32-bit Debug")
            LIBS += -L$$PWD/Assimp/libx86/Debug/ -lassimp-vc142-mtd
        }
        CONFIG(release, debug|release){
            message("Building: Windows 32-bit Release")
            LIBS += -L$$PWD/Assimp/libx86/MinSizeRel/ -lassimp-vc142-mt
        }
    } else {
        CONFIG(debug, debug|release) {
            message("Building: Windows 64-bit Debug")
            LIBS += -L$$PWD/Assimp/libx64/Debug/ -lassimp-vc142-mtd
        }
        CONFIG(release, debug|release){
            message("Building: Windows 64-bit Release")
            LIBS += -L$$PWD/Assimp/libx64/MinSizeRel/ -lassimp-vc142-mt
        }
    }
}
else:message("Viewer Error: Target platform unsupported")

INCLUDEPATH += $$PWD/Assimp/include
DEPENDPATH += $$PWD/Assimp/include

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
