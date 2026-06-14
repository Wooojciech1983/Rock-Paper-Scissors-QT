#include "NetworkClient.h"
#include <QJsonArray>
#include <QTcpSocket>

namespace {
QStringList toStringList(const QJsonArray& arr)
{
    QStringList out;
    for (const auto& v : arr) out << v.toString();
    return out;
}
} // namespace

NetworkClient::NetworkClient(QObject* parent)
    : QObject(parent), mSocket(new QTcpSocket(this))
{
    connect(mSocket, &QTcpSocket::connected,    this, &NetworkClient::onConnected);
    connect(mSocket, &QTcpSocket::readyRead,    this, &NetworkClient::onReadyRead);
    connect(mSocket, &QTcpSocket::disconnected, this, &NetworkClient::disconnected);
    connect(mSocket, &QTcpSocket::errorOccurred, this, &NetworkClient::onSocketError);
}

void NetworkClient::connectToHost(const QString& host, quint16 port, const QString& playerName)
{
    mPlayerName = playerName;
    mSocket->connectToHost(host, port);
}

void NetworkClient::sendMove(const QString& move)
{
    if (mSocket->state() == QAbstractSocket::ConnectedState)
        mSocket->write(Protocol::encode(Protocol::move(move)));
}

void NetworkClient::disconnectFromHost()
{
    mSocket->disconnectFromHost();
}

bool NetworkClient::isConnected() const
{
    return mSocket->state() == QAbstractSocket::ConnectedState;
}

void NetworkClient::onConnected()
{
    mSocket->write(Protocol::encode(Protocol::hello(mPlayerName)));
}

void NetworkClient::onReadyRead()
{
    mReader.append(mSocket->readAll());
    while (auto msg = mReader.next())
        handle(*msg);
}

void NetworkClient::onSocketError(QAbstractSocket::SocketError)
{
    emit errorOccurred(mSocket->errorString());
}

void NetworkClient::handle(const QJsonObject& msg)
{
    const QString type = Protocol::typeOf(msg);

    if (type == QLatin1String(Protocol::Type::Welcome)) {
        emit welcomed(toStringList(msg.value(QStringLiteral("roster")).toArray()));
    } else if (type == QLatin1String(Protocol::Type::Lobby)) {
        emit rosterChanged(toStringList(msg.value(QStringLiteral("roster")).toArray()));
    } else if (type == QLatin1String(Protocol::Type::GameStarted)) {
        emit gameStarted();
    } else if (type == QLatin1String(Protocol::Type::YourTurn)) {
        emit movePromptRequested();
    } else if (type == QLatin1String(Protocol::Type::Log)) {
        emit logMessage(msg.value(QStringLiteral("text")).toString());
    } else if (type == QLatin1String(Protocol::Type::Scores)) {
        emit scoresUpdated(toStringList(msg.value(QStringLiteral("names")).toArray()),
                           toStringList(msg.value(QStringLiteral("scores")).toArray()));
    } else if (type == QLatin1String(Protocol::Type::RoundFinished)) {
        emit roundFinished();
    } else if (type == QLatin1String(Protocol::Type::GameFinished)) {
        emit gameFinished(toStringList(msg.value(QStringLiteral("lines")).toArray()));
    }
}
