#pragma once
#include "forward.h"

#include "olcPixelGameEngine.h"
#include "bot.h"
#include "entity.h"
#include "grid.h"

#include <string>
#include <map>
#include <vector>
#include <memory>
#include <cassert>


class Game : public olc::PixelGameEngine {
public:
    struct Command {
        std::function<std::string(Game& game, std::stringstream& out)> function;
    };

    std::weak_ptr<Bot> selectedBot;
    int updateInterval;
    bool displayScreen, verboseTraining;

private:
    static Game* _curGame;
    static const std::map<std::string, Command> commands;
public:
    bool running;

    Game();
    virtual ~Game();
    Game(const Game&)=delete;

    inline static Game& Current() { assert(_curGame != nullptr); return *_curGame; }

    std::unique_ptr<GridMap> map;

    bool OnUserCreate() override;
    bool OnUserUpdate(float delta) override;
    bool OnConsoleCommand(const std::string& command) override;
};