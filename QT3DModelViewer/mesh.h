#ifndef MESH_H
#define MESH_H

#include "component.h"

class Mesh : public Component
{
public:
    Mesh(GameObject* go = nullptr, bool isActive = true);
    ~Mesh() override;

    void Draw(MyOpenGLWidget* renderer = nullptr) override;
    void Save(QDataStream& stream) override;
    void Load(QDataStream& stream) override;
    void CleanUp() override;

public:

    int vao_index = -1;
    int vbo_index = -1;
    int texture_index = -1;
};

#endif // MODEL_H
