#pragma once
#include <IGameView.h>
#include <iosfwd>
#include <mutex>

// Console implementation of IGameView.
// All methods are thread-safe: an internal mutex serialises writes to `out`
// so that bot worker threads and the main thread do not interleave output.
class ConsoleView : public IGameView {
    std::ostream& out;
    mutable std::mutex mutex;

public:
    ConsoleView();                           // uses std::cout
    explicit ConsoleView(std::ostream& out);

    void ShowMovePrompt() override;
    void ShowInvalidInput(int attemptsRemaining) override;
    void ShowBotReady(const std::string& botName) override;
    void ShowRoundMoves(const std::string& playerName,
                        Move player,
                        const std::vector<std::string>& botNames,
                        const std::vector<Move>& bots) override;
    void ShowRoundResult(const std::string& playerName,
                         const std::vector<std::string>& botNames,
                         const RoundResult& result) override;
    void ShowFinalScores(const std::string& playerName,
                         int playerScore,
                         const std::vector<std::string>& botNames,
                         const std::vector<int>& botScores) override;
    void ShowMessage(const std::string& msg) override;

    void ShowTournamentRoundStart(int roundNum) override;
    void ShowMatchStart(const std::vector<std::string>& names) override;
    void ShowMatchResult(const std::string& winnerName) override;
    void ShowSubMatchStart(const std::vector<std::string>& names) override;
    void ShowTournamentWinner(const std::string& winnerName) override;
    void ShowTournamentBotReady(const std::string& botName) override;
    void ShowTournamentMoves(const std::vector<std::string>& names,
                             const std::vector<Move>& moves) override;
    void ShowTournamentResult(const std::vector<std::string>& names,
                              const RoundResult& result) override;
};
