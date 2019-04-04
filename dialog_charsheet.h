#ifndef DIALOG_CHARSHEET_H
#define DIALOG_CHARSHEET_H

#include <QDialog>
#include <QString>
#include <QComboBox>
#include <QSpinBox>
#include <QPushButton>
#include <QLabel>
#include <QList>
#include <QMap>
#include <QStack>

#include <random>

#include "c_world.h"
#include "pq7_config.h"

class QComboBox;
class QSpinBox;
class QGroupBox;

class Dialog_charSheet : public QDialog
{
    Q_OBJECT
    
public:
    explicit Dialog_charSheet(QWidget *parent = nullptr);
    ~Dialog_charSheet();

public slots:
    void transferCharToGame();

private slots:
    void onNameChanged(const QString &name);
    void generateName();
    void onStatsChanged();
    void onClassOrRaceChanged();
    void doRoll();
    void doUnroll();

private:
    QButtonGroup *createGroup(const QString &name, const QStringList &elements, QLayout *layout);

    QComboBox cmb_race;
    QComboBox cmb_class;

    QPushButton btn_ok;
    QPushButton btn_cancel;

    QButtonGroup *m_raceButtons;
    QButtonGroup *m_classButtons;

    std::mt19937 m_randGen;

    QLineEdit *m_nameEdit;

    QMap<QString, QLineEdit*> m_statsEdits;
    QLineEdit *m_totalStatCount;

    QStack<QMap<QString, int>> m_rollHistory;

    QPushButton *m_unrollButton;
    QPushButton *m_soldButton;
};

#endif // DIALOG_CHARSHEET_H
