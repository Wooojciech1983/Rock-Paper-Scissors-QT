#pragma once
#include <istream>
#include <ostream>
#include <string>

class AppSession {
    std::istream& in;
    std::ostream& out;

    std::string ReadPlayerName();
    int         ReadOpponentCount();
    bool        AskZhejiangBot();
    bool        AskTournamentMode();
    void        RunTournamentMode(const std::string& playerName, int opponents, bool withZhejiangBot);
    void        RunRegularMode(const std::string& playerName, int opponents, bool withZhejiangBot);

public:
    AppSession(std::istream& in, std::ostream& out);
    void Run();
};
