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
    if (!hasPlayed || lastDelta == 1) {
        lastMove = RandomMove();
    } else if (lastDelta == 2) {
        // Won last round — keep the same move.
    } else {
        // Lost last round — shift clockwise.
        lastMove = ShiftClockwise(lastMove);
    }
    hasPlayed = true;
    return lastMove;
}

void ZhejiangBot::NotifyResult(int delta)
{
    lastDelta = delta;
}
