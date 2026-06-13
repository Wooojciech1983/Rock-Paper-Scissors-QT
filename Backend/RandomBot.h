#pragma once
#include "IBotStrategy.h"

class RandomBot : public IBotStrategy {
public:
    Move PickMove() override;
};
