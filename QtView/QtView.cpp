#include "QtView.h"
#include <GameEngine.h>
#include <algorithm>

namespace {
QString moveChar(Move m) { return QString(QChar(GameEngine::MoveToChar(m))); }
QString q(const std::string& s) { return QString::fromStdString(s); }
} // namespace

QtView::QtView(QObject* parent) : QObject(parent) {}

int QtView::scoreIndexOf(const QString& name)
{
    for (int i = 0; i < static_cast<int>(mScoreNames.size()); ++i)
        if (mScoreNames[i] == name) return i;
    mScoreNames.push_back(name);
    mScoreValues.push_back(0);
    return static_cast<int>(mScoreNames.size()) - 1;
}

void QtView::addScore(const QString& name, int delta)
{
    mScoreValues[scoreIndexOf(name)] += delta;
}

void QtView::emitScores()
{
    QStringList names, scores;
    for (size_t i = 0; i < mScoreNames.size(); ++i) {
        names  << mScoreNames[i];
        scores << QString::number(mScoreValues[i]);
    }
    emit scoresUpdated(names, scores);
}

void QtView::ShowMovePrompt()
{
    emit movePromptRequested();
}

void QtView::ShowInvalidInput(int attemptsRemaining)
{
    emit invalidInput(attemptsRemaining);
    emit logMessage(QStringLiteral("Invalid choice. %1 attempt(s) remaining.")
                        .arg(attemptsRemaining));
}

void QtView::ShowBotReady(const std::string& botName)
{
    // Called from a bot worker thread — emit only, never touch shared state.
    emit logMessage(q(botName) + QStringLiteral(" is ready."));
}

void QtView::ShowRoundMoves(const std::string& playerName, Move player,
                            const std::vector<std::string>& botNames,
                            const std::vector<Move>& bots)
{
    QString text = QStringLiteral("It's %1: %2 vs ").arg(q(playerName), moveChar(player));
    for (size_t i = 0; i < bots.size(); ++i) {
        if (i != 0) text += QStringLiteral(", ");
        text += QStringLiteral("%1: %2").arg(q(botNames[i]), moveChar(bots[i]));
    }
    text += QStringLiteral("...");
    emit logMessage(text);
}

void QtView::ShowRoundResult(const std::string& playerName,
                             const std::vector<std::string>& botNames,
                             const RoundResult& result)
{
    QString text;
    switch (result.outcome) {
    case RoundResult::Outcome::ThreeWayDraw:
        text = QStringLiteral("All three options in play. It is a three-way draw!");
        break;
    case RoundResult::Outcome::Draw:
        text = QStringLiteral("It's a draw!");
        break;
    case RoundResult::Outcome::PlayerWins:
    case RoundResult::Outcome::BotWins: {
        QStringList winners;
        if (result.outcome == RoundResult::Outcome::PlayerWins)
            winners << q(playerName);
        for (size_t i = 1; i < result.deltas.size(); ++i)
            if (result.deltas[i] == 2) winners << q(botNames[i - 1]);
        if (winners.size() == 1)
            text = winners.front() + QStringLiteral(" wins the round!");
        else
            text = QStringLiteral("Tie between winners: ") + winners.join(QStringLiteral(", ")) + QChar('.');
        break;
    }
    case RoundResult::Outcome::InvalidInput:
        text = QStringLiteral("Too many invalid attempts. %1 automatically loses!").arg(q(playerName));
        break;
    }
    emit logMessage(text);

    // Accumulate the running scoreboard from this round's deltas.
    if (!result.deltas.empty()) {
        addScore(q(playerName), result.deltas[0]);
        for (size_t i = 0; i < botNames.size() && i + 1 < result.deltas.size(); ++i)
            addScore(q(botNames[i]), result.deltas[i + 1]);
        emitScores();
    }

    emit roundFinished();
}

void QtView::ShowFinalScores(const std::string& playerName, int playerScore,
                             const std::vector<std::string>& botNames,
                             const std::vector<int>& botScores)
{
    QStringList names, scores, lines;
    names  << q(playerName);
    scores << QString::number(playerScore);
    lines  << QStringLiteral("%1 : %2").arg(q(playerName)).arg(playerScore);
    for (size_t i = 0; i < botNames.size(); ++i) {
        names  << q(botNames[i]);
        scores << QString::number(botScores[i]);
        lines  << QStringLiteral("%1 : %2").arg(q(botNames[i])).arg(botScores[i]);
    }

    emit scoresUpdated(names, scores);
    emit gameFinished(lines);
}

void QtView::ShowMessage(const std::string& msg)
{
    emit logMessage(q(msg));
}

void QtView::ShowTournamentRoundStart(int roundNum)
{
    emit logMessage(QStringLiteral("=== Tournament Round %1 ===").arg(roundNum));
}

void QtView::ShowMatchStart(const std::vector<std::string>& names)
{
    QStringList n;
    for (const auto& s : names) n << q(s);
    emit logMessage(QStringLiteral("--- Match: ") + n.join(QStringLiteral(" vs ")) + QStringLiteral(" ---"));
}

void QtView::ShowMatchResult(const std::string& winnerName)
{
    emit logMessage(QStringLiteral("Match winner: ") + q(winnerName));
}

void QtView::ShowSubMatchStart(const std::vector<std::string>& names)
{
    QStringList n;
    for (const auto& s : names) n << q(s);
    emit logMessage(QStringLiteral("--- Deciding match: ") + n.join(QStringLiteral(" vs ")) + QStringLiteral(" ---"));
}

void QtView::ShowTournamentWinner(const std::string& winnerName)
{
    emit logMessage(QStringLiteral("=== TOURNAMENT CHAMPION: %1! ===").arg(q(winnerName)));
    emit gameFinished(QStringList{ QStringLiteral("Champion: ") + q(winnerName) });
}

void QtView::ShowTournamentBotReady(const std::string& botName)
{
    // Called from a bot worker thread — emit only.
    emit logMessage(q(botName) + QStringLiteral(" is ready."));
}

void QtView::ShowTournamentMoves(const std::vector<std::string>& names,
                                 const std::vector<Move>& moves)
{
    QStringList parts;
    for (size_t i = 0; i < names.size(); ++i)
        parts << QStringLiteral("%1: %2").arg(q(names[i]), moveChar(moves[i]));
    emit logMessage(parts.join(QStringLiteral(" | ")));
}

void QtView::ShowTournamentResult(const std::vector<std::string>& names,
                                  const RoundResult& result)
{
    if (result.outcome == RoundResult::Outcome::InvalidInput) {
        QString loser = q(names[0]);
        for (size_t i = 0; i < result.deltas.size(); ++i)
            if (result.deltas[i] == 0) { loser = q(names[i]); break; }
        emit logMessage(QStringLiteral("Too many invalid attempts. %1 automatically loses!").arg(loser));
        return;
    }

    const int maxDelta = result.deltas.empty()
                             ? 0
                             : *std::max_element(result.deltas.begin(), result.deltas.end());

    if (maxDelta == 1) {
        emit logMessage(QStringLiteral("It's a draw!"));
        return;
    }

    QStringList winners;
    for (size_t i = 0; i < result.deltas.size(); ++i)
        if (result.deltas[i] == maxDelta) winners << q(names[i]);

    if (winners.size() == 1)
        emit logMessage(winners.front() + QStringLiteral(" wins the round!"));
    else
        emit logMessage(QStringLiteral("Tied between: ") + winners.join(QStringLiteral(" and "))
                        + QStringLiteral(" - deciding match required!"));
}
