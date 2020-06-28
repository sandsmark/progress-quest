#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QTimer>
#include <QKeyEvent>
#include <QFocusEvent>

/*
#include "entity.h"
#include "c_spell.h"
#include "c_item.h"
#include "c_monster.h"
#include "pq7_config.h"
*/
#include "c_world.h"

#include "dialog_opening.h"
#include <QMainWindow>

namespace Ui
{
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();


private:
    Ui::MainWindow *ui;

    //int Act;
    //QString Action;
    //c_Monster* curMonster;
    //t_pq_state State;

    void initFrames();
    //void initPlayer();

    void addAct();
    void startAct();
    void addQuest(QString name);
    QString generateQuest();

    void setMonster();
    void addMonsterDrop();
    QString sellInventoryItem();
    void removeInventoryItem();

    void buyEquipment();
    bool canBuyEquipment(Equipment eqtype);
    c_Item *getPurchaseItem(Equipment eqtype);
    c_Item *upgradeEquipment(Equipment eqtype, int grade);

    void winStats();
    void winSpells();

    void updateSpellTable();
    void updateInventoryTable();
    void updateEquipmentTable();
    void updateStatsTable();
    void updateQuestList();
    void postLoadUpdates();

    void transitionState();

    // key responder reimplementation
    void keyPressEvent(QKeyEvent *k);

    // timer drive
    QTimer *pb_action_timer;

public slots:
    void incr_pb_action_value();
    void increaseExperience();
    void updateEncumberance();
    void progressPlot();
    void progressQuest();

    void gameSave();
    //void gameLoad();
    \
    void setAction();
};

#endif // MAINWINDOW_H
