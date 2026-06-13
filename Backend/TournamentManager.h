#pragma once
#include "TournamentParticipant.h"
#include <IGameView.h>
#include <istream>
#include <random>
#include <string>
#include <vector>

class GameController;

class TournamentManager {
    std::string humanName;
    int botCount;
    bool withZhejiangBot;
    IGameView& view;
    std::istream& in;
    mutable std::mt19937 rng;

    using Group   = std::vector<TournamentParticipant>;
    using Bracket = std::vector<Group>;

    Bracket    CreateBracket(std::vector<TournamentParticipant> participants) const;
    TournamentParticipant RunMatch(const Group& group, GameController& ctrl);

public:
    TournamentManager(std::string humanName, int botCount,
                      IGameView& view, std::istream& in,
                      bool withZhejiangBot = false);
    void RunTournament();
};
