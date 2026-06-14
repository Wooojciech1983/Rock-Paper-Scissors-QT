#pragma once
#include "Protocol.h"
#include <IBotStrategy.h>
#include <QObject>
#include <QStringList>
#include <memory>
#include <unordered_map>
#include <vector>

class QTcpServer;
class QTcpSocket;
class RemotePlayerStrategy;

// Authoritative host transport. During the lobby it accepts TCP clients, performs
// the hello/welcome handshake and tracks the roster. On startGame() it mints a
// RemotePlayerStrategy per connected client (returned as GameController
// participants; the server keeps raw pointers to route moves) and wires each
// strategy's turn request to a "your_turn" sent to that client. During play it
// broadcasts the host's view events to every client and forwards incoming moves
// to the matching strategy.
class NetworkServer : public QObject {
    Q_OBJECT

public:
    explicit NetworkServer(QObject* parent = nullptr);
    ~NetworkServer() override;

    bool listen(quint16 port);
    [[nodiscard]] quint16 port() const;
    [[nodiscard]] QStringList roster() const;   // connected client names
    [[nodiscard]] int clientCount() const;

    struct StartedGame {
        std::vector<std::unique_ptr<IBotStrategy>> remotePlayers;
        std::vector<std::string>                   remoteNames;
    };
    // Lobby → in-game. Builds a strategy per client and returns them as
    // participants; further lobby joins are refused afterwards.
    StartedGame startGame();

    // Stops routing and forfeits every remote player (unblocking a worker that is
    // waiting on one), then drops the raw strategy pointers. MUST be called before
    // the GameController that owns those strategies is destroyed.
    void stopGame();

public slots:
    // Connected to the host QtView signals; serialized and sent to all clients.
    void broadcastLog(const QString& text);
    void broadcastScores(const QStringList& names, const QStringList& scores);
    void broadcastRoundFinished();
    void broadcastGameFinished(const QStringList& lines);

signals:
    void rosterChanged(QStringList names);

private slots:
    void onNewConnection();
    void onReadyRead();
    void onDisconnected();

private:
    struct Client {
        QTcpSocket*           socket   = nullptr;
        QString               name;
        Protocol::FrameReader reader;
        RemotePlayerStrategy* strategy = nullptr;   // set at startGame; not owned
    };

    void handleMessage(int id, const QJsonObject& msg);
    void sendTo(int id, const QJsonObject& msg);
    void broadcast(const QJsonObject& msg);
    void sendYourTurn(int id);          // marshalled here from a worker thread
    void broadcastRoster();
    Client* find(int id);

    QTcpServer* mServer = nullptr;
    std::unordered_map<int, Client> mClients;
    int  mNextId = 1;
    bool mInGame = false;
};
