#include "SetupDialog.h"
#include <GameController.h>
#include <QCheckBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

SetupDialog::SetupDialog(QWidget* parent) : QDialog(parent)
{
    setWindowTitle(QStringLiteral("Rock-Paper-Scissors — Setup"));

    mNameEdit = new QLineEdit(QStringLiteral("Player"), this);

    mOpponentsSpin = new QSpinBox(this);
    mOpponentsSpin->setRange(1, GameController::maxOpponents);
    mOpponentsSpin->setValue(1);

    mZhejiangCheck = new QCheckBox(QStringLiteral("Include the Zhejiang Bot"), this);

    mTournamentCheck = new QCheckBox(QStringLiteral("Tournament mode (needs 3+ opponents)"), this);
    mTournamentCheck->setEnabled(false);

    // Tournament mode only makes sense with a bracket of 3 or more opponents.
    connect(mOpponentsSpin, &QSpinBox::valueChanged, this, [this](int n) {
        const bool ok = n >= 3;
        mTournamentCheck->setEnabled(ok);
        if (!ok) mTournamentCheck->setChecked(false);
    });

    auto* form = new QFormLayout;
    form->addRow(QStringLiteral("Player name:"), mNameEdit);
    form->addRow(QStringLiteral("AI opponents:"), mOpponentsSpin);
    form->addRow(QString(), mZhejiangCheck);
    form->addRow(QString(), mTournamentCheck);

    auto* playBtn = new QPushButton(QStringLiteral("Play vs AI"), this);
    auto* hostBtn = new QPushButton(QStringLiteral("Host Network Game"), this);
    auto* joinBtn = new QPushButton(QStringLiteral("Join Network Game"), this);
    playBtn->setDefault(true);

    connect(playBtn, &QPushButton::clicked, this, [this] { accomplish(Mode::Local); });
    connect(hostBtn, &QPushButton::clicked, this, [this] { accomplish(Mode::Host); });
    connect(joinBtn, &QPushButton::clicked, this, [this] { accomplish(Mode::Join); });

    auto* buttons = new QHBoxLayout;
    buttons->addWidget(playBtn);
    buttons->addWidget(hostBtn);
    buttons->addWidget(joinBtn);

    auto* root = new QVBoxLayout(this);
    root->addLayout(form);
    root->addLayout(buttons);
}

void SetupDialog::accomplish(Mode mode)
{
    mMode = mode;
    accept();
}

QString SetupDialog::playerName() const
{
    const QString name = mNameEdit->text().trimmed();
    return name.isEmpty() ? QStringLiteral("Player") : name;
}

int  SetupDialog::opponents() const   { return mOpponentsSpin->value(); }
bool SetupDialog::withZhejiang() const { return mZhejiangCheck->isChecked(); }
bool SetupDialog::tournament() const  { return mTournamentCheck->isChecked(); }
