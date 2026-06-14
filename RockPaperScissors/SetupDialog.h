#pragma once
#include <QDialog>
#include <QString>

class QLineEdit;
class QSpinBox;
class QCheckBox;

// Initial screen: collects the session configuration and the chosen play mode.
class SetupDialog : public QDialog {
    Q_OBJECT

public:
    enum class Mode { Local, Host, Join };

    explicit SetupDialog(QWidget* parent = nullptr);

    QString playerName() const;
    int     opponents() const;
    bool    withZhejiang() const;
    bool    tournament() const;
    Mode    mode() const { return mMode; }

private:
    void accomplish(Mode mode);   // store mode + accept()

    QLineEdit* mNameEdit = nullptr;
    QSpinBox*  mOpponentsSpin = nullptr;
    QCheckBox* mZhejiangCheck = nullptr;
    QCheckBox* mTournamentCheck = nullptr;
    Mode       mMode = Mode::Local;
};
