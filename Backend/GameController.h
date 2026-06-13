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
    std::string playerName;
    size_t opponents;
    int playerScore = 0;
    std::vector<int> botScores;
    std::vector<std::string> botNames;
    std::vector<std::unique_ptr<IBotStrategy>> bots;

    IGameView& view;
    std::istream& in;

    Move ReadPlayerMove();

public:
    static constexpr int maxOpponents = 100;

    // withZhejiangBot: if true, the first opponent uses the Zhejiang win-stay/lose-shift strategy.
    GameController(std::string name, int opponents,
                   IGameView& view, std::istream& in,
                   bool withZhejiangBot = false);

    void PlayRound();
    void ShowResults() const;

    // Plays one round between the given participants (2 or 3 people).
    // Bots pick randomly; if a human is present they are prompted for input.
    // Does NOT update the session scores. Returns the raw RoundResult.
    RoundResult PlayTournamentRound(const std::vector<TournamentParticipant>& participants);

    [[nodiscard]] int GetPlayerScore() const;
    [[nodiscard]] const std::vector<int>& GetBotScores() const;
};
