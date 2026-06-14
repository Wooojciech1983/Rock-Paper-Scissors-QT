#include "GameEngine.h"
#include <algorithm>
#include <stdexcept>

char GameEngine::MoveToChar(Move m)
{
    switch (m)
    {
    case Move::Rock:     return 'r';
    case Move::Paper:    return 'p';
    case Move::Scissors: return 's';
    default:             return '?';
    }
}

int GameEngine::GetScore(Move m1, Move m2)
{
    if (m1 == m2) return 1;
    switch (m1)
    {
    case Move::Rock:     return (m2 == Move::Scissors) ? 2 : 0;
    case Move::Paper:    return (m2 == Move::Rock)     ? 2 : 0;
    case Move::Scissors: return (m2 == Move::Paper)    ? 2 : 0;
    default: throw std::invalid_argument("GetScore: invalid move value");
    }
}

RoundResult GameEngine::EvaluateRound(Move playerMove, const std::vector<Move>& botMoves)
{
    const size_t n = botMoves.size() + 1;

    RoundResult result;
    result.moves.resize(n);
    result.moves[0] = playerMove;
    for (size_t i = 0; i < botMoves.size(); ++i)
        result.moves[i + 1] = botMoves[i];
    result.deltas.assign(n, 0);

    if (playerMove == Move::Invalid) {
        result.outcome = RoundResult::Outcome::InvalidInput;
        for (size_t i = 1; i < n; ++i) result.deltas[i] = 2; // bots win, player(0) loses
        return result;
    }

    // A bot may forfeit with Move::Invalid (e.g. a disconnected remote player).
    // Such participants lose every pairing and never award/score points; only the
    // valid moves decide draws. (GetScore is never called with Invalid, so its
    // defensive throw is preserved.)
    bool hasRock = false, hasPaper = false, hasScissors = false;
    for (Move m : result.moves) {
        if (m == Move::Rock)     hasRock     = true;
        if (m == Move::Paper)    hasPaper    = true;
        if (m == Move::Scissors) hasScissors = true;
    }

    // Awards pts to every non-forfeiting participant (forfeiters keep their 0).
    auto awardValid = [&](int pts) {
        for (size_t i = 0; i < n; ++i)
            if (result.moves[i] != Move::Invalid) result.deltas[i] = pts;
    };

    if (hasRock && hasPaper && hasScissors) {
        result.outcome = RoundResult::Outcome::ThreeWayDraw;
        awardValid(1);
        return result;
    }

    // Round-robin: every pair plays; participant(s) with most wins take 2 pts.
    // A forfeiter (Invalid) loses to any valid move; two forfeiters tie with no win.
    std::vector<int> wins(n, 0);
    for (size_t i = 0; i < n; ++i)
        for (size_t j = i + 1; j < n; ++j) {
            const Move a = result.moves[i], b = result.moves[j];
            if (a == Move::Invalid && b == Move::Invalid) continue;
            if (a == Move::Invalid) { ++wins[j]; continue; }
            if (b == Move::Invalid) { ++wins[i]; continue; }
            const int r = GetScore(a, b);
            if (r == 2)      ++wins[i];
            else if (r == 0) ++wins[j];
        }

    const int maxWins = *std::max_element(wins.begin(), wins.end());

    // maxWins == 0: every pairing tied. A forfeiter would have handed the (always
    // valid) player a win, so this only happens when all participants are valid and
    // chose the same move — a genuine draw.
    if (maxWins == 0) {
        result.outcome = RoundResult::Outcome::Draw;
        for (auto& d : result.deltas) d = 1;
        return result;
    }

    for (size_t i = 0; i < n; ++i)
        result.deltas[i] = (wins[i] == maxWins) ? 2 : 0;

    result.outcome = (result.deltas[0] == 2)
                     ? RoundResult::Outcome::PlayerWins
                     : RoundResult::Outcome::BotWins;
    return result;
}
