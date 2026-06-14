#include "GameWindow.h"
#include <IGameSession.h>

#include <QCloseEvent>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QWidget>

GameWindow::GameWindow(std::unique_ptr<IGameSession> session, QWidget* parent)
    : QMainWindow(parent), mSession(std::move(session))
{
    using Kind = IGameSession::Kind;
    const Kind kind = mSession->kind();
    mTournament   = (kind == Kind::Tournament);
    mManualRounds = (kind == Kind::Local || kind == Kind::Host);

    QString title = QStringLiteral("Rock-Paper-Scissors");
    switch (kind) {
    case Kind::Local:      mWaitingText = QStringLiteral("Opponents are thinking…");        break;
    case Kind::Tournament: mWaitingText = QStringLiteral("Tournament in progress…");        break;
    case Kind::Host:       mWaitingText = QStringLiteral("Waiting for the other players…");
                           title += QStringLiteral(" — Host");                              break;
    case Kind::Client:     mWaitingText = QStringLiteral("Waiting for the host…");
                           title += QStringLiteral(" — Client");                            break;
    }
    setWindowTitle(title);
    buildUi();

    connect(mSession.get(), &IGameSession::logMessage,          this, &GameWindow::onLog);
    connect(mSession.get(), &IGameSession::movePromptRequested, this, &GameWindow::onMovePrompt);
    connect(mSession.get(), &IGameSession::scoresUpdated,       this, &GameWindow::onScores);
    connect(mSession.get(), &IGameSession::roundFinished,       this, &GameWindow::onRoundFinished);
    connect(mSession.get(), &IGameSession::gameFinished,        this, &GameWindow::onGameFinished);

    mSession->start();
    // Manual-round play (local regular / host) begins idle awaiting a "Start round"
    // click; flow-driven play (tournament / network client) begins waiting.
    setState(mManualRounds ? State::Idle : State::Waiting);
}

GameWindow::~GameWindow() = default;   // mSession destroyed here → worker stopped/joined

void GameWindow::buildUi()
{
    auto* central = new QWidget(this);
    setCentralWidget(central);

    mStatusLabel = new QLabel(this);
    mStatusLabel->setStyleSheet(QStringLiteral("font-weight: bold;"));

    mHistoryView = new QTextEdit(this);
    mHistoryView->setReadOnly(true);

    mScoreTable = new QTableWidget(0, 2, this);
    mScoreTable->setHorizontalHeaderLabels({ QStringLiteral("Player"), QStringLiteral("Score") });
    mScoreTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    mScoreTable->verticalHeader()->setVisible(false);
    mScoreTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    mScoreTable->setSelectionMode(QAbstractItemView::NoSelection);

    mRockBtn     = new QPushButton(QStringLiteral("Rock"), this);
    mPaperBtn    = new QPushButton(QStringLiteral("Paper"), this);
    mScissorsBtn = new QPushButton(QStringLiteral("Scissors"), this);
    connect(mRockBtn,     &QPushButton::clicked, this, [this] { pushMove(QStringLiteral("r")); });
    connect(mPaperBtn,    &QPushButton::clicked, this, [this] { pushMove(QStringLiteral("p")); });
    connect(mScissorsBtn, &QPushButton::clicked, this, [this] { pushMove(QStringLiteral("s")); });

    mStartBtn = new QPushButton(QStringLiteral("Start round"), this);
    connect(mStartBtn, &QPushButton::clicked, this, &GameWindow::onStartRound);
    mStartBtn->setVisible(mManualRounds);

    mPrimaryBtn = new QPushButton(this);
    connect(mPrimaryBtn, &QPushButton::clicked, this, &GameWindow::onPrimaryButton);

    auto* moves = new QHBoxLayout;
    moves->addWidget(mRockBtn);
    moves->addWidget(mPaperBtn);
    moves->addWidget(mScissorsBtn);

    auto* controls = new QHBoxLayout;
    controls->addWidget(mStartBtn);
    controls->addStretch();
    controls->addWidget(mPrimaryBtn);

    auto* root = new QVBoxLayout(central);
    root->addWidget(mStatusLabel);
    root->addWidget(mHistoryView, 1);
    root->addWidget(mScoreTable);
    root->addLayout(moves);
    root->addLayout(controls);

    resize(460, 560);
}

void GameWindow::setState(State state)
{
    mState = state;

    const bool awaiting = (state == State::AwaitingMove);
    mRockBtn->setEnabled(awaiting);
    mPaperBtn->setEnabled(awaiting);
    mScissorsBtn->setEnabled(awaiting);

    // "Start round" only applies to manual-round (regular local) play while idle.
    mStartBtn->setEnabled(mManualRounds && state == State::Idle);

    switch (state) {
    case State::Idle:
        mStatusLabel->setText(QStringLiteral("Ready — click \"Start round\"."));
        mPrimaryBtn->setText(QStringLiteral("Finish game"));
        mPrimaryBtn->setEnabled(true);
        break;
    case State::Waiting:
        mStatusLabel->setText(mWaitingText);
        mPrimaryBtn->setText(mTournament ? QStringLiteral("Quit") : QStringLiteral("Finish game"));
        mPrimaryBtn->setEnabled(true);
        break;
    case State::AwaitingMove:
        mStatusLabel->setText(QStringLiteral("Your turn — pick a move!"));
        mPrimaryBtn->setText(mTournament ? QStringLiteral("Quit") : QStringLiteral("Finish game"));
        mPrimaryBtn->setEnabled(true);
        break;
    case State::GameOver:
        mStatusLabel->setText(QStringLiteral("Game over."));
        mPrimaryBtn->setText(QStringLiteral("Close"));
        mPrimaryBtn->setEnabled(true);
        break;
    }
}

void GameWindow::pushMove(const QString& move)
{
    if (mState != State::AwaitingMove) return;
    mSession->submitMove(move);
    setState(State::Waiting);
}

void GameWindow::onStartRound()
{
    if (mState != State::Idle) return;
    mSession->requestStartRound();
    setState(State::Waiting);
}

void GameWindow::onPrimaryButton()
{
    if (mState == State::GameOver) {
        close();
    } else if (mTournament) {
        close();                       // closeEvent performs the graceful shutdown
    } else {
        mSession->requestFinish();     // regular loop ends → ShowResults → gameFinished
        setState(State::Waiting);
    }
}

void GameWindow::onLog(const QString& text)
{
    mHistoryView->append(text);
}

void GameWindow::onMovePrompt()
{
    setState(State::AwaitingMove);
}

void GameWindow::onScores(const QStringList& names, const QStringList& scores)
{
    mScoreTable->setRowCount(names.size());
    for (int i = 0; i < names.size(); ++i) {
        mScoreTable->setItem(i, 0, new QTableWidgetItem(names[i]));
        mScoreTable->setItem(i, 1, new QTableWidgetItem(i < scores.size() ? scores[i] : QString()));
    }
}

void GameWindow::onRoundFinished()
{
    if (mState != State::GameOver)
        setState(mManualRounds ? State::Idle : State::Waiting);
}

void GameWindow::onGameFinished(const QStringList& finalLines)
{
    mHistoryView->append(QStringLiteral("\n=== Final scores ==="));
    for (const QString& line : finalLines)
        mHistoryView->append(line);
    setState(State::GameOver);
}

void GameWindow::closeEvent(QCloseEvent* event)
{
    mSession.reset();   // stops/joins the worker before the widgets tear down
    QMainWindow::closeEvent(event);
}
