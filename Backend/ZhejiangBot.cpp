#include "ZhejiangBot.h"
#include <array>
#include <random>

namespace {

Move ShiftClockwise(Move m)
{
    switch (m) {
    case Move::Rock:     return Move::Paper;
    case Move::Paper:    return Move::Scissors;
    case Move::Scissors: return Move::Rock;
    default:             return Move::Rock;
    }
}

Move RandomMove()
{
    thread_local std::mt19937 rng{ std::random_device{}() };
    std::uniform_int_distribution<int> dist(0, 2);
    static constexpr std::array<Move, 3> moves = {
        Move::Rock, Move::Paper, Move::Scissors
    };
    return moves[static_cast<size_t>(dist(rng))];
}

} // namespace

Move ZhejiangBot::PickMove()
{
    if (!mHasPlayed || mLastDelta == 1) {
        mLastMove = RandomMove();
    } else if (mLastDelta == 2) {
        // Won last round — keep the same move.
    } else {
        // Lost last round — shift clockwise.
        mLastMove = ShiftClockwise(mLastMove);
    }
    mHasPlayed = true;
    return mLastMove;
}

void ZhejiangBot::NotifyResult(int delta)
{
    mLastDelta = delta;
}
