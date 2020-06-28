#include "entity.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

Entity::Entity(QObject *parent) :
    QObject(parent)
{
    // traits
    Level = "1";

    // stats
    STR = statRand(12,6);
    INT = statRand(12,6);
    WIS = statRand(12,6);
    DEX = statRand(12,6);
    CON = statRand(12,6);
    CHA = statRand(12,6);

    HPMax = statRand(22,10);
    MPMax = statRand(12,8);

    XP = "0";

    // spells / equip aren't freely given
    Inventory.clear();
    Quantity.clear();
    Spells.clear();
    Armor.clear();

    isNewPurchase = true;
}

int Entity::Encumbrance()
{
    int totWeight(0);
    for (int i = 0; i < Inventory.size(); i++) {
        totWeight += Inventory.at(i)->Weight * Quantity.at(i);
    }
    return totWeight;
}

QString Entity::purchaseType()
{
    if (isNewPurchase) {
        return "new";
    } else {
        return "upgrade";
    }
}
void Entity::setPurchNew(bool is_new)
{
    isNewPurchase = is_new;
}

qulonglong Entity::maxXP()
{
    // xp for next level is calculated by exp curve
    // see pq7_level_advancement_curvatures.ggb (geogebra4 file)
    // for more info

    float Lv = Level.toFloat();
    float curvature = 1.09512; // coeffecient
    float bend = 20.0; // start bending at level
    float base = 1130.0; // xp at bend level

    return (int)(base * pow(curvature, (Lv - bend)));
    //2^64 = 1130 * 1.09512^(x - 20)
    //x = 430.85
}

int Entity::maxEncumbrance()
{
    return STR.toInt() + 15;
}

QString Entity::statRand(int basevalue, int offset)
{
    return QString::number(rand() % basevalue + offset);
}

QString Entity::generateName()
{
    QString build;
    QStringList nameparts;
    nameparts << "zul" << "arg" << "eb" << "cork" << "dar"
              << "dor" << "sar" << "nar" << "en" << "mor"
              << "goth" << "buf" << "er" << "ag" << "ort"
              << "tor" << "tar" << "grim" << "val" << "kul"
              << "kun" << "xi" << "ja" << "ju" << "or"
              << "mard" << "ed" << "chu" << "thi" << "oub"
              << "sch" << "chi" << "ki" << "lo" << "lun";

    for (int i =0; i < 3; i++) {
        build += nameparts.at(rand() % nameparts.size());
    }

    build[0] = build[0].toUpper();

    return build;
}

QString Entity::randomRace()
{
    return gConfig->Races.at(rand() % gConfig->Races.size()).split("|").first();
}

QString Entity::randomVocation()
{
    return gConfig->Klasses.at(rand() % gConfig->Klasses.size()).split("|").first();
}

void Entity::increaseLevel()
{
    XP = "0";
    Level = QString().number(Level.toInt() + 1);
}

QJsonObject Entity::save()
{
    QJsonObject entity;

    entity["Name"] = Name;
    entity["Race"] = Race;
    entity["Class"] = Voc;
    entity["Level"] = Level;

    entity["STR"] = STR;
    entity["INT"] = INT;
    entity["WIS"] = WIS;
    entity["DEX"] = DEX;
    entity["CON"] = CON;
    entity["CHA"] = CHA;
    entity["HPMax"] = HPMax;
    entity["MPMax"] = MPMax;

    entity["XP"] = XP;

    entity["Spells"] = Entity::spellListToArray(Spells);

    entity["Weapon"] = Weapon->save();
    entity["Shield"] = Shield->save();
    entity["Armor"] = Entity::itemListToArray(Armor);

    entity["Inventory"] = Entity::invListToArray(Inventory, Quantity);

    entity["Gold"] = Gold;

    QJsonObject root;
    root["Entity"] = entity;
    return root;
}

void Entity::load(QJsonObject entityRoot)
{
    // unpack entity values
    entityRoot = entityRoot["Entity"].toObject();

    Name = entityRoot["Name"].toString();
    Race = entityRoot["Race"].toString();
    Voc = entityRoot["Class"].toString();
    Level = entityRoot["Level"].toString();
    STR = entityRoot["STR"].toString();
    INT = entityRoot["INT"].toString();
    WIS = entityRoot["WIS"].toString();
    DEX = entityRoot["DEX"].toString();
    CON = entityRoot["CON"].toString();
    CHA = entityRoot["CHA"].toString();
    HPMax = entityRoot["HPMax"].toString();
    MPMax = entityRoot["MPMax"].toString();

    XP = entityRoot["XP"].toString();

    Spells = Entity::arrayToSpellList(entityRoot["Spells"].toArray());

    Weapon->load(entityRoot["Weapon"].toObject());
    Shield->load(entityRoot["Shield"].toObject());
    Armor = Entity::arrayToItemList(entityRoot["Armor"].toArray());

    Entity::arrayToInvList(entityRoot["Inventory"].toArray(), Inventory, Quantity);

    Gold = entityRoot["Gold"].toInt();
}

QJsonArray Entity::spellListToArray(QList<c_Spell *> &list)
{
    QJsonArray array;
    for (int i=0; i < list.size(); i++) {
        array.append(list.at(i)->save());
    }
    return array;
}

QJsonArray Entity::itemListToArray(QList<c_Item *> &list)
{
    QJsonArray array;
    for (int i=0; i < list.size(); i++) {
        array.append(list.at(i)->save());
    }
    return array;
}

QJsonArray Entity::invListToArray(QList<c_Item *> &iList, QList<int> &nList)
{
    QJsonArray array;
    for (int i=0; i < iList.size(); i++) {
        QJsonArray pair;
        pair.append(iList.at(i)->save());
        pair.append(nList.at(i));

        array.append(pair);
    }
    return array;
}

QList<c_Spell *> Entity::arrayToSpellList(QJsonArray array)
{
    QList<c_Spell *> list;
    for (int i=0; i < array.size(); i++) {
        list.append(new c_Spell);
        list.at(list.size() - 1)->load(array[i].toObject());
    }
    return list;
}

QList<c_Item *> Entity::arrayToItemList(QJsonArray array)
{
    QList<c_Item *> list;
    for (int i=0; i < array.size(); i++) {
        list.append(new c_Item);
        list.at(list.size() - 1)->load(array[i].toObject());
    }
    return list;
}

void Entity::arrayToInvList(QJsonArray array, QList<c_Item *> &iList, QList<int> &nList)
{
    iList.clear();
    nList.clear();

    for (int i=0; i < array.size(); i++) {
        QJsonArray pair = array[i].toArray();

        iList.append(new c_Item);
        iList.at(iList.size() - 1)->load(pair[i].toObject());

        nList.append(pair[1].toInt());
    }
}
