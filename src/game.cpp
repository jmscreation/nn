#include "game.h"

Game* Game::_curGame = nullptr;

const std::map<std::string, Game::Command> Game::commands {
    { "echo",
        {
            [](Game& game, std::stringstream& out) -> std::string {
                std::string echo;
                while(!out.eof()){
                    std::string _; out >> _;
                    if(echo.size()) echo += " ";
                    echo += _;
                }
                return echo;
            }
        }
    },
    { "cls",
        {
            [](Game& game, std::stringstream& out) -> std::string {
                game.ConsoleClear();
                return "";
            }
        }
    },
    { "exit",
        {
            [](Game& game, std::stringstream& out) -> std::string {
                game.running = false;
                return "user terminated application";
            }
        }
    },

    { "addbot",
        {
            [](Game& game, std::stringstream& out) -> std::string {
                int x, y;
                out >> x;
                out >> y;
                game.selectedBot = game.map->CreateBot({x,y});

                return "bot added";
            }
        }
    },
    { "selectbot",
        {
            [](Game& game, std::stringstream& out) -> std::string {
                int x, y;
                out >> x;
                out >> y;
                auto ent = std::dynamic_pointer_cast<Bot>(game.map->GetEntity({x,y}));
                if(!ent) return "bot not found";
                game.selectedBot = ent;
                return "bot selected";
            }
        }
    },
    { "killbot",
        {
            [](Game& game, std::stringstream& out) -> std::string {
                if(game.selectedBot.expired()) return "no bot selected";
                auto bot = game.selectedBot.lock();

                game.map->DestroyEntity(bot);
                return "bot destroyed";
            }
        }
    },
    { "training",
        {
            [](Game& game, std::stringstream& out) -> std::string {
                if(game.selectedBot.expired()) return "no bot selected";
                auto bot = game.selectedBot.lock();

                bot->training = !bot->training;
                return bot->training ? std::string("bot is training") + std::string(bot->dummy ? " (dummy is overriding)" : "") : std::string("bot is not training");
            }
        }
    },
    { "xtra",
        {
            [](Game& game, std::stringstream& out) -> std::string {
                if(game.selectedBot.expired()) return "no bot selected";
                auto bot = game.selectedBot.lock();

                bot->xtraining = !bot->xtraining;
                return bot->xtraining ? std::string("smart training mode") : std::string("dummy training mode");
            }
        }
    },
    { "dummy",
        {
            [](Game& game, std::stringstream& out) -> std::string {
                if(game.selectedBot.expired()) return "no bot selected";
                auto bot = game.selectedBot.lock();

                bot->dummy = !bot->dummy;
                return bot->dummy ? "bot is dummy" : "bot is not dummy";
            }
        }
    },
    { "flipflop",
        {
            [](Game& game, std::stringstream& out) -> std::string {
                if(game.selectedBot.expired()) return "no bot selected";
                auto bot = game.selectedBot.lock();

                bot->flipflop = !bot->flipflop;
                return bot->flipflop ? "flipflop mode is enabled" : "flipflop mode is disabled";
            }
        }
    },
    { "saveai",
        {
            [](Game& game, std::stringstream& out) -> std::string {
                if(game.selectedBot.expired()) return "no bot selected";
                auto bot = game.selectedBot.lock();
                std::string name;
                out >> name;
                bot->ai.Save(name);
                return "bot ai saved";
            }
        }
    },
    { "loadai",
        {
            [](Game& game, std::stringstream& out) -> std::string {
                if(game.selectedBot.expired()) return "no bot selected";
                auto bot = game.selectedBot.lock();
                std::string name;
                out >> name;
                bot->ai.Load(name);
                return "bot ai loaded";
            }
        }
    },

    { "interval",
        {
            [](Game& game, std::stringstream& out) -> std::string {
                out >> game.updateInterval;
                return "update interval changed";
            }
        }
    },
    { "display",
        {
            [](Game& game, std::stringstream& out) -> std::string {
                game.displayScreen = !game.displayScreen;
                return game.displayScreen ? "display simulation" : "hide simulation";
            }
        }
    },
    { "verbose",
        {
            [](Game& game, std::stringstream& out) -> std::string {
                game.verboseTraining = !game.verboseTraining;
                return game.verboseTraining ? "verbose on" : "verbose off";
            }
        }
    },



};

Game::Game() {
    assert(_curGame == nullptr);
    if(Construct(640, 640, 1, 1)){
        sAppName = "AI Trainer";
        _curGame = this;
        Start();
    }
}

Game::~Game() {
    _curGame = nullptr;
}

bool Game::OnUserCreate() {
    map.reset(new GridMap(ScreenWidth(), ScreenHeight()));
    SetPixelMode(olc::Pixel::ALPHA);

    updateInterval = 1000;
    displayScreen = true;
    verboseTraining = false;
    running = true;
    return true;
}

bool Game::OnUserUpdate(float delta) {
    static Clock tm;
    if(tm.getSeconds() > 2){
        sAppName = "AI Trainer " + std::to_string(int(1.f / delta)) + "fps";
        tm.restart();
    }

    if(GetKey(olc::TAB).bPressed){
        ConsoleShow(olc::TAB, false);
    }

    if(GetMouse(olc::Mouse::LEFT).bPressed){
        auto ent = std::dynamic_pointer_cast<Bot>(map->GetEntity(GetMousePos() / map->cellSize));
        if(ent) selectedBot = ent;
    }
    if(!IsConsoleShowing()){
        map->Update(delta);
    }

    map->GarbageCollection();

    if(displayScreen){
        Clear(olc::BLANK);
        map->Draw();
    }
    return running;
}

bool Game::OnConsoleCommand(const std::string& data) {
    std::stringstream out(data);
    bool success = true;
    ConsoleCaptureStdOut(true);

    std::string command;
    out >> command;

    if(commands.count(command)){
        const Command& cmd = commands.at(command);
        std::cout << cmd.function(Current(), out) << "\n";
    } else {
        success = false;
        std::cout << "Invalid Command: " << command << "\n";
    }
    ConsoleCaptureStdOut(false);
    return success;
}