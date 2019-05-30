#include "component.h"

Component::Component(ComponentTYPE type, GameObject* gameobject, bool isActive) :
    type(type),
    gameobject(gameobject),
    isActive(isActive)
{}

Component::~Component()
{}
