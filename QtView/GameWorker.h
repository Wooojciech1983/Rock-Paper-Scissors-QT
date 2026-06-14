#pragma once
#include <IBotStrategy.h>
#include <QObject>
#include <memory>
#include <string>
#include <vector>

class IGameView;
class MoveStream;
class GameController;

// Runs the (blocking) backend game loop on a worker thread so the Qt event loop
// stays responsive. Input arrives through the MoveStream (fed by the GUI); output
// goes through the injected IGameView (QtView, possibly tapped for broadcast).
//
// run() is intended to be triggered by QThread::started after the worker has been
// moved onto that thread. It emits finished() when the game/tournament ends.
class GameWorker : public QObject {
    Q_OBJECT

public:
    struct Config {
        std::string playerName = "Player";
        int  opponents = 1;
        bool withZhejiangBot = false;
        bool tournamentMode = false;
    };

    // Local play (regular or tournament): the worker builds the bots itself.
    GameWorker(Config config, IGameView& view, MoveStream& stream, QObject* parent = nullptr);

    // Host play: the caller supplies ready-made participants (remote players + AI
    // bots) and their names. Always runs the regular round loop (no tournament).
    GameWorker(std::string playerName,
               std::vector<std::unique_ptr<IBotStrategy>> participants,
               std::vector<std::string> participantNames,
               IGameView& view, MoveStream& stream, QObject* parent = nullptr);

    ~GameWorker() override;   // defined in the .cpp where GameController is complete

public slots:
    void run();

signals:
    void finished();

private:
    // Shared regular-mode loop: empty line starts a round, "exit"/EOF ends it.
    void runRegularLoop(GameController& controller);

    enum class Mode { Regular, Tournament, Host };

    Mode        mMode;
    Config      mConfig;
    std::string mHostPlayerName;
    std::vector<std::unique_ptr<IBotStrategy>> mHostParticipants;
    std::vector<std::string>                   mHostNames;
    IGameView&  mView;
    MoveStream& mStream;

    // Owns the controller for the whole worker lifetime (NOT just run()'s scope) so
    // injected RemotePlayerStrategy participants outlive the worker thread. They are
    // destroyed only when the worker is deleted — after the server has stopped
    // routing — preventing the server's raw strategy pointers from dangling.
    std::unique_ptr<GameController> mController;
};
