#pragma once
#include "Move.h"
#include <vector>

struct RoundResult {
    enum class Outcome {
        PlayerWins,    // participant 0 has most round-robin wins
        BotWins,       // some other participant wins, participant 0 does not
        Draw,          // everyone picked the same move
        ThreeWayDraw,  // all three move types present among participants
        InvalidInput   // a participant forfeited after 3 invalid attempts
    };

    Outcome outcome = Outcome::Draw;

    // Positional, one entry per participant.
    // Index 0 == the player in regular mode; in tournament mode simply the first group member.
    std::vector<int>  deltas; // points awarded to participant i
    std::vector<Move> moves;  // move chosen by participant i
};

class GameEngine {
public:
    static char MoveToChar(Move m);

    // Returns 2 if m1 wins over m2, 1 if tie, 0 if m1 loses.
    static int GetScore(Move m1, Move m2);

    // Pure round evaluation — no I/O, no threading.
    static RoundResult EvaluateRound(Move playerMove, const std::vector<Move>& botMoves);
};
