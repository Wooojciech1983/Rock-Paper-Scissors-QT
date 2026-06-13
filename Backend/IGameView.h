#pragma once
#include <GameEngine.h>
#include <string>
#include <vector>

class IGameView {
public:
    virtual ~IGameView() = default;

    // Called before reading player input each round.
    virtual void ShowMovePrompt() = 0;

    // Called after an invalid user input (attemptsRemaining > 0).
    virtual void ShowInvalidInput(int attemptsRemaining) = 0;

    // Called from a bot worker thread once the bot has decided its move.
    virtual void ShowBotReady(const std::string& botName) = 0;

    // Called after all moves are collected; shows player and bot choices.
    virtual void ShowRoundMoves(const std::string& playerName,
                                Move player,
                                const std::vector<std::string>& botNames,
                                const std::vector<Move>& bots) = 0;

    // Called after EvaluateRound; shows outcome message.
    virtual void ShowRoundResult(const std::string& playerName,
                                 const std::vector<std::string>& botNames,
                                 const RoundResult& result) = 0;

    // Called once at game end; shows final scores for all participants.
    virtual void ShowFinalScores(const std::string& playerName,
                                 int playerScore,
                                 const std::vector<std::string>& botNames,
                                 const std::vector<int>& botScores) = 0;

    // General-purpose message (welcome text, prompts, errors).
    virtual void ShowMessage(const std::string& msg) = 0;

    // Tournament-mode methods
    virtual void ShowTournamentRoundStart(int roundNum) = 0;
    virtual void ShowMatchStart(const std::vector<std::string>& names) = 0;
    virtual void ShowMatchResult(const std::string& winnerName) = 0;
    virtual void ShowSubMatchStart(const std::vector<std::string>& names) = 0;
    virtual void ShowTournamentWinner(const std::string& winnerName) = 0;
    virtual void ShowTournamentBotReady(const std::string& botName) = 0;
    virtual void ShowTournamentMoves(const std::vector<std::string>& names,
                                     const std::vector<Move>& moves) = 0;
    virtual void ShowTournamentResult(const std::vector<std::string>& names,
                                      const RoundResult& result) = 0;
};
