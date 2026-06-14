#include "TournamentManager.h"
#include "GameController.h"
#include "RandomBot.h"
#include "ZhejiangBot.h"
#include <algorithm>
#include <random>
#include <string>

TournamentManager::TournamentManager(std::string humanName, int botCount,
                                     IGameView& view, std::istream& in,
                                     bool withZhejiangBot)
    : mHumanName(std::move(humanName)), mBotCount(botCount), mWithZhejiangBot(withZhejiangBot)
    , mView(view), mIn(in), mRng(std::random_device{}()) {}

TournamentManager::Bracket TournamentManager::CreateBracket(
    std::vector<TournamentParticipant> participants) const
{
    std::shuffle(participants.begin(), participants.end(), mRng);

    Bracket bracket;
    size_t i = 0;
    const size_t n = participants.size();

    while (i < n) {
        const size_t remaining = n - i;
        // Threshold 3: collect the last 2 or 3 as one group — avoids leaving a lone participant.
        if (remaining <= 3) {
            bracket.emplace_back(participants.begin() + i, participants.end());
            break;
        }
        bracket.push_back({ participants[i], participants[i + 1] });
        i += 2;
    }

    return bracket;
}

TournamentParticipant TournamentManager::RunMatch(const Group& group, GameController& ctrl)
{
    auto containsHuman = [](const Group& g) {
        return std::any_of(g.begin(), g.end(), [](const TournamentParticipant& p){ return p.isHuman; });
    };
    Group current = group;
    bool hasHuman = containsHuman(current);

    while (true) {
        if (hasHuman) {
            mView.ShowMessage("Press Enter to play the round.");
            std::string dummy;
            std::getline(mIn, dummy);
        }

        const RoundResult result = ctrl.PlayTournamentRound(current);

        const int maxDelta = *std::max_element(result.deltas.begin(), result.deltas.end());

        if (maxDelta == 1) continue; // draw — repeat round

        std::vector<size_t> winnerIndices;
        for (size_t i = 0; i < result.deltas.size(); ++i)
            if (result.deltas[i] == maxDelta) winnerIndices.push_back(i);

        if (winnerIndices.size() == 1)
            return current[winnerIndices[0]];

        // Two co-winners from a 3-person group: narrow the group and replay
        Group subGroup;
        for (size_t idx : winnerIndices) subGroup.push_back(current[idx]);

        std::vector<std::string> subNames;
        for (const auto& p : subGroup) subNames.push_back(p.name);
        mView.ShowSubMatchStart(subNames);

        hasHuman = containsHuman(subGroup);
        current = std::move(subGroup);
    }
}

void TournamentManager::RunTournament()
{
    std::vector<TournamentParticipant> all;
    all.push_back({ .name = mHumanName, .isHuman = true, .strategy = nullptr });

    if (mWithZhejiangBot)
        all.push_back({ .name = "Zhejiang Bot", .isHuman = false, .strategy = std::make_shared<ZhejiangBot>() });

    const int randomBotCount = mWithZhejiangBot ? mBotCount - 1 : mBotCount;
    for (int i = 1; i <= randomBotCount; ++i)
        all.push_back({ .name = "Bot " + std::to_string(i), .isHuman = false, .strategy = std::make_shared<RandomBot>() });

    // GameController is used solely for PlayTournamentRound (view + input access)
    GameController ctrl(mHumanName, mBotCount, mView, mIn);

    std::vector<TournamentParticipant> current = all;
    int roundNum = 0;

    while (current.size() > 1) {
        ++roundNum;
        mView.ShowTournamentRoundStart(roundNum);

        const Bracket bracket = CreateBracket(current);
        std::vector<TournamentParticipant> nextRound;

        for (const Group& group : bracket) {
            std::vector<std::string> names;
            for (const auto& p : group) names.push_back(p.name);
            mView.ShowMatchStart(names);

            const TournamentParticipant winner = RunMatch(group, ctrl);
            mView.ShowMatchResult(winner.name);
            nextRound.push_back(winner);
        }

        current = nextRound;
    }

    mView.ShowTournamentWinner(current[0].name);
}
