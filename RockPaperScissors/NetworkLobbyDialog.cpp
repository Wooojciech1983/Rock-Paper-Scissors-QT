#include "NetworkLobbyDialog.h"
#include <NetworkClient.h>
#include <NetworkServer.h>

#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

namespace {
constexpr quint16 kDefaultPort = 12345;
}

NetworkLobbyDialog::NetworkLobbyDialog(Role role, QString playerName, QWidget* parent)
    : QDialog(parent), mRole(role), mPlayerName(std::move(playerName))
{
    setWindowTitle(mRole == Role::Host ? QStringLiteral("Host a Network Game")
                                       : QStringLiteral("Join a Network Game"));
    if (mRole == Role::Host) buildHostUi();
    else                     buildJoinUi();
    resize(360, mRole == Role::Host ? 320 : 160);
}

NetworkLobbyDialog::~NetworkLobbyDialog()
{
    delete mServer;   // no-op if taken (set to nullptr)
    delete mClient;
}

quint16 NetworkLobbyDialog::port() const
{
    return static_cast<quint16>(mPortSpin->value());
}

void NetworkLobbyDialog::buildHostUi()
{
    mPortSpin = new QSpinBox(this);
    mPortSpin->setRange(1024, 65535);
    mPortSpin->setValue(kDefaultPort);

    mActionBtn = new QPushButton(QStringLiteral("Open lobby"), this);
    connect(mActionBtn, &QPushButton::clicked, this, &NetworkLobbyDialog::startHosting);

    mRosterList = new QListWidget(this);

    mStatusLabel = new QLabel(QStringLiteral("Pick a port and open the lobby."), this);
    mStatusLabel->setWordWrap(true);

    mStartBtn = new QPushButton(QStringLiteral("Start game"), this);
    mStartBtn->setEnabled(false);
    connect(mStartBtn, &QPushButton::clicked, this, &QDialog::accept);

    auto* cancelBtn = new QPushButton(QStringLiteral("Cancel"), this);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);

    auto* portRow = new QHBoxLayout;
    portRow->addWidget(new QLabel(QStringLiteral("Port:"), this));
    portRow->addWidget(mPortSpin, 1);
    portRow->addWidget(mActionBtn);

    auto* buttons = new QHBoxLayout;
    buttons->addWidget(cancelBtn);
    buttons->addStretch();
    buttons->addWidget(mStartBtn);

    auto* root = new QVBoxLayout(this);
    root->addLayout(portRow);
    root->addWidget(new QLabel(QStringLiteral("Players in the room:"), this));
    root->addWidget(mRosterList, 1);
    root->addWidget(mStatusLabel);
    root->addLayout(buttons);
}

void NetworkLobbyDialog::buildJoinUi()
{
    mHostEdit = new QLineEdit(QStringLiteral("127.0.0.1"), this);

    mPortSpin = new QSpinBox(this);
    mPortSpin->setRange(1024, 65535);
    mPortSpin->setValue(kDefaultPort);

    mStatusLabel = new QLabel(QStringLiteral("Enter the host address and connect."), this);
    mStatusLabel->setWordWrap(true);

    mActionBtn = new QPushButton(QStringLiteral("Connect"), this);
    connect(mActionBtn, &QPushButton::clicked, this, &NetworkLobbyDialog::beginConnect);

    auto* cancelBtn = new QPushButton(QStringLiteral("Cancel"), this);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);

    auto* form = new QFormLayout;
    form->addRow(QStringLiteral("Host:"), mHostEdit);
    form->addRow(QStringLiteral("Port:"), mPortSpin);

    auto* buttons = new QHBoxLayout;
    buttons->addWidget(cancelBtn);
    buttons->addStretch();
    buttons->addWidget(mActionBtn);

    auto* root = new QVBoxLayout(this);
    root->addLayout(form);
    root->addWidget(mStatusLabel);
    root->addLayout(buttons);
}

void NetworkLobbyDialog::startHosting()
{
    if (!mServer) {
        mServer = new NetworkServer;   // not parented: ownership tracked here / transferred on accept
        connect(mServer, &NetworkServer::rosterChanged, this, &NetworkLobbyDialog::setRoster);
    }

    if (!mServer->listen(port())) {
        mStatusLabel->setText(QStringLiteral("Could not listen on port %1 — is it in use?")
                                  .arg(port()));
        return;
    }

    mPortSpin->setEnabled(false);
    mActionBtn->setEnabled(false);
    mStartBtn->setEnabled(true);
    mStatusLabel->setText(QStringLiteral("Listening on port %1. Waiting for players… "
                                         "(you can start at any time).").arg(mServer->port()));
}

void NetworkLobbyDialog::beginConnect()
{
    if (!mClient) {
        mClient = new NetworkClient;   // ownership tracked here / transferred on accept
        connect(mClient, &NetworkClient::welcomed, this, [this](const QStringList&) {
            mStatusLabel->setText(QStringLiteral("Connected. Waiting for the host to start…"));
        });
        connect(mClient, &NetworkClient::gameStarted, this, &QDialog::accept);
        connect(mClient, &NetworkClient::errorOccurred, this, [this](const QString& msg) {
            mStatusLabel->setText(QStringLiteral("Connection failed: %1").arg(msg));
            mActionBtn->setEnabled(true);
            mHostEdit->setEnabled(true);
            mPortSpin->setEnabled(true);
        });
    }

    mActionBtn->setEnabled(false);
    mHostEdit->setEnabled(false);
    mPortSpin->setEnabled(false);
    mStatusLabel->setText(QStringLiteral("Connecting to %1:%2…").arg(mHostEdit->text()).arg(port()));
    mClient->connectToHost(mHostEdit->text().trimmed(), port(), mPlayerName);
}

void NetworkLobbyDialog::setRoster(const QStringList& names)
{
    mRosterList->clear();
    mRosterList->addItems(names);
    mStatusLabel->setText(QStringLiteral("Listening on port %1. %2 player(s) joined.")
                              .arg(mServer->port()).arg(names.size()));
}

NetworkServer* NetworkLobbyDialog::takeServer()
{
    NetworkServer* s = mServer;
    mServer = nullptr;   // caller now owns it; dtor must not delete
    return s;
}

NetworkClient* NetworkLobbyDialog::takeClient()
{
    NetworkClient* c = mClient;
    mClient = nullptr;
    return c;
}
