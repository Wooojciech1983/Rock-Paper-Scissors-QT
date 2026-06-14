#pragma once
#include <QMainWindow>
#include <QStringList>
#include <memory>

class IGameSession;
class QLabel;
class QTextEdit;
class QTableWidget;
class QPushButton;
class QCloseEvent;

// Main game screen. Pure UI: it owns an IGameSession and translates between the
// session's display signals and the widgets, and between button clicks and the
// session's intent slots. A small state machine keeps the button set consistent
// with where the session currently is. The same window drives local play, the
// network host, and (later) a network client — only the injected session differs.
class GameWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit GameWindow(std::unique_ptr<IGameSession> session, QWidget* parent = nullptr);
    ~GameWindow() override;

protected:
    void closeEvent(QCloseEvent* event) override;

private slots:
    void onStartRound();
    void onPrimaryButton();        // "Finish" (regular) / "Quit" (tournament) / "Close" (game over)
    void onLog(const QString& text);
    void onMovePrompt();
    void onScores(const QStringList& names, const QStringList& scores);
    void onRoundFinished();
    void onGameFinished(const QStringList& finalLines);

private:
    enum class State { Idle, Waiting, AwaitingMove, GameOver };

    void buildUi();
    void setState(State state);
    void pushMove(const QString& move);

    std::unique_ptr<IGameSession> mSession;   // destroyed (→ worker stopped) in closeEvent/dtor
    bool mTournament   = false;               // affects primary-button label ("Quit")
    bool mManualRounds = true;                // Local/Host: a "Start round" button applies
    QString mWaitingText;                     // status shown in the Waiting state (per session kind)

    QLabel*       mStatusLabel  = nullptr;
    QTextEdit*    mHistoryView  = nullptr;
    QTableWidget* mScoreTable   = nullptr;
    QPushButton*  mRockBtn      = nullptr;
    QPushButton*  mPaperBtn     = nullptr;
    QPushButton*  mScissorsBtn  = nullptr;
    QPushButton*  mStartBtn     = nullptr;
    QPushButton*  mPrimaryBtn   = nullptr;

    State mState = State::Idle;
};
