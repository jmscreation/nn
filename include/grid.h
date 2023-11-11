#pragma once
#include "forward.h"

#include "olcPixelGameEngine.h"
#include "game.h"
#include "entity.h"
#include "bot.h"

#include <algorithm>
#include <memory>

class GridMap {
    std::vector<std::shared_ptr<Entity>> entities, garbage;
public:
    olc::vi2d size, cellSize;

    GridMap(int sWidth, int sHeight, int cWidth = 32, int cHeight = 32);
    virtual ~GridMap();

    std::shared_ptr<Bot> CreateBot(olc::vi2d position);

    void DestroyEntity(std::shared_ptr<Entity> entity);
    void DestroyEntity(Entity* entity);
    std::shared_ptr<Entity> GetEntity(const olc::vi2d& pos);
    std::vector<std::shared_ptr<Entity>> GetEntities(const olc::vi2d& pos);

    void Update(float delta);
    void Draw();
    void GarbageCollection();
};