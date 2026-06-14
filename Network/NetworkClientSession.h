#pragma once
#include <IGameSession.h>

class NetworkClient;

// IGameSession for a network client. There is no GameController here — the host is
// authoritative. It forwards the client's incoming view-event signals to the
// GameWindow and sends the local player's move back. Rounds are host-driven
// (Kind::Client), so there is no "Start round" button.
//
// Takes over an already-connected NetworkClient (the lobby established it).
class NetworkClientSession : public IGameSession {
    Q_OBJECT

public:
    explicit NetworkClientSession(NetworkClient* client, QObject* parent = nullptr);

    [[nodiscard]] Kind kind() const override { return Kind::Client; }

public slots:
    void start() override;
    void requestStartRound() override {}             // host drives rounds
    void submitMove(const QString& move) override;
    void requestFinish() override;

private:
    NetworkClient* mClient = nullptr;   // owned (re-parented to this)
    bool mEnded = false;
};
