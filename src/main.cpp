#include "ai.h"
#include "game.h"
#include <memory>


int main() {
    std::make_unique<Game>().reset(nullptr);

    return 0;
}


/*
int main() {
    Matrix first = Matrix::Random(7,1), second = Matrix::Random(3,1);

    std::cout << MergeMatrix(first, second);

    return 0;
}
*/