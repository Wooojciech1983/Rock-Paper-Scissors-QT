#pragma once
#include <QObject>
#include <QString>
#include <QStringList>

// Abstraction the GameWindow binds to, decoupling the UI from how a game is
// actually run. Implementations:
//   • LocalGameSession     — runs the backend GameController/TournamentManager on
//                            a worker thread (local play and, later, the host).
//   • NetworkClientSession — replays view events received from a remote host and
//                            forwards the local player's moves back (added later).
//
// Signals flow session → window (what to display); slots flow window → session
// (player intent). The signal signatures intentionally mirror QtView so a local
// session can forward QtView's emissions straight through.
class IGameSession : public QObject {
    Q_OBJECT

public:
    explicit IGameSession(QObject* parent = nullptr) : QObject(parent) {}
    ~IGameSession() override = default;

    // What kind of session this is. The UI derives everything cosmetic/behavioural
    // from it: window title, the "waiting" status text, button labels, and whether
    // rounds are started manually (Local/Host) or driven by the flow (Tournament/
    // Client, where there is no "Start round" button).
    enum class Kind { Local, Tournament, Host, Client };
    [[nodiscard]] virtual Kind kind() const = 0;

public slots:
    // Begin the session (spawn the worker / open the connection). Call once after
    // the owner has connected to the signals below.
    virtual void start() = 0;

    // The player asked to begin the next round (only meaningful for Local/Host).
    virtual void requestStartRound() = 0;

    // The player chose a move: "r", "p" or "s".
    virtual void submitMove(const QString& move) = 0;

    // The player asked to end the session (graceful finish where supported).
    virtual void requestFinish() = 0;

signals:
    void logMessage(QString text);
    void movePromptRequested();
    void scoresUpdated(QStringList names, QStringList scores);
    void roundFinished();
    void gameFinished(QStringList finalScoreLines);
};
