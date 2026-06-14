#include "NetworkClientSession.h"
#include "NetworkClient.h"

NetworkClientSession::NetworkClientSession(NetworkClient* client, QObject* parent)
    : IGameSession(parent), mClient(client)
{
    if (mClient) mClient->setParent(this);   // session now owns the client
}

void NetworkClientSession::start()
{
    connect(mClient, &NetworkClient::logMessage,          this, &IGameSession::logMessage);
    connect(mClient, &NetworkClient::movePromptRequested, this, &IGameSession::movePromptRequested);
    connect(mClient, &NetworkClient::scoresUpdated,       this, &IGameSession::scoresUpdated);
    connect(mClient, &NetworkClient::roundFinished,       this, &IGameSession::roundFinished);

    connect(mClient, &NetworkClient::gameFinished, this, [this](const QStringList& lines) {
        mEnded = true;
        emit gameFinished(lines);
    });
    connect(mClient, &NetworkClient::disconnected, this, [this] {
        if (mEnded) return;                  // already over (normal game end)
        mEnded = true;
        emit logMessage(QStringLiteral("Connection to the host was lost."));
        emit gameFinished({});               // push the window into its game-over state
    });
}

void NetworkClientSession::submitMove(const QString& move)
{
    mClient->sendMove(move);
}

void NetworkClientSession::requestFinish()
{
    if (!mEnded) {
        mEnded = true;
        emit gameFinished({});
    }
    mClient->disconnectFromHost();
}
