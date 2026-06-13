#pragma once
#include "Move.h"

class IBotStrategy {
public:
    virtual ~IBotStrategy() = default;
    virtual Move PickMove() = 0;
    // Called after each round with the bot's point delta (2=win, 1=draw, 0=loss).
    virtual void NotifyResult(int delta) {}
};
