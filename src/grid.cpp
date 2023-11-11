#include "grid.h"

GridMap::GridMap(int sWidth, int sHeight, int cWidth, int cHeight) {
    cellSize = {cWidth, cHeight};
    size = olc::vi2d(sWidth, sHeight) / cellSize;
}

GridMap::~GridMap() {
    entities.clear();
    garbage.clear();
}

std::shared_ptr<Bot> GridMap::CreateBot(olc::vi2d position) {
    auto bot = std::make_shared<Bot>(position);
    entities.push_back(bot);
    return bot;
}

void GridMap::DestroyEntity(std::shared_ptr<Entity> entity) {
    DestroyEntity(entity.get());
}

void GridMap::DestroyEntity(Entity* entity) {
    if(std::find_if(garbage.begin(), garbage.end(), [&](const auto& e){ return e.get() == entity; }) != garbage.end()) return;
    auto it = std::find_if(entities.begin(), entities.end(), [&](const auto& e){ return e.get() == entity; });
    if(it == entities.end()) return;

    garbage.emplace_back(std::move(*it));
}

std::shared_ptr<Entity> GridMap::GetEntity(const olc::vi2d& pos) {
    auto it = std::find_if(entities.begin(), entities.end(), [&](const auto& ent){ return ent && ent->position == pos; });

    if(it == entities.end()) return nullptr;
    return *it;
}

std::vector<std::shared_ptr<Entity>> GridMap::GetEntities(const olc::vi2d& pos) {
    std::vector<std::shared_ptr<Entity>> rval;
    for(const auto &ent : entities){
        if(ent && ent->position == pos){
            rval.emplace_back(ent);
        }
    }
    return rval;
}

void GridMap::Update(float delta) {
    for(auto& obj : entities){
        if(!obj) continue;
        obj->Update(delta);
    }
}

void GridMap::Draw() {
    auto& game = Game::Current();

    // draw basic grid
    for(int y = 0; y < size.y; y++){
        for(int x = 0; x < size.x; x++){
            game.DrawRect(olc::vi2d(x,y) * cellSize, cellSize, 0x80FFFFFF);
        }
    }

    // draw entites
    for(auto& obj : entities){
        if(!obj) continue;
        obj->Draw();
    }
}

void GridMap::GarbageCollection() {
    size_t len = garbage.size();
    if(len){
        do {
            entities.erase(std::find(entities.begin(), entities.end(), nullptr));
        } while(--len);
        garbage.clear();
    }
}