#ifndef ENTITY_H
#define ENTITY_H

#include <cmath>
#include "c_spell.h"
#include "c_item.h"
#include "pq7_config.h"
#include <iostream>
#include <fstream>

#include <QObject>
#include <QString>

class Entity : public QObject
{
    Q_OBJECT

public:
    explicit Entity(QObject *parent = 0);

    // traits (Voc=Vocation)
    QString Name, Race, Voc, Level;

    // stats
    QString STR, INT, WIS, DEX, CON, CHA, HPMax, MPMax;

    // experience
    QString XP;
    qulonglong maxXP();



    // spells
    QList<c_Spell*> Spells;

    // equipment
    c_Item*         Weapon;
    c_Item*         Sheild;
    QList<c_Item*>  Armor;

    // used for buying new equipment
    c_Item*         Purchase;
    QString         purchType();
    void            setPurchNew(bool is_new);

    // inventory
    QList<c_Item*> Inventory;
    QList<int> Quantity;
    int Encumbrance();
    int maxEncumbrance();

    // Gold (ah, weightless gold)
    int Gold;

    // random methods - used in init
    QString nameRand();
    QString raceRand();
    QString vocRand();
    void incrLevel();

    QJsonObject save();
    void load(QJsonObject entityRoot);

signals:
    void levelUp();

public slots:

    
private:

    bool isNewPurchase;

    // methods
    QString statRand(int basevalue, int offset);

    // save / load helper
    QJsonArray spellListToArray(QList<c_Spell *> &list);
    QJsonArray itemListToArray(QList<c_Item *> &list);
    QJsonArray invListToArray(QList<c_Item *> &iList, QList<int> &nList);

    QList<c_Spell *> arrayToSpellList(QJsonArray array);
    QList<c_Item *> arrayToItemList(QJsonArray array);
    void arrayToInvList(QJsonArray array, QList<c_Item *> &iList, QList<int> &nList);
};

#endif // ENTITY_H
