#pragma once
#include "IGameSession.h"
#include "MoveStream.h"
#include "GameWorker.h"

class QtView;
class QThread;

// IGameSession backed by the real backend: owns a QtView and a MoveStream and
// runs a GameWorker (GameController or TournamentManager) on a dedicated QThread.
// The MoveStream bridges non-blocking GUI intent into the worker's blocking
// getline; QtView's queued signals are forwarded out as IGameSession signals.
class LocalGameSession : public IGameSession {
    Q_OBJECT

public:
    explicit LocalGameSession(GameWorker::Config config, QObject* parent = nullptr);
    ~LocalGameSession() override;

    [[nodiscard]] Kind kind() const override {
        return mConfig.tournamentMode ? Kind::Tournament : Kind::Local;
    }

public slots:
    void start() override;
    void requestStartRound() override;
    void submitMove(const QString& move) override;
    void requestFinish() override;

private slots:
    // Intercepts the tournament "press Enter" pause (there is no console here) and
    // forwards every other line to logMessage unchanged.
    void onViewLog(const QString& text);

private:
    void stopWorker();

    GameWorker::Config mConfig;
    MoveStream  mStream;            // declared first → destroyed last (after the worker is joined)
    QtView*     mView   = nullptr;  // child of this
    QThread*    mThread = nullptr;  // owned manually (joined in stopWorker)
    GameWorker* mWorker = nullptr;  // owned manually
};
