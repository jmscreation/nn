
#include "ai.h"

using namespace MiniDNN;
namespace fs = std::filesystem;

Matrix MergeMatrix(const Matrix& a, const Matrix& b) {
    size_t len = a.rows() + b.rows();
    Matrix output = Matrix::Zero(len,1);
    for(int i=0; i < len; ++i){
        output(i) = i < a.rows() ? a(i) : b(i - a.rows());
    }
    return output;
}


AI::AI(): netfolder("net"), network(std::make_unique<Network>()) {

}

AI::~AI() {

}

void AI::Save(const std::string& name) {
    std::string dir = netfolder + "/" + name;
    if(fs::exists(dir) && fs::is_directory(dir)){
        fs::remove_all(dir);
    }
    network->export_net(dir, name);
}

// you will lose unsaved changes
void AI::Load(const std::string& name) {
    opt.reset();
    network.reset(new Network);
    network->read_net(netfolder + "/" + name, name);
}

int AI::Test_setup(size_t rows) {
    // Create three layers
    Layer* layer1 = new FullyConnected<Identity>(rows, 250);
    Layer* layer2 = new FullyConnected<ReLU>(250, 250);   
    Layer* layer3 = new FullyConnected<Identity>(250, 1);

    // Add layers to the network object
    network->add_layer(layer1);
    network->add_layer(layer2);
    network->add_layer(layer3);

    // Set output layer
    network->set_output(new RegressionMSE());

    // Learning rate
    opt.m_lrate = 0.01;

    network->init(0, 0.01);
    return 0;
}

// Stephen is such a better programmer than me 
//   -Josh

int AI::Test_train(const Matrix& state, std::function<Matrix()> calcReward, int repeat) {
    Matrix reward = calcReward(); // return Matrix(1,1) which is real reward

    // Fit the model
    // network->forward(gstate.raw);
    // network->backprop(gstate.raw, reward);
    // network->update(opt);
    try{
        network->fit(opt, state, reward, 1, repeat, -1);
    } catch(std::exception e){
        std::cout << e.what() << "\n";
        return 0.0;
    }

    return 0;
}

double AI::Test_run(const Matrix& state) {
    // Obtain prediction -- each column is an observation
    try {
        Matrix pred = network->predict(state);
        return pred(0);
    } catch(std::exception e){
        std::cout << e.what() << "\n";
        return 0.0;
    }
}