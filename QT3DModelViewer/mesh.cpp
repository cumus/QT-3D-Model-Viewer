#include "mesh.h"
#include "myopenglwidget.h"

Mesh::Mesh(GameObject* go, bool isActive) :
    Component(MESH, go, isActive)
{
    vao_index = -1;
    vbo_index = -1;
    texture_index = -1;
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
    // release resources
}
