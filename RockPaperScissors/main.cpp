#include "GameWindow.h"
#include "SetupDialog.h"
#include "NetworkLobbyDialog.h"
#include <GameWorker.h>
#include <IGameSession.h>
#include <LocalGameSession.h>
#include <HostGameSession.h>
#include <NetworkClientSession.h>
#include <QApplication>
#include <memory>

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    SetupDialog setup;
    if (setup.exec() != QDialog::Accepted)
        return 0;

    std::unique_ptr<IGameSession> session;

    switch (setup.mode()) {
    case SetupDialog::Mode::Local: {
        GameWorker::Config config;
        config.playerName      = setup.playerName().toStdString();
        config.opponents       = setup.opponents();
        config.withZhejiangBot = setup.withZhejiang();
        config.tournamentMode  = setup.tournament();
        session = std::make_unique<LocalGameSession>(config);
        break;
    }
    case SetupDialog::Mode::Host: {
        NetworkLobbyDialog lobby(NetworkLobbyDialog::Role::Host, setup.playerName());
        if (lobby.exec() != QDialog::Accepted)
            return 0;
        HostGameSession::Config cfg;
        cfg.playerName      = setup.playerName().toStdString();
        cfg.aiBots          = setup.opponents();
        cfg.withZhejiangBot = setup.withZhejiang();
        session = std::make_unique<HostGameSession>(cfg, lobby.takeServer());
        break;
    }
    case SetupDialog::Mode::Join: {
        NetworkLobbyDialog lobby(NetworkLobbyDialog::Role::Join, setup.playerName());
        if (lobby.exec() != QDialog::Accepted)
            return 0;
        session = std::make_unique<NetworkClientSession>(lobby.takeClient());
        break;
    }
    }

    GameWindow window(std::move(session));
    window.show();

    return app.exec();
}
