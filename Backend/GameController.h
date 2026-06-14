#pragma once
#include "GameEngine.h"
#include "IBotStrategy.h"
#include "TournamentParticipant.h"
#include <IGameView.h>
#include <istream>
#include <memory>
#include <string>
#include <vector>

class GameController {
    std::string mPlayerName;
    size_t mOpponents;
    int mPlayerScore = 0;
    std::vector<int> mBotScores;
    std::vector<std::string> mBotNames;
    std::vector<std::unique_ptr<IBotStrategy>> mBots;

    IGameView& mView;
    std::istream& mIn;

    Move ReadPlayerMove();

public:
    static constexpr int maxOpponents = 100;

    // withZhejiangBot: if true, the first opponent uses the Zhejiang win-stay/lose-shift strategy.
    GameController(std::string name, int opponents,
                   IGameView& view, std::istream& in,
                   bool withZhejiangBot = false);

    // Injection constructor for the network host: the caller supplies ready-made
    // participant strategies (remote players + AI bots) and their display names,
    // positionally aligned. PlayRound()/ReadPlayerMove()/ShowResults() are
    // unchanged — they operate on these members regardless of who built them.
    GameController(std::string name,
                   std::vector<std::unique_ptr<IBotStrategy>> participants,
                   std::vector<std::string> participantNames,
                   IGameView& view, std::istream& in);

    void PlayRound();
    void ShowResults() const;

    // Plays one round between the given participants (2 or 3 people).
    // Bots pick randomly; if a human is present they are prompted for input.
    // Does NOT update the session scores. Returns the raw RoundResult.
    RoundResult PlayTournamentRound(const std::vector<TournamentParticipant>& participants);

    [[nodiscard]] int GetPlayerScore() const;
    [[nodiscard]] const std::vector<int>& GetBotScores() const;
};
