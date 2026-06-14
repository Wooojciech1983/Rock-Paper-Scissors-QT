#pragma once
#include <QDialog>
#include <QString>

class QLineEdit;
class QSpinBox;
class QListWidget;
class QLabel;
class QPushButton;
class NetworkServer;
class NetworkClient;

// Pre-game network lobby.
//   • Host role: opens a listening server and shows players as they join, with a
//     "Start game" button.
//   • Join role: connects to a host and waits for the game to begin.
// On Accepted, ownership of the created server/client transfers to the caller via
// takeServer()/takeClient(); anything not taken is destroyed with the dialog.
class NetworkLobbyDialog : public QDialog {
    Q_OBJECT

public:
    enum class Role { Host, Join };
    NetworkLobbyDialog(Role role, QString playerName, QWidget* parent = nullptr);
    ~NetworkLobbyDialog() override;

    [[nodiscard]] quint16 port() const;
    NetworkServer* takeServer();   // host only (nullptr otherwise)
    NetworkClient* takeClient();   // join only (nullptr otherwise)

private:
    void buildHostUi();
    void buildJoinUi();
    void startHosting();
    void beginConnect();
    void setRoster(const QStringList& names);

    Role    mRole;
    QString mPlayerName;

    QSpinBox*    mPortSpin    = nullptr;
    QLineEdit*   mHostEdit    = nullptr;   // join: host address
    QListWidget* mRosterList  = nullptr;   // host: connected players
    QLabel*      mStatusLabel = nullptr;
    QPushButton* mActionBtn   = nullptr;   // host: "Open lobby"; join: "Connect"
    QPushButton* mStartBtn    = nullptr;   // host only: "Start game"

    NetworkServer* mServer = nullptr;
    NetworkClient* mClient = nullptr;
};
