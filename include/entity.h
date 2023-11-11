#pragma once
#include "forward.h"
#include "entity_type.h"
#include "olcPixelGameEngine.h"

class Entity {
protected:
    inline Entity(EntityTypes::EntityType ty, olc::vi2d pos): type(ty), position(pos) {}
public:
    const EntityTypes::EntityType type;

    olc::vi2d position;

    virtual void Update(float delta);
    virtual void Draw();
};