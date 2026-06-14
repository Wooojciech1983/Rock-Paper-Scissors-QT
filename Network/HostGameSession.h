#pragma once
#include <IGameSession.h>
#include <MoveStream.h>
#include <memory>
#include <string>

class QtView;
class QThread;
class GameWorker;
class NetworkServer;

// IGameSession for the authoritative host. Like LocalGameSession it owns a QtView,
// a MoveStream and a GameWorker on a QThread, but its participants are the remote
// players (RemotePlayerStrategy, supplied by the server) plus AI bots, and the
// QtView's view events are additionally broadcast to all clients via the server.
//
// Takes over an already-listening NetworkServer whose lobby clients become the
// remote players. Round flow is host-driven (Kind::Host): the host's local human
// clicks "Start round".
class HostGameSession : public IGameSession {
    Q_OBJECT

public:
    struct Config {
        std::string playerName = "Host";   // the host's local human
        int  aiBots = 0;                    // extra AI opponents to add
        bool withZhejiangBot = false;
    };

    HostGameSession(Config config, NetworkServer* server, QObject* parent = nullptr);
    ~HostGameSession() override;

    [[nodiscard]] Kind kind() const override { return Kind::Host; }

public slots:
    void start() override;
    void requestStartRound() override;
    void submitMove(const QString& move) override;
    void requestFinish() override;

private:
    void stopWorker();

    Config         mConfig;
    NetworkServer* mServer = nullptr;   // owned (re-parented to this)
    MoveStream     mStream;             // declared first → destroyed last
    QtView*        mView   = nullptr;   // child of this
    QThread*       mThread = nullptr;   // owned manually (joined in stopWorker)
    GameWorker*    mWorker = nullptr;   // owned manually
};
