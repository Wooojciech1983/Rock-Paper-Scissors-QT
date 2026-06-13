#include "RandomBot.h"
#include <array>
#include <random>

Move RandomBot::PickMove()
{
    thread_local std::mt19937 rng{ std::random_device{}() };
    std::uniform_int_distribution<int> dist(0, 2);
    static constexpr std::array<Move, 3> moves = {
        Move::Rock, Move::Paper, Move::Scissors
    };
    return moves[static_cast<size_t>(dist(rng))];
}
