#include "GameWorker.h"
#include "MoveStream.h"
#include <GameController.h>
#include <TournamentManager.h>
#include <string>

GameWorker::GameWorker(Config config, IGameView& view, MoveStream& stream, QObject* parent)
    : QObject(parent)
    , mMode(config.tournamentMode ? Mode::Tournament : Mode::Regular)
    , mConfig(std::move(config))
    , mView(view)
    , mStream(stream) {}

GameWorker::GameWorker(std::string playerName,
                       std::vector<std::unique_ptr<IBotStrategy>> participants,
                       std::vector<std::string> participantNames,
                       IGameView& view, MoveStream& stream, QObject* parent)
    : QObject(parent)
    , mMode(Mode::Host)
    , mHostPlayerName(std::move(playerName))
    , mHostParticipants(std::move(participants))
    , mHostNames(std::move(participantNames))
    , mView(view)
    , mStream(stream) {}

GameWorker::~GameWorker() = default;

void GameWorker::run()
{
    switch (mMode) {
    case Mode::Tournament: {
        // Tournament has no injected/remote participants, so a local manager is fine.
        TournamentManager tournament(mConfig.playerName, mConfig.opponents,
                                     mView, mStream, mConfig.withZhejiangBot);
        tournament.RunTournament();
        break;
    }
    case Mode::Host:
        mController = std::make_unique<GameController>(
            std::move(mHostPlayerName), std::move(mHostParticipants),
            std::move(mHostNames), mView, mStream);
        runRegularLoop(*mController);
        break;
    case Mode::Regular:
        mController = std::make_unique<GameController>(
            mConfig.playerName, mConfig.opponents, mView, mStream, mConfig.withZhejiangBot);
        runRegularLoop(*mController);
        break;
    }

    // NB: mController is intentionally NOT reset here — it (and any remote-player
    // strategies it owns) must outlive the worker thread; ~GameWorker frees it.
    emit finished();
}

void GameWorker::runRegularLoop(GameController& controller)
{
    // Mirrors the old console loop: an empty line starts a round, "exit" (or EOF
    // from MoveStream::Close) ends the session. The GUI pushes "" when "Start
    // round" is clicked and "r"/"p"/"s" for the move itself.
    std::string line;
    while (std::getline(mStream, line)) {
        if (line == "exit") break;
        if (line.empty()) controller.PlayRound();
    }

    controller.ShowResults();
}
