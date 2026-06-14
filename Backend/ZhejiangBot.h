#pragma once
#include "IBotStrategy.h"

// Implements the "win-stay, lose-shift" strategy documented in the Zhejiang University
// study on Rock-Paper-Scissors: repeat the same move after a win; after a loss shift
// clockwise (Rock→Paper, Paper→Scissors, Scissors→Rock); pick randomly after a draw
// or on the first round.
class ZhejiangBot : public IBotStrategy {
    Move mLastMove  = Move::Rock;
    int  mLastDelta = 1; // 1 = draw, treated as "no clear history" initially
    bool mHasPlayed = false;

public:
    Move PickMove()          override;
    void NotifyResult(int delta) override;
};
