#include "NetworkServer.h"
#include "RemotePlayerStrategy.h"
#include <Move.h>
#include <QTcpServer>
#include <QTcpSocket>

namespace {
constexpr char kCidProperty[] = "cid";   // client id stashed on each socket

Move parseMove(const QString& s)
{
    if (s.isEmpty()) return Move::Invalid;
    switch (s.at(0).toLatin1()) {
    case 'r': return Move::Rock;
    case 'p': return Move::Paper;
    case 's': return Move::Scissors;
    default:  return Move::Invalid;
    }
}
} // namespace

NetworkServer::NetworkServer(QObject* parent)
    : QObject(parent), mServer(new QTcpServer(this))
{
    connect(mServer, &QTcpServer::newConnection, this, &NetworkServer::onNewConnection);
}

NetworkServer::~NetworkServer() = default;

bool NetworkServer::listen(quint16 port)
{
    return mServer->listen(QHostAddress::Any, port);
}

quint16 NetworkServer::port() const
{
    return mServer->serverPort();
}

QStringList NetworkServer::roster() const
{
    QStringList names;
    for (const auto& [id, c] : mClients)
        names << (c.name.isEmpty() ? QStringLiteral("(connecting…)") : c.name);
    return names;
}

int NetworkServer::clientCount() const
{
    return static_cast<int>(mClients.size());
}

void NetworkServer::onNewConnection()
{
    while (mServer->hasPendingConnections()) {
        QTcpSocket* socket = mServer->nextPendingConnection();

        if (mInGame) {                    // late joiners are not supported
            socket->disconnectFromHost();
            socket->deleteLater();
            continue;
        }

        const int id = mNextId++;
        socket->setProperty(kCidProperty, id);
        mClients[id].socket = socket;

        connect(socket, &QTcpSocket::readyRead,    this, &NetworkServer::onReadyRead);
        connect(socket, &QTcpSocket::disconnected, this, &NetworkServer::onDisconnected);
    }
}

void NetworkServer::onReadyRead()
{
    auto* socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;
    const int id = socket->property(kCidProperty).toInt();
    Client* c = find(id);
    if (!c) return;

    c->reader.append(socket->readAll());
    while (auto msg = c->reader.next())
        handleMessage(id, *msg);
}

void NetworkServer::handleMessage(int id, const QJsonObject& msg)
{
    const QString type = Protocol::typeOf(msg);

    if (type == QLatin1String(Protocol::Type::Hello)) {
        if (Client* c = find(id)) {
            c->name = msg.value(QStringLiteral("name")).toString();
            if (c->name.isEmpty()) c->name = QStringLiteral("Player %1").arg(id);
            sendTo(id, Protocol::welcome(id, roster()));
            broadcastRoster();
        }
    } else if (type == QLatin1String(Protocol::Type::Move)) {
        if (!mInGame) return;
        if (Client* c = find(id); c && c->strategy)
            c->strategy->SupplyMove(parseMove(msg.value(QStringLiteral("move")).toString()));
    }
}

NetworkServer::StartedGame NetworkServer::startGame()
{
    mInGame = true;
    mServer->pauseAccepting();

    StartedGame out;
    for (auto& [id, c] : mClients) {
        auto strategy = std::make_unique<RemotePlayerStrategy>();
        const int cid = id;
        // Called from the worker thread inside PickMove(); marshal onto our thread.
        strategy->SetOnTurnRequested([this, cid] {
            QMetaObject::invokeMethod(this, [this, cid] { sendYourTurn(cid); },
                                      Qt::QueuedConnection);
        });
        c.strategy = strategy.get();
        out.remotePlayers.push_back(std::move(strategy));
        out.remoteNames.push_back((c.name.isEmpty() ? QStringLiteral("Player %1").arg(id)
                                                     : c.name).toStdString());
    }

    broadcast(Protocol::gameStarted());   // tell clients to leave the lobby
    return out;
}

void NetworkServer::stopGame()
{
    mInGame = false;
    for (auto& [id, c] : mClients) {
        if (c.strategy) {
            c.strategy->Disconnect();   // unblock any in-flight PickMove
            c.strategy = nullptr;       // GameController is about to delete it
        }
    }
}

void NetworkServer::broadcastLog(const QString& text)
{
    broadcast(Protocol::log(text));
}

void NetworkServer::broadcastScores(const QStringList& names, const QStringList& scores)
{
    broadcast(Protocol::scores(names, scores));
}

void NetworkServer::broadcastRoundFinished()
{
    broadcast(Protocol::roundFinished());
}

void NetworkServer::broadcastGameFinished(const QStringList& lines)
{
    broadcast(Protocol::gameFinished(lines));
}

void NetworkServer::onDisconnected()
{
    auto* socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;
    const int id = socket->property(kCidProperty).toInt();

    if (Client* c = find(id); c && c->strategy)
        c->strategy->Disconnect();      // forfeit so the round can resolve

    mClients.erase(id);
    socket->deleteLater();

    if (!mInGame)
        broadcastRoster();
}

void NetworkServer::sendTo(int id, const QJsonObject& msg)
{
    if (Client* c = find(id); c && c->socket)
        c->socket->write(Protocol::encode(msg));
}

void NetworkServer::broadcast(const QJsonObject& msg)
{
    const QByteArray bytes = Protocol::encode(msg);
    for (auto& [id, c] : mClients)
        if (c.socket) c.socket->write(bytes);
}

void NetworkServer::sendYourTurn(int id)
{
    sendTo(id, Protocol::yourTurn());
}

void NetworkServer::broadcastRoster()
{
    const QStringList names = roster();
    broadcast(Protocol::lobby(names));
    emit rosterChanged(names);
}

NetworkServer::Client* NetworkServer::find(int id)
{
    auto it = mClients.find(id);
    return it == mClients.end() ? nullptr : &it->second;
}
