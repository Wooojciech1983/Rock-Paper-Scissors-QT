#include "LocalGameSession.h"
#include "QtView.h"
#include <QThread>

namespace {
// TournamentManager emits exactly this and then blocks on getline before each
// human round. With no console to press Enter on, the session auto-advances.
constexpr char kTournamentContinuePrompt[] = "Press Enter to play the round.";
} // namespace

LocalGameSession::LocalGameSession(GameWorker::Config config, QObject* parent)
    : IGameSession(parent), mConfig(std::move(config)) {}

LocalGameSession::~LocalGameSession()
{
    stopWorker();
}

void LocalGameSession::start()
{
    mView = new QtView(this);
    connect(mView, &QtView::logMessage,          this, &LocalGameSession::onViewLog);
    connect(mView, &QtView::movePromptRequested, this, &IGameSession::movePromptRequested);
    connect(mView, &QtView::scoresUpdated,       this, &IGameSession::scoresUpdated);
    connect(mView, &QtView::roundFinished,       this, &IGameSession::roundFinished);
    connect(mView, &QtView::gameFinished,        this, &IGameSession::gameFinished);

    mThread = new QThread;
    mWorker = new GameWorker(mConfig, *mView, mStream);
    mWorker->moveToThread(mThread);

    connect(mThread, &QThread::started, mWorker, &GameWorker::run);
    // Direct so the thread's event loop exits even while the GUI thread is blocked
    // inside stopWorker()'s wait().
    connect(mWorker, &GameWorker::finished, mThread, &QThread::quit, Qt::DirectConnection);

    mThread->start();
}

void LocalGameSession::stopWorker()
{
    if (!mThread) return;

    mStream.Close();      // EOF unblocks any pending getline in the worker
    mThread->quit();
    mThread->wait();

    delete mWorker;
    mWorker = nullptr;
    delete mThread;
    mThread = nullptr;
}

void LocalGameSession::requestStartRound()
{
    mStream.PushLine("");   // empty line starts a round in the regular loop
}

void LocalGameSession::submitMove(const QString& move)
{
    mStream.PushLine(move.toStdString());
}

void LocalGameSession::requestFinish()
{
    // Regular loop: "exit" ends it → ShowResults → gameFinished. Tournament has no
    // such command; closing the window tears the worker down via the destructor.
    if (!mConfig.tournamentMode)
        mStream.PushLine("exit");
}

void LocalGameSession::onViewLog(const QString& text)
{
    if (mConfig.tournamentMode && text == QLatin1String(kTournamentContinuePrompt)) {
        mStream.PushLine("");
        return;
    }
    emit logMessage(text);
}
