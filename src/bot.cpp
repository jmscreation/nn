#include "bot.h"
#include "game.h"

static double invLerp(double xx, double yy, double value) {
    return std::clamp((value - xx)/(yy - xx) * 2.0 - 1.0, -1.0, 1.0);
};

// convert game state to AI game state matrix
Matrix Bot::GameState::convert() {
    Matrix m(1,1);
    m(0) = invLerp(0.0, 5.0, (double)action);
    //m(1) = invLerp(0.0, 100.0, health);

    return MergeMatrix(m, grid_matrix());
}

Matrix Bot::GameState::grid_matrix() {
    Matrix m;
    m = grid.size() ? Matrix(grid.size(), 1) : Matrix::Zero(GRID_SIZE,1);
    int i=0;
    for(auto& [_pos,types] : grid){
        double typeavg {};
        if(types.size()){
            for(auto type : types) typeavg += invLerp(0.0, 5.0, double(type));
            typeavg /= types.size();
        } else {
            typeavg = 1.0;
        }
        m(i++) = typeavg; // type as -1 to 1
    }
    return m;
}

Bot::Bot(olc::vi2d pos): Entity(EntityTypes::Bot, pos), color(0xFFFFFFFF), health(100.f),
        training(true), xtraining(false), dummy(true), flipflop(false),
        lastStateReward(0), lastLoss(0), lastState({}) {
    ai.Test_setup(GameState::GRID_SIZE + 1); // 7x7 grid + 1 action
}

Bot::~Bot() {

}

void Bot::ExecuteAction(Action action) {
    auto& map = *Game::Current().map;
    lastPosition = position;
    switch(action) {
        case MOVE_LEFT:
            position.x--;
            break;
        case MOVE_RIGHT:
            position.x++;
            break;
        case MOVE_UP:
            position.y--;
            break;
        case MOVE_DOWN:
            position.y++;
            break;
    }
    if(position.x < 0 || position.y < 0 || position.x >= map.size.x || position.y >= map.size.y){
        position = lastPosition;
    }
    lastAction = action;
}

Bot::GameState Bot::GetCurrentState() {
    auto& game = Game::Current();
    GameState state;
    
    // look around me -- this is extremely inefficient right now
    for(size_t i=0; i < GameState::GRID_SIZE; ++i){
        int x = i % GameState::GRID_WIDTH - GameState::GRID_WIDTH / 2,
            y = i / GameState::GRID_WIDTH - GameState::GRID_WIDTH / 2;

        if(!state.grid.count({x,y})){
            state.grid.insert_or_assign(std::pair<int,int>(x,y), std::vector<EntityTypes::EntityType>());
        }
        std::vector<EntityTypes::EntityType>& list = state.grid.at({x,y});
        list.clear();
        if(position.x + x < 0 || position.y + y < 0 || position.x + x >= game.map->size.x || position.y + y >= game.map->size.y){
            list.emplace_back(EntityTypes::OutOfBounds);
            continue;
        }
        for(const auto& e : game.map->GetEntities({position.x + x, position.y + y})){
            if(e.get() == this) continue; // don't add myself to the grid
            list.emplace_back(e->type);
        }
    }
    state.action = lastAction;
    state.health = health;
    return state;
}

double Bot::CalculateReward(const GameState& state) {
    // return -100 to 100 reward based on state
    double reward = 35.0 + double(std::rand() % 15);

    if(state.action != Action::MOVE_NONE && lastPosition == position)
        reward -= 50.0; // the bot collided with something
    if(state.action == Action::MOVE_NONE)
        reward -= 20.0; // not really good to stay

    for(auto& [_pos,types] : state.grid){
        olc::vi2d pos(_pos.first, _pos.second);
        double dist = olc::vf2d(pos).mag2();
        if(std::isnan(dist) || dist < 0.5) dist = 0.5;
        for(const auto& type : types){
            switch(type){
                case EntityTypes::Bot: // reward or punish based on type and distance
                    reward -= 20.0 / dist;
                    break;
                case EntityTypes::OutOfBounds:
                case EntityTypes::Wall:
                    reward -= 10.0 / dist;
                    break;
            }
        }
    }
    return invLerp(-100.0, 100.0, reward);
}

void Bot::Update(float delta) {
    auto& game = Game::Current();
    Entity::Update(delta); // inherit

    if(stateChangeTimer.getMilliseconds() > game.updateInterval){
        GameState state = GetCurrentState(); // get the state before moving
        if(dummy){
            Action a = Action(std::rand() % 5);
            ExecuteAction(a); // move randomly
            state.action = a;
        } else {// temporary
            if(flipflop) training = !training;
            if(!training || xtraining){
                const static std::vector<Action> actionList = {
                    MOVE_NONE,MOVE_LEFT,MOVE_RIGHT,MOVE_UP,MOVE_DOWN
                };
                std::map<double, Action> actions;
                for(const auto& action : actionList){
                    state.action = action; // check all rewards for all actions
                    double reward = ai.Test_run(state.convert());
                    actions.insert_or_assign(reward, action); // current state and predict cur action
                    if(game.verboseTraining){
                        std::cout << "Action=" << action << " Reward=" << reward
                                << " Loss=" << std::abs(reward - CalculateReward(state)) << "\n";
                    }
                }

                auto it = --actions.end(); if(!(std::rand() % 5)) --it;
                Action action = it->second;
                lastStateReward = action; // last best reward
                if(game.verboseTraining){
                    std::cout << "Chose Action: " << action << "\n";
                }
                ExecuteAction(action);
                state.action = action; // set the last state's action to the current move
            }

            if(training){
                if(!xtraining){
                    Action a = Action(std::rand() % 5);
                    ExecuteAction(a);
                    state.action = a;
                }
                // test train
                Matrix mState = state.convert(); // the state before moving

                ai.Test_train(mState, [&](){
                    Matrix raw = Matrix(1,1);
                    lastAction = state.action;
                    lastStateReward = CalculateReward(GetCurrentState()); // train with a reward value with the current state after moving
                    raw(0) = lastStateReward;
                    return raw;
                });
                lastLoss = std::abs(ai.Test_run(mState) - lastStateReward);
            }
        }

        if(dummy){
            color = olc::BLUE;
        } else {
            color = training ? (xtraining ? 0xFF0084DB : olc::RED) : olc::GREEN;
        }

        lastState = state; // update last state to the current state before the state is updated to the current state
        stateChangeTimer.restart();
    }

}

void Bot::Draw() {
    auto& game = Game::Current();

    int rd = std::min(game.map->cellSize.x, game.map->cellSize.y) / 2;
    olc::vi2d worldPos = position * game.map->cellSize + olc::vi2d(rd, rd);

    game.FillCircle(worldPos, rd, color);
    if(!game.selectedBot.expired() && game.selectedBot.lock().get() == this){
        game.FillCircle(worldPos, rd + 4, 0x4F70FF70);
    }
    game.DrawString(worldPos, std::to_string(lastStateReward));
}