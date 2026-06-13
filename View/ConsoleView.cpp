#include "ConsoleView.h"
#include <GameEngine.h>
#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

ConsoleView::ConsoleView() : ConsoleView(std::cout) {}
ConsoleView::ConsoleView(std::ostream& out) : out(out) {}

void ConsoleView::ShowMovePrompt()
{
    std::lock_guard<std::mutex> lock(mutex);
    out << "Type \"r\", \"p\" or \"s\" to pick a move.\n";
}

void ConsoleView::ShowInvalidInput(int attemptsRemaining)
{
    std::lock_guard<std::mutex> lock(mutex);
    out << "Invalid choice. You have " << attemptsRemaining << " attempt(s) remaining.\n";
}

void ConsoleView::ShowBotReady(const std::string& botName)
{
    std::lock_guard<std::mutex> lock(mutex);
    out << botName << " is ready.\n";
}

void ConsoleView::ShowRoundMoves(const std::string& playerName, Move player,
                                  const std::vector<std::string>& botNames,
                                  const std::vector<Move>& bots)
{
    std::lock_guard<std::mutex> lock(mutex);
    out << "It's " << playerName << ": " << GameEngine::MoveToChar(player) << " vs ";
    for (size_t i = 0; i < bots.size(); ++i) {
        if (i != 0) out << ", ";
        out << botNames[i] << ": " << GameEngine::MoveToChar(bots[i]);
    }
    out << "...\n";
}

void ConsoleView::ShowRoundResult(const std::string& playerName,
                                   const std::vector<std::string>& botNames,
                                   const RoundResult& result)
{
    std::lock_guard<std::mutex> lock(mutex);
    switch (result.outcome)
    {
    case RoundResult::Outcome::ThreeWayDraw:
        out << "All three options in play. It is a three-way draw!\n";
        break;
    case RoundResult::Outcome::Draw:
        out << "It's a draw!\n";
        break;
    case RoundResult::Outcome::PlayerWins:
    {
        std::vector<std::string> winners;
        winners.push_back(playerName);
        for (size_t i = 1; i < result.deltas.size(); ++i)
            if (result.deltas[i] == 2)
                winners.push_back(botNames[i - 1]);
        if (winners.size() == 1) {
            out << playerName << " wins the round!\n";
        } else {
            out << "Tie between winners: ";
            for (size_t i = 0; i < winners.size(); ++i) {
                if (i != 0) out << ", ";
                out << winners[i];
            }
            out << ".\n";
        }
    }
        break;
    case RoundResult::Outcome::BotWins:
    {
        std::vector<std::string> winners;
        for (size_t i = 1; i < result.deltas.size(); ++i)
            if (result.deltas[i] == 2)
                winners.push_back(botNames[i - 1]);
        if (winners.size() == 1) {
            out << winners[0] << " wins the round!\n";
        } else {
            out << "Tie between winners: ";
            for (size_t i = 0; i < winners.size(); ++i) {
                if (i != 0) out << ", ";
                out << winners[i];
            }
            out << ".\n";
        }
    }
        break;
    case RoundResult::Outcome::InvalidInput:
        out << "Too many invalid attempts. " << playerName << " automatically loses!\n";
        break;
    }
}

void ConsoleView::ShowFinalScores(const std::string& playerName, int playerScore,
                                   const std::vector<std::string>& botNames,
                                   const std::vector<int>& botScores)
{
    std::lock_guard<std::mutex> lock(mutex);
    out << playerName << " score  : " << playerScore << '\n';
    for (size_t i = 0; i < botScores.size(); ++i)
        out << botNames[i] << " score : " << botScores[i] << '\n';
}

void ConsoleView::ShowMessage(const std::string& msg)
{
    std::lock_guard<std::mutex> lock(mutex);
    out << msg << '\n';
}

void ConsoleView::ShowTournamentRoundStart(int roundNum)
{
    std::lock_guard<std::mutex> lock(mutex);
    out << "\n=== Tournament Round " << roundNum << " ===\n";
}

void ConsoleView::ShowMatchStart(const std::vector<std::string>& names)
{
    std::lock_guard<std::mutex> lock(mutex);
    out << "--- Match: ";
    for (size_t i = 0; i < names.size(); ++i) {
        if (i != 0) out << " vs ";
        out << names[i];
    }
    out << " ---\n";
}

void ConsoleView::ShowMatchResult(const std::string& winnerName)
{
    std::lock_guard<std::mutex> lock(mutex);
    out << "Match winner: " << winnerName << "\n";
}

void ConsoleView::ShowSubMatchStart(const std::vector<std::string>& names)
{
    std::lock_guard<std::mutex> lock(mutex);
    out << "--- Deciding match: ";
    for (size_t i = 0; i < names.size(); ++i) {
        if (i != 0) out << " vs ";
        out << names[i];
    }
    out << " ---\n";
}

void ConsoleView::ShowTournamentWinner(const std::string& winnerName)
{
    std::lock_guard<std::mutex> lock(mutex);
    out << "\n=== TOURNAMENT CHAMPION: " << winnerName << "! ===\n";
}

void ConsoleView::ShowTournamentBotReady(const std::string& botName)
{
    std::lock_guard<std::mutex> lock(mutex);
    out << botName << " is ready.\n";
}

void ConsoleView::ShowTournamentMoves(const std::vector<std::string>& names,
                                      const std::vector<Move>& moves)
{
    std::lock_guard<std::mutex> lock(mutex);
    for (size_t i = 0; i < names.size(); ++i) {
        if (i != 0) out << " | ";
        out << names[i] << ": " << GameEngine::MoveToChar(moves[i]);
    }
    out << "\n";
}

void ConsoleView::ShowTournamentResult(const std::vector<std::string>& names,
                                       const RoundResult& result)
{
    std::lock_guard<std::mutex> lock(mutex);

    if (result.outcome == RoundResult::Outcome::InvalidInput) {
        std::string loserName = names[0];
        for (size_t i = 0; i < result.deltas.size(); ++i)
            if (result.deltas[i] == 0) { loserName = names[i]; break; }
        out << "Too many invalid attempts. " << loserName << " automatically loses!\n";
        return;
    }

    const int maxDelta = *std::max_element(result.deltas.begin(), result.deltas.end());

    if (maxDelta == 1) {
        out << "It's a draw!\n";
        return;
    }

    std::vector<std::string> winners;
    for (size_t i = 0; i < result.deltas.size(); ++i)
        if (result.deltas[i] == maxDelta) winners.push_back(names[i]);

    if (winners.size() == 1) {
        out << winners[0] << " wins the round!\n";
    } else {
        out << "Tied between: ";
        for (size_t i = 0; i < winners.size(); ++i) {
            if (i != 0) out << " and ";
            out << winners[i];
        }
        out << " - deciding match required!\n";
    }
}
