#pragma once
#include "forward.h"

#include "ai.h"
#include "entity.h"
#include "clock.h"

#include <vector>

class Bot : public Entity {
public:
    enum Action {
        MOVE_NONE,
        MOVE_LEFT,
        MOVE_RIGHT,
        MOVE_UP,
        MOVE_DOWN
    };

    // Game State affects my next move
    struct GameState {
        static const size_t GRID_WIDTH = 7;
        static const size_t GRID_SIZE = GRID_WIDTH * GRID_WIDTH;

        Action action;
        float health;
        std::map<std::pair<int,int>,std::vector<EntityTypes::EntityType>> grid;

        Matrix convert();
        Matrix grid_matrix();
    };

    olc::Pixel color;
    float health;
    Action lastAction;
    olc::vi2d lastPosition;
    GameState lastState;

    AI ai;
    Clock stateChangeTimer;
    bool training, xtraining, dummy, flipflop;
    double lastStateReward, lastLoss;

    Bot(olc::vi2d pos);
    virtual ~Bot();

    virtual void ExecuteAction(Action action);

    virtual GameState GetCurrentState();
    virtual double CalculateReward(const GameState& state);

    virtual void Update(float delta) override;
    virtual void Draw() override;
};