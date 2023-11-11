#pragma once

#include <iostream>
#include <string>
#include <memory>
#include <algorithm>
#include <functional>
#include <filesystem>
#include <vector>

#include "MiniDNN.h"

typedef Eigen::MatrixXd Matrix;
typedef Eigen::VectorXd Vector;

extern Matrix MergeMatrix(const Matrix& a, const Matrix& b);

class AI {
protected:
    std::string netfolder;
    std::unique_ptr<MiniDNN::Network> network;
    MiniDNN::Adam opt;

public:
    AI();
    virtual ~AI();

    void Save(const std::string& name);
    void Load(const std::string& name);

    int Test_setup(size_t rows);
    int Test_train(const Matrix& state, std::function<Matrix()> calcReward, int repeat=1);
    double Test_run(const Matrix& state);
};