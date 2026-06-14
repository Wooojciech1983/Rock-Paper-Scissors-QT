#include "HostGameSession.h"
#include "NetworkServer.h"
#include <QtView.h>
#include <GameWorker.h>
#include <RandomBot.h>
#include <ZhejiangBot.h>
#include <QThread>
#include <utility>

HostGameSession::HostGameSession(Config config, NetworkServer* server, QObject* parent)
    : IGameSession(parent), mConfig(std::move(config)), mServer(server)
{
    if (mServer) mServer->setParent(this);   // session now owns the server
}

HostGameSession::~HostGameSession()
{
    stopWorker();
}

void HostGameSession::start()
{
    mView = new QtView(this);

    // Forward to the host's own GameWindow (includes the local move prompt).
    connect(mView, &QtView::logMessage,          this, &IGameSession::logMessage);
    connect(mView, &QtView::movePromptRequested, this, &IGameSession::movePromptRequested);
    connect(mView, &QtView::scoresUpdated,       this, &IGameSession::scoresUpdated);
    connect(mView, &QtView::roundFinished,       this, &IGameSession::roundFinished);
    connect(mView, &QtView::gameFinished,        this, &IGameSession::gameFinished);

    // Broadcast the same view events to every client (movePrompt stays host-local;
    // each client is cued individually via the server's "your_turn").
    connect(mView, &QtView::logMessage,    mServer, &NetworkServer::broadcastLog);
    connect(mView, &QtView::scoresUpdated, mServer, &NetworkServer::broadcastScores);
    connect(mView, &QtView::roundFinished, mServer, &NetworkServer::broadcastRoundFinished);
    connect(mView, &QtView::gameFinished,  mServer, &NetworkServer::broadcastGameFinished);

    // Remote players become participants; append the AI bots requested at setup.
    NetworkServer::StartedGame started = mServer->startGame();
    std::vector<std::unique_ptr<IBotStrategy>> participants = std::move(started.remotePlayers);
    std::vector<std::string> names = std::move(started.remoteNames);

    if (mConfig.withZhejiangBot) {
        participants.push_back(std::make_unique<ZhejiangBot>());
        names.emplace_back("Zhejiang Bot");
    }
    for (int i = 0; i < mConfig.aiBots; ++i) {
        participants.push_back(std::make_unique<RandomBot>());
        names.push_back("Bot " + std::to_string(i + 1));
    }

    mThread = new QThread;
    mWorker = new GameWorker(mConfig.playerName, std::move(participants), std::move(names),
                             *mView, mStream);
    mWorker->moveToThread(mThread);

    connect(mThread, &QThread::started, mWorker, &GameWorker::run);
    connect(mWorker, &GameWorker::finished, mThread, &QThread::quit, Qt::DirectConnection);

    mThread->start();
}

void HostGameSession::stopWorker()
{
    if (!mThread) return;

    // Tear down server routing first: forfeits remote players (unblocking a
    // PickMove waiting on a client) and drops the raw strategy pointers BEFORE the
    // GameController that owns those strategies is destroyed with the worker.
    if (mServer) mServer->stopGame();

    mStream.Close();      // EOF unblocks the host's local getline
    mThread->quit();
    mThread->wait();

    delete mWorker;
    mWorker = nullptr;
    delete mThread;
    mThread = nullptr;
}

void HostGameSession::requestStartRound()
{
    mStream.PushLine("");
}

void HostGameSession::submitMove(const QString& move)
{
    mStream.PushLine(move.toStdString());
}

void HostGameSession::requestFinish()
{
    mStream.PushLine("exit");
}
