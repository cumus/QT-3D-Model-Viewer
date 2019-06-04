#include "mesh.h"
#include "gameobject.h"
#include "myopenglwidget.h"
#include <qmath.h>

Mesh::Mesh(GameObject* go, bool isActive) :
    Component(MESH, go, isActive),
    vbo(QOpenGLBuffer::VertexBuffer),
    nbo(QOpenGLBuffer::VertexBuffer),
    tbo(QOpenGLBuffer::VertexBuffer),
    ibo(QOpenGLBuffer::IndexBuffer),
    num_vertices(0),
    num_faces(0)
{
    if (go != nullptr)
        go->components.push_back(this);

    vertex_data.clear();
    normal_data.clear();
    texcoord_data.clear();
    index_data.clear();
}

Mesh::~Mesh()
{
    CleanUp();
}

void Mesh::Draw(MyOpenGLWidget* renderer)
{
    renderer->DrawMesh(this);
}

void Mesh::Save(QDataStream &stream) {}
void Mesh::Load(QDataStream &stream) {}

void Mesh::CleanUp()
{
    vertex_data.clear();
    normal_data.clear();
    texcoord_data.clear();
    index_data.clear();
}
