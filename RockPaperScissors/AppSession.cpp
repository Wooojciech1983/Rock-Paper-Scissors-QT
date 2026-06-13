#include "AppSession.h"
#include <ConsoleView.h>
#include <GameController.h>
#include <TournamentManager.h>

AppSession::AppSession(std::istream& in, std::ostream& out)
    : in(in), out(out) {}

std::string AppSession::ReadPlayerName()
{
    out << "Enter player name:\n";
    std::string name;
    std::getline(in, name);
    if (name.empty()) {
        name = "Player";
        out << "No name entered, using \"Player\".\n";
    }
    return name;
}

int AppSession::ReadOpponentCount()
{
    out << "Enter number of AI players (1-" << GameController::maxOpponents << ", default 1):\n";
    std::string input;
    std::getline(in, input);
    if (input.empty()) return 1;

    try {
        const int n = std::stoi(input);
        if (n >= 1 && n <= GameController::maxOpponents) return n;
    } catch (...) {}

    out << "Invalid input, using 1 opponent.\n";
    return 1;
}

bool AppSession::AskZhejiangBot()
{
    out << "Would you like to challenge the Zhejiang Bot? (y/n)\n";
    std::string answer;
    std::getline(in, answer);
    return !answer.empty() && (answer[0] == 'y' || answer[0] == 'Y');
}

bool AppSession::AskTournamentMode()
{
    out << "Would you like to play tournament mode? (y/n)\n";
    std::string answer;
    std::getline(in, answer);
    return !answer.empty() && (answer[0] == 'y' || answer[0] == 'Y');
}

void AppSession::RunTournamentMode(const std::string& playerName, int opponents, bool withZhejiangBot)
{
    ConsoleView view(out);
    TournamentManager tournament(playerName, opponents, view, in, withZhejiangBot);
    tournament.RunTournament();
}

void AppSession::RunRegularMode(const std::string& playerName, int opponents, bool withZhejiangBot)
{
    ConsoleView view(out);
    GameController controller(playerName, opponents, view, in, withZhejiangBot);

    while (true) {
        out << "\nPress Enter to start a round or type \"exit\" to quit the game.\n";
        std::string input;
        if (!std::getline(in, input))
            break;

        if (input.empty())
            controller.PlayRound();
        else if (input == "exit")
            break;
    }

    controller.ShowResults();
}

void AppSession::Run()
{
    const std::string playerName = ReadPlayerName();
    const int opponents = ReadOpponentCount();
    const bool withZhejiangBot = AskZhejiangBot();

    if (opponents >= 3 && AskTournamentMode())
        RunTournamentMode(playerName, opponents, withZhejiangBot);
    else
        RunRegularMode(playerName, opponents, withZhejiangBot);

    out << "\nPress Enter to close.\n";
    std::string dummy;
    std::getline(in, dummy);
}
