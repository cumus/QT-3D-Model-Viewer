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
        Assimp/include/assimp/Compiler/poppack1.h \
        Assimp/include/assimp/Compiler/pstdint.h \
        Assimp/include/assimp/Compiler/pushpack1.h \
        Assimp/include/assimp/DefaultIOStream.h \
        Assimp/include/assimp/DefaultIOSystem.h \
        Assimp/include/assimp/DefaultLogger.hpp \
        Assimp/include/assimp/Defines.h \
        Assimp/include/assimp/Exporter.hpp \
        Assimp/include/assimp/IOStream.hpp \
        Assimp/include/assimp/IOSystem.hpp \
        Assimp/include/assimp/Importer.hpp \
        Assimp/include/assimp/LogStream.hpp \
        Assimp/include/assimp/Logger.hpp \
        Assimp/include/assimp/NullLogger.hpp \
        Assimp/include/assimp/ProgressHandler.hpp \
        Assimp/include/assimp/SceneCombiner.h \
        Assimp/include/assimp/ai_assert.h \
        Assimp/include/assimp/anim.h \
        Assimp/include/assimp/camera.h \
        Assimp/include/assimp/cexport.h \
        Assimp/include/assimp/cfileio.h \
        Assimp/include/assimp/cimport.h \
        Assimp/include/assimp/color4.h \
        Assimp/include/assimp/color4.inl \
        Assimp/include/assimp/config.h \
        Assimp/include/assimp/defs.h \
        Assimp/include/assimp/importerdesc.h \
        Assimp/include/assimp/light.h \
        Assimp/include/assimp/material.h \
        Assimp/include/assimp/material.inl \
        Assimp/include/assimp/matrix3x3.h \
        Assimp/include/assimp/matrix3x3.inl \
        Assimp/include/assimp/matrix4x4.h \
        Assimp/include/assimp/matrix4x4.inl \
        Assimp/include/assimp/mesh.h \
        Assimp/include/assimp/metadata.h \
        Assimp/include/assimp/postprocess.h \
        Assimp/include/assimp/quaternion.h \
        Assimp/include/assimp/quaternion.inl \
        Assimp/include/assimp/scene.h \
        Assimp/include/assimp/texture.h \
        Assimp/include/assimp/types.h \
        Assimp/include/assimp/vector2.h \
        Assimp/include/assimp/vector2.inl \
        Assimp/include/assimp/vector3.h \
        Assimp/include/assimp/vector3.inl \
        Assimp/include/assimp/version.h \
        component.h \
        gameobject.h \
        mainwindow.h \
        mesh.h \
        myopenglwidget.h \
        scene.h \
        transform.h

FORMS += \
        mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    icons.qrc

DISTFILES += \
    Assimp/lib/assimp.dll \
    Assimp/lib/assimp.lib \
    Assimp/lib/assimpd.dll \
    Assimp/lib/assimpd.lib \
    Models/Patrick/Color.png \
    Models/Patrick/Flowers.png \
    Models/Patrick/Patrick.mtl \
    Models/Patrick/Skin_Patrick.png \
    Resources/icons/Badge-multiply.png \
    Resources/icons/Disquette.png \
    Resources/icons/Doc.png \
    Resources/icons/Document-new.png \
    Resources/icons/Folder.png \
    Resources/icons/Info.png \
    Shaders/default.frag \
    Shaders/default.vert \
    Shaders/deferred_light.frag \
    Shaders/deferred_light.vert \
    Shaders/deferred_shading.frag \
    Shaders/deferred_shading.vert \
    Shaders/graphic_buffer.frag \
    Shaders/graphic_buffer.vert \
    default.frag \
    default.vert \
    deferred_light.frag \
    deferred_light.vert \
    deferred_shading.frag \
    deferred_shading.vert \
    graphic_buffer.frag \
    graphic_buffer.vert

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/Assimp/lib/ -lassimp
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/Assimp/lib/ -lassimpd
else:unix: LIBS += -L$$PWD/Assimp/lib/ -lassimp

INCLUDEPATH += $$PWD/Assimp/include
DEPENDPATH += $$PWD/Assimp/include
