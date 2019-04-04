#ifndef C_WORLD_H
#define C_WORLD_H

#include "entity.h"
#include "c_spell.h"
#include "c_item.h"
#include "c_monster.h"
#include "pq7_config.h"
#include <iostream>
#include <fstream>

#include <QObject>
#include <QList>
#include <QMessageBox>
#include <QJsonObject>

typedef enum {
    pq_debug_none                       = 0,
    pq_debug_zero_action_timer          = 1 << 0,
    pq_debug_always_trigger_action      = 1 << 1,
    pq_debug_action_triggers_save       = 1 << 2,
    pq_debug_reserved_1                 = 1 << 3,
    pq_debug_active                     = 1 << 15
} t_pq_debug;


enum class State {
    Reserved1,
    HeadingToKillingFields,
    Fighting,
    HeadingToTown,
    SellingOff,
    BuyingNewEquipment
};

class c_World : public QObject
{
    Q_OBJECT
public:
    explicit c_World(QObject *parent = nullptr);

    /*
     *      Player instance - gamewide
     */
    Entity* Player;

    /*
     *      Monster instance - gamewide
     */
    c_Monster* Monster;

    /*
     *      State values
     */
    int Act;
    QString Action;
    State currentState;

    /*
     *      Progress Bar values
     */
    int pb_action;
    int pb_experience;
    int pb_encumbrance;
    int pb_plot;
    int pb_quest;

    /*
     *      Quest history
     */
    QStringList quests;

    /*
     *      Timer Interval
     */
    int actionTime;

    /*
     *      Flag for Post Load UI Processing
     */
    bool isLoaded;

    /*
     *    Public Methods
     */
    c_Item* makeEqByGrade(Equipment eqtype , int grade);
    void initPlayer();

    bool isDebugFlagSet(t_pq_debug flag);
    bool isDebugFlagReset(t_pq_debug flag);
    void setDebugFlag(t_pq_debug flag);
    void resetDebugFlag(t_pq_debug flag);
    void toggleDebugFlag(t_pq_debug flag);
    void debugClear();
    void debugActive();

private:

    // save / load helpers
    QStringList arrayToList(QJsonArray array);

    /*
     *      Debug flags
     */
    quint32 debug;

signals:
    //void debugChanged(quint32 dbgValue);
    //void gameSaved();
    //void gameLoaded();

public slots:
    void save();
    void save(QString filename);
    void load();
    void load(QString filename);
    void load(QJsonObject root);
};

#endif // C_WORLD_H

extern c_World* game;
