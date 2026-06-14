#pragma once
#include "Protocol.h"
#include <QAbstractSocket>
#include <QObject>
#include <QString>
#include <QStringList>

class QTcpSocket;

// Client end of the host/client link. Connects to a host, performs the hello
// handshake, and turns incoming protocol messages into Qt signals that mirror the
// host's view events (so a NetworkClientSession can forward them to the GameWindow
// unchanged). Outbound, it sends the local player's move when asked.
class NetworkClient : public QObject {
    Q_OBJECT

public:
    explicit NetworkClient(QObject* parent = nullptr);

    void connectToHost(const QString& host, quint16 port, const QString& playerName);
    void sendMove(const QString& move);
    void disconnectFromHost();
    [[nodiscard]] bool isConnected() const;

signals:
    // Lobby phase.
    void welcomed(QStringList roster);       // welcome received from host
    void rosterChanged(QStringList names);   // lobby roster updated
    void gameStarted();                      // host began the game

    // In-game (mirror IGameSession / QtView).
    void movePromptRequested();              // your_turn
    void logMessage(QString text);
    void scoresUpdated(QStringList names, QStringList scores);
    void roundFinished();
    void gameFinished(QStringList lines);

    // Transport.
    void disconnected();
    void errorOccurred(QString message);

private slots:
    void onConnected();
    void onReadyRead();
    void onSocketError(QAbstractSocket::SocketError error);

private:
    void handle(const QJsonObject& msg);

    QTcpSocket*           mSocket;
    Protocol::FrameReader mReader;
    QString               mPlayerName;
};
