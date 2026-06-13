#include "GameController.h"
#include "RandomBot.h"
#include "ZhejiangBot.h"
#include <algorithm>
#include <chrono>
#include <future>
#include <random>
#include <thread>

namespace {
void SimulateBotThinking() {
    thread_local std::mt19937 rng{ std::random_device{}() };
    std::uniform_int_distribution<int> dist(500, 2499);
    std::this_thread::sleep_for(std::chrono::milliseconds(dist(rng)));
}
} // namespace

GameController::GameController(std::string name, int opponents,
                                IGameView& view, std::istream& in,
                                bool withZhejiangBot)
    : playerName(std::move(name))
    , opponents(static_cast<size_t>(std::clamp(opponents, 1, maxOpponents)))
    , view(view)
    , in(in)
{
    botScores.resize(this->opponents, 0);
    bots.reserve(this->opponents);
    botNames.reserve(this->opponents);

    if (withZhejiangBot) {
        bots.emplace_back(std::make_unique<ZhejiangBot>());
        botNames.push_back("Zhejiang Bot");
    }

    const size_t randomBotCount = withZhejiangBot ? this->opponents - 1 : this->opponents;
    for (size_t i = 0; i < randomBotCount; ++i) {
        bots.emplace_back(std::make_unique<RandomBot>());
        botNames.push_back("Opponent " + std::to_string(i + 1));
    }
}

Move GameController::ReadPlayerMove()
{
    view.ShowMovePrompt();

    for (int remaining = 3; remaining > 0; --remaining)
    {
        std::string input;
        if (!std::getline(in, input)) break;

        switch (!input.empty() ? input[0] : '\0')
        {
        case 'r': return Move::Rock;
        case 'p': return Move::Paper;
        case 's': return Move::Scissors;
        default:  break;
        }

        if (remaining > 1)
            view.ShowInvalidInput(remaining - 1);
    }

    return Move::Invalid;
}

void GameController::PlayRound()
{
    std::vector<std::future<Move>> botFutures;
    botFutures.reserve(opponents);
    for (size_t i = 0; i < opponents; ++i) {
        botFutures.emplace_back(std::async(std::launch::async, [this, i]() -> Move {
            SimulateBotThinking();
            const Move choice = bots[i]->PickMove();
            view.ShowBotReady(botNames[i]);
            return choice;
        }));
    }

    // Block until every bot has decided — player input opens after all are ready.
    std::vector<Move> botMoves;
    botMoves.reserve(opponents);
    for (auto& f : botFutures)
        botMoves.push_back(f.get());

    const Move playerMove = ReadPlayerMove();

    const RoundResult result = GameEngine::EvaluateRound(playerMove, botMoves);

    // Skip move display on forfeit — '?' would be confusing; ShowRoundResult covers the outcome.
    if (playerMove != Move::Invalid) {
        view.ShowRoundMoves(playerName, playerMove, botNames, botMoves);
    }

    view.ShowRoundResult(playerName, botNames, result);

    playerScore += result.deltas[0];
    for (size_t i = 0; i < botMoves.size(); ++i) {
        botScores[i] += result.deltas[i + 1];
        bots[i]->NotifyResult(result.deltas[i + 1]);
    }
}

RoundResult GameController::PlayTournamentRound(const std::vector<TournamentParticipant>& participants)
{
    std::vector<Move> moves(participants.size(), Move::Invalid);

    int humanIdx = -1;
    for (int i = 0; i < static_cast<int>(participants.size()); ++i)
        if (participants[i].isHuman) { humanIdx = i; break; }

    if (humanIdx >= 0) {
        struct BotResult { size_t idx; Move move; };
        std::vector<std::future<BotResult>> botFutures;
        botFutures.reserve(participants.size() - 1);

        for (size_t i = 0; i < participants.size(); ++i) {
            if (participants[i].isHuman) continue;
            const std::string botName = participants[i].name;
            auto strategy = participants[i].strategy;
            botFutures.push_back(std::async(std::launch::async,
                [this, i, botName, strategy]() -> BotResult {
                    SimulateBotThinking();
                    Move choice = strategy->PickMove();
                    view.ShowTournamentBotReady(botName);
                    return {i, choice};
                }));
        }

        for (auto& f : botFutures) {
            auto res = f.get();
            moves[res.idx] = res.move;
        }

        moves[static_cast<size_t>(humanIdx)] = ReadPlayerMove();

    } else {
        for (size_t i = 0; i < participants.size(); ++i)
            moves[i] = participants[i].strategy->PickMove();
    }

    std::vector<std::string> names;
    names.reserve(participants.size());
    for (const auto& p : participants) names.push_back(p.name);

    // Human forfeit: build result manually so Move::Invalid never reaches EvaluateRound
    const bool humanForfeited = (humanIdx >= 0) &&
                                (moves[static_cast<size_t>(humanIdx)] == Move::Invalid);

    RoundResult result;
    if (humanForfeited) {
        result.outcome = RoundResult::Outcome::InvalidInput;
        result.moves   = moves;
        result.deltas.assign(participants.size(), 2);        // everyone wins...
        result.deltas[static_cast<size_t>(humanIdx)] = 0;    // ...except the forfeiter
    } else {
        // moves[0] maps to EvaluateRound's playerMove param; result deltas are purely positional.
        const std::vector<Move> botMoves(moves.begin() + 1, moves.end());
        result = GameEngine::EvaluateRound(moves[0], botMoves);
    }

    view.ShowTournamentMoves(names, moves);
    view.ShowTournamentResult(names, result);

    for (size_t i = 0; i < participants.size(); ++i)
        if (!participants[i].isHuman && participants[i].strategy)
            participants[i].strategy->NotifyResult(result.deltas[i]);

    return result;
}

void GameController::ShowResults() const
{
    view.ShowFinalScores(playerName, playerScore, botNames, botScores);
}

int GameController::GetPlayerScore() const { return playerScore; }
const std::vector<int>& GameController::GetBotScores() const { return botScores; }
