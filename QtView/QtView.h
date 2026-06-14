#pragma once
#include <IGameView.h>
#include <QObject>
#include <QString>
#include <QStringList>
#include <string>
#include <vector>

// IGameView implementation that turns backend display calls into Qt signals.
//
// The backend invokes these methods from the game-logic worker thread (and bot
// "ready" notifications from std::async bot threads). Because the receiving
// widgets live in the GUI thread, Qt::AutoConnection promotes every emission to
// a queued connection, so delivery is marshalled onto the main thread — no
// explicit locking is required here. Methods therefore only emit signals; the
// only mutable state (the running scoreboard) is touched solely from the worker
// thread inside the round/result callbacks, never from the bot-ready callbacks.
class QtView : public QObject, public IGameView {
    Q_OBJECT

public:
    explicit QtView(QObject* parent = nullptr);

    void ShowMovePrompt() override;
    void ShowInvalidInput(int attemptsRemaining) override;
    void ShowBotReady(const std::string& botName) override;
    void ShowRoundMoves(const std::string& playerName, Move player,
                        const std::vector<std::string>& botNames,
                        const std::vector<Move>& bots) override;
    void ShowRoundResult(const std::string& playerName,
                         const std::vector<std::string>& botNames,
                         const RoundResult& result) override;
    void ShowFinalScores(const std::string& playerName, int playerScore,
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

signals:
    // A line of narration to append to the history/log view.
    void logMessage(QString text);
    // It is now the local human's turn to choose a move (enable move buttons).
    void movePromptRequested();
    // Player previously made an invalid choice; attemptsRemaining left.
    void invalidInput(int attemptsRemaining);
    // Refreshed scoreboard: parallel lists of participant names and score strings.
    void scoresUpdated(QStringList names, QStringList scores);
    // A regular-mode round finished resolving (cue to re-offer "Next round").
    void roundFinished();
    // The game (regular mode) or tournament has ended; final rows for display.
    void gameFinished(QStringList finalScoreLines);

private:
    int  scoreIndexOf(const QString& name);   // returns index, inserting if new
    void addScore(const QString& name, int delta);
    void emitScores();                         // emit current running scoreboard

    // Running scoreboard, accumulated from per-round deltas (regular mode only).
    std::vector<QString> mScoreNames;
    std::vector<int>     mScoreValues;
};
