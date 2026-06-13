#include <gtest/gtest.h>
#include <GameEngine.h>
#include <algorithm>
#include <stdexcept>

// ── MoveToChar ────────────────────────────────────────────────────────────────

TEST(MoveToChar, KnownMoves) {
    EXPECT_EQ(GameEngine::MoveToChar(Move::Rock),     'r');
    EXPECT_EQ(GameEngine::MoveToChar(Move::Paper),    'p');
    EXPECT_EQ(GameEngine::MoveToChar(Move::Scissors), 's');
}

TEST(MoveToChar, InvalidReturnsQuestionMark) {
    EXPECT_EQ(GameEngine::MoveToChar(Move::Invalid), '?');
}

// ── GetScore ──────────────────────────────────────────────────────────────────

TEST(GetScore, Ties) {
    EXPECT_EQ(GameEngine::GetScore(Move::Rock,     Move::Rock),     1);
    EXPECT_EQ(GameEngine::GetScore(Move::Paper,    Move::Paper),    1);
    EXPECT_EQ(GameEngine::GetScore(Move::Scissors, Move::Scissors), 1);
}

TEST(GetScore, Wins) {
    EXPECT_EQ(GameEngine::GetScore(Move::Rock,     Move::Scissors), 2);
    EXPECT_EQ(GameEngine::GetScore(Move::Paper,    Move::Rock),     2);
    EXPECT_EQ(GameEngine::GetScore(Move::Scissors, Move::Paper),    2);
}

TEST(GetScore, Losses) {
    EXPECT_EQ(GameEngine::GetScore(Move::Rock,     Move::Paper),    0);
    EXPECT_EQ(GameEngine::GetScore(Move::Paper,    Move::Scissors), 0);
    EXPECT_EQ(GameEngine::GetScore(Move::Scissors, Move::Rock),     0);
}

TEST(GetScore, InvalidM1Throws) {
    EXPECT_THROW(GameEngine::GetScore(Move::Invalid, Move::Rock), std::invalid_argument);
}

// ── EvaluateRound ─────────────────────────────────────────────────────────────

TEST(EvaluateRound, ForfeitInvalidInput) {
    RoundResult r = GameEngine::EvaluateRound(Move::Invalid, {Move::Rock, Move::Paper});
    EXPECT_EQ(r.outcome, RoundResult::Outcome::InvalidInput);
    EXPECT_EQ(r.deltas, (std::vector<int>{0, 2, 2}));
}

TEST(EvaluateRound, PlayerWins1v1) {
    RoundResult r = GameEngine::EvaluateRound(Move::Rock, {Move::Scissors});
    EXPECT_EQ(r.outcome, RoundResult::Outcome::PlayerWins);
    EXPECT_EQ(r.deltas, (std::vector<int>{2, 0}));
}

TEST(EvaluateRound, BotWins1v1) {
    RoundResult r = GameEngine::EvaluateRound(Move::Rock, {Move::Paper});
    EXPECT_EQ(r.outcome, RoundResult::Outcome::BotWins);
    EXPECT_EQ(r.deltas, (std::vector<int>{0, 2}));
}

TEST(EvaluateRound, IdenticalMoveDraw) {
    RoundResult r = GameEngine::EvaluateRound(Move::Scissors, {Move::Scissors});
    EXPECT_EQ(r.outcome, RoundResult::Outcome::Draw);
    EXPECT_EQ(r.deltas, (std::vector<int>{1, 1}));
}

TEST(EvaluateRound, ThreeWayDraw) {
    RoundResult r = GameEngine::EvaluateRound(Move::Rock, {Move::Paper, Move::Scissors});
    EXPECT_EQ(r.outcome, RoundResult::Outcome::ThreeWayDraw);
    EXPECT_EQ(r.deltas, (std::vector<int>{1, 1, 1}));
}

TEST(EvaluateRound, ThreeWayDrawWithMoreBots) {
    // All three move types present among 4 participants.
    RoundResult r = GameEngine::EvaluateRound(Move::Rock, {Move::Paper, Move::Scissors, Move::Rock});
    EXPECT_EQ(r.outcome, RoundResult::Outcome::ThreeWayDraw);
    EXPECT_EQ(r.deltas, (std::vector<int>{1, 1, 1, 1}));
}

TEST(EvaluateRound, TieAtTopPlayerAndBot) {
    // Rock vs Rock vs Scissors: player and bot0 both beat bot1.
    RoundResult r = GameEngine::EvaluateRound(Move::Rock, {Move::Rock, Move::Scissors});
    EXPECT_EQ(r.outcome, RoundResult::Outcome::PlayerWins);
    EXPECT_EQ(r.deltas, (std::vector<int>{2, 2, 0}));  // player and bot0 at top
}

TEST(EvaluateRound, MultipleBotsWinPlayerLoses) {
    // Player Paper, bots Scissors+Scissors: both bots beat the player.
    RoundResult r = GameEngine::EvaluateRound(Move::Paper, {Move::Scissors, Move::Scissors});
    EXPECT_EQ(r.outcome, RoundResult::Outcome::BotWins);
    EXPECT_EQ(r.deltas, (std::vector<int>{0, 2, 2}));
}

TEST(EvaluateRound, EmptyBotVector) {
    // Only the player — no pairs to compare, maxWins==0 -> Draw.
    RoundResult r = GameEngine::EvaluateRound(Move::Rock, {});
    EXPECT_EQ(r.outcome, RoundResult::Outcome::Draw);
    EXPECT_EQ(r.deltas, (std::vector<int>{1}));
}

TEST(EvaluateRound, DeltasSizeMatchesParticipantCount) {
    RoundResult r = GameEngine::EvaluateRound(Move::Rock, {Move::Paper, Move::Scissors, Move::Rock});
    EXPECT_EQ(r.deltas.size(), 4u);  // player + 3 bots
}

TEST(EvaluateRound, MovesPreservedInResult) {
    RoundResult r = GameEngine::EvaluateRound(Move::Rock, {Move::Paper, Move::Scissors});
    EXPECT_EQ(r.moves, (std::vector<Move>{Move::Rock, Move::Paper, Move::Scissors}));
}

// ── EvaluateRound — tournament scenarios ──────────────────────────────────────
// TournamentManager::RunMatch identifies winners by delta values, not by
// the outcome field. These tests verify the delta contract that RunMatch relies on:
//   maxDelta == 1  →  draw, repeat the round
//   exactly 1 participant with delta == 2  →  advance that participant
//   exactly 2 participants with delta == 2  →  deciding sub-match needed

TEST(EvaluateRound_Tournament, Duel_Draw_MaxDeltaIsOne) {
    RoundResult r = GameEngine::EvaluateRound(Move::Rock, {Move::Rock});
    EXPECT_EQ(r.deltas, (std::vector<int>{1, 1}));
    int maxDelta = *std::max_element(r.deltas.begin(), r.deltas.end());
    EXPECT_EQ(maxDelta, 1);
}

TEST(EvaluateRound_Tournament, Duel_ExactlyOneWinner) {
    RoundResult r = GameEngine::EvaluateRound(Move::Rock, {Move::Scissors});
    EXPECT_EQ(r.deltas, (std::vector<int>{2, 0}));
    int countWinners = 0;
    for (int d : r.deltas) countWinners += (d == 2 ? 1 : 0);
    EXPECT_EQ(countWinners, 1);
}

TEST(EvaluateRound_Tournament, Triple_ClearSingleWinner) {
    RoundResult r = GameEngine::EvaluateRound(Move::Rock, {Move::Scissors, Move::Scissors});
    EXPECT_EQ(r.deltas, (std::vector<int>{2, 0, 0}));
    int countWinners = 0;
    for (int d : r.deltas) countWinners += (d == 2 ? 1 : 0);
    EXPECT_EQ(countWinners, 1);
}

TEST(EvaluateRound_Tournament, Triple_AllSameDraw_MaxDeltaIsOne) {
    RoundResult r = GameEngine::EvaluateRound(Move::Rock, {Move::Rock, Move::Rock});
    EXPECT_EQ(r.outcome, RoundResult::Outcome::Draw);
    EXPECT_EQ(r.deltas, (std::vector<int>{1, 1, 1}));
    int maxDelta = *std::max_element(r.deltas.begin(), r.deltas.end());
    EXPECT_EQ(maxDelta, 1);
}

TEST(EvaluateRound_Tournament, Triple_ThreeWayDraw_MaxDeltaIsOne) {
    RoundResult r = GameEngine::EvaluateRound(Move::Rock, {Move::Paper, Move::Scissors});
    EXPECT_EQ(r.outcome, RoundResult::Outcome::ThreeWayDraw);
    EXPECT_EQ(r.deltas, (std::vector<int>{1, 1, 1}));
    int maxDelta = *std::max_element(r.deltas.begin(), r.deltas.end());
    EXPECT_EQ(maxDelta, 1);
}

TEST(EvaluateRound_Tournament, Triple_TwoCoWinners_PlayerAndBot) {
    // Rock vs {Rock, Scissors}: idx0 and idx1 both beat idx2 → two co-winners,
    // deciding sub-match required.
    RoundResult r = GameEngine::EvaluateRound(Move::Rock, {Move::Rock, Move::Scissors});
    EXPECT_EQ(r.deltas, (std::vector<int>{2, 2, 0}));
    int countWinners = 0;
    for (int d : r.deltas) countWinners += (d == 2 ? 1 : 0);
    EXPECT_EQ(countWinners, 2);
}

TEST(EvaluateRound_Tournament, Triple_TwoCoWinners_BothBots) {
    // Scissors vs {Rock, Rock}: both bots beat the player → player eliminated,
    // deciding sub-match between the two bots required.
    RoundResult r = GameEngine::EvaluateRound(Move::Scissors, {Move::Rock, Move::Rock});
    EXPECT_EQ(r.deltas, (std::vector<int>{0, 2, 2}));
    int countWinners = 0;
    for (int d : r.deltas) countWinners += (d == 2 ? 1 : 0);
    EXPECT_EQ(countWinners, 2);
}
