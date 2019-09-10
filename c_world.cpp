#include "c_world.h"
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

c_World::c_World(QObject *parent) :
    QObject(parent)
{
    debug = pq_debug_none;
    Player = new Entity;
    Monster = new c_Monster;
    Act = 0;
    Action  = "";
    currentState = State::Reserved1;
    pb_action = 0;
    pb_encumbrance = 0;
    pb_experience = 0;
    pb_plot = 0;
    pb_quest = 0;
    isLoaded = false;
}

void c_World::initPlayer()
/*
 *      Player creation
 *
 *  calls:      makeEqByGrade
 *  rtns:       void
 *  affects:    c_World::Player
 *
 *  Relies on entity constructor to baseline some values,
 *  fills in and overwrites others that are player
 *  specific.
 */
{
    c_Spell *spell = new c_Spell;
    //c_Item *weapon = new c_Item;
    //c_Item *sheild = new c_Item;
    //QList<c_Item*> armor;
    int index(0), i(0);

    // rand traits
    Player->Name = Player->nameRand();
    Player->Race = Player->raceRand();
    Player->Voc  = Player->vocRand();

    // add starting spell
    spell->setRandName();
    Player->Spells.append(spell);

    // add starting weapon (a weak one =D )
    Player->Weapon = c_World::makeEqByGrade(Equipment::Weapon, -4);

    // Look ma, no sheild!
    Player->Shield = new c_Item;

    // add two starting armors (weak, random)
    for(int i(0); i < 9; i++) {
        Player->Armor.append(new c_Item);
    }
    // 2 *unique* armors - not the same one twice
    do {
        index = rand() % Player->Armor.size();
        if (Player->Armor.at(index)->Name() == tr("")) {
            delete Player->Armor[index];
            Player->Armor[index] = c_World::makeEqByGrade(Equipment::Armoy, -2);
            i++;
        }
    } while (i < 2);

    // finally, money to drink at the bar with
    Player->Gold = 25;
}

c_Item* c_World::makeEqByGrade(Equipment eqtype, int grade)
/*
 *      Make Equipment By Grade
 *
 *  calls:      c_Item methods
 *  rtns:       c_Item* (of specified level)
 *  affects:    none
 *
 *  Used by various to produce items of specific type at
 *  requested level (grade)
 */
{
    c_Item* equip = new c_Item;
    Equipment eqSelect = eqtype;

    // handle "any" type
    if (eqSelect == Equipment::Any)
        eqSelect = Equipment(rand() % 3);

    // main types
    switch(eqSelect) {

    case Equipment::Weapon:

        equip->makeClosestGrade(eqSelect, grade);

        // 2 possible mods, 50% chnc each: player level affects pos/neg
        for(int i(0); i < 2; i++) {
            if (rand() % 2 == 0) {
                if (rand() % Player->Level.toInt() < 5) equip->addWeaponNegMod();
                else equip->addWeaponMod();
            }
        }

        // set bonus to make up differance in grade
        equip->setBonus(grade - equip->Grade());

        break;

    case Equipment::Shield:

        equip->makeClosestGrade(eqSelect, grade);

        // 2 possible mods, 50% chnc each: player level affects pos/neg
        for(int i(0); i < 2; i++) {
            if (rand() % 2 == 0) {
                if (rand() % Player->Level.toInt() < 5) equip->addDefNegMod();
                else equip->addDefMod();
            }
        }

        // set bonus to make up differance in grade
        equip->setBonus(grade - equip->Grade());

        break;

    case Equipment::Armoy:

        equip->makeClosestGrade(eqSelect, grade);

        // 2 possible mods, 50% chnc each: player level affects pos/neg
        for(int i(0); i < 2; i++) {
            if (rand() % 2 == 0) {
                if (rand() % Player->Level.toInt() < 5) equip->addDefNegMod();
                else equip->addDefMod();
            }
        }

        // set bonus to make up differance in grade
        equip->setBonus(grade - equip->Grade());

        break;

    case Equipment::Any:
        break;
    }

    return equip;
}

void c_World::save()
{
    c_World::save("pq_savefile.pqd");
}

void c_World::save(QString filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Failed to open save file" << filename;
        return;
    }

    QJsonObject world;

    /*
        int Act;
        QString Action;
        t_pq_state State;
        */
    world["Act"] = Act;
    world["Action"] = Action;
    world["ActionTime"] = actionTime;
    world["State"] = int(currentState);

    /*
        int pb_action;
        int pb_experience;
        int pb_encumbrance;
        int pb_plot;
        int pb_quest;
        */
    world["pb_action"] = pb_action;
    world["pb_experience"] = pb_experience;
    world["pb_encumbrance"] = pb_encumbrance;
    world["pb_plot"] = pb_plot;
    world["pb_quest"] = pb_quest;

    /*
        quint32 debug
         */
    world["Debug"] = QString::number(debug);

    /*
        QStringList quests;
        */
    world["Quests"] = QJsonArray::fromStringList(quests);

    //Entity* Player;
    world["Player"] = Player->save();

    //c_Monster* Monster;
    world["Monster"] = Monster->save();

    QJsonObject root;
    root["World"] = world;

    file.write(QJsonDocument(root).toJson());
}

void c_World::load()
{
    c_World::load(QString::fromStdString("pq_savefile.pqd"));
}

void c_World::load(QString filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        // file fail
        //std::cerr << "file " << filename << " failed to open" << std::endl;
        //exit(1);
        QMessageBox mBox;
        mBox.setWindowTitle(QString::fromStdString("Progress Quest load ") + filename);
        mBox.setText(QString::fromStdString("Failed to open"));
        mBox.exec();
        exit(1);
    }
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &error);
    if (error.error != QJsonParseError::NoError) {
        // failed to read
        //std::cerr << "JSON Parse fail on file " << filename << ":" << std::endl;
        //std::cerr << readSave.getFormatedErrorMessages();
        //exit (1);
        QMessageBox mBox;
        mBox.setWindowTitle("JSON Parse fail on file " + filename );
        mBox.setText(error.errorString());
        mBox.exec();
        exit(1);
    }

    //chain the json loader
    c_World::load(doc.object());
}

void c_World::load(QJsonObject root)
{
    // unpack world values from anonymous main json object
    root = root["World"].toObject();

    //Entity* Player;
    Player->load(root["Player"].toObject());

    //c_Monster* Monster;
    Monster->load(root["Monster"].toObject());

    /*
        int Act;
        QString Action;
        t_pq_state State;
    */
    Act     = root["Act"].toInt(99);
    Action  = root["Action"].toString("Knock knock...");
    actionTime = root["ActionTime"].toInt(50);
    currentState   = State(root["State"].toInt());

    /*
    quint32 debug
     */
    debug = root["Debug"].toString().toUInt();

    /*
        QStringList quests;
    */
    // if not found, passes empty json array to helper which will return empty qstringlist
    quests = c_World::arrayToList(root["Quests"].toArray());

    /*
        int pb_action;
        int pb_experience;
        int pb_encumbrance;
        int pb_plot;
        int pb_quest;
    */
    pb_action       = root["pb_action"].toInt(99);
    pb_experience   = root["pb_experience"].toInt(99);
    pb_encumbrance  = root["pb_encumbrance"].toInt(99);
    pb_plot         = root["pb_plot"].toInt(99);
    pb_quest        = root["pb_quest"].toInt(99);

    // set loaded flag
    isLoaded = true;
}

// load helper for quest list
QStringList c_World::arrayToList(QJsonArray array)
{
    QStringList list;
    list.clear();

    // load json array to qstring list
    for (int i=0; i < array.size(); i++)
    {
        list.append( array[i].toString() );
    }
    return list;
}


bool c_World::isDebugFlagSet(t_pq_debug flag)
{
    return (debug & flag);
}

bool c_World::isDebugFlagReset(t_pq_debug flag)
{
    return ! (debug & flag);
}

void c_World::setDebugFlag(t_pq_debug flag)
{
    debug |= flag;
}

void c_World::resetDebugFlag(t_pq_debug flag)
{
    debug &= ~flag;
}

void c_World::toggleDebugFlag(t_pq_debug flag)
{
    if (c_World::isDebugFlagReset(flag))
        c_World::setDebugFlag(flag);
    else
        c_World::resetDebugFlag(flag);
}

void c_World::debugClear()
{
    debug = pq_debug_none;
}

void c_World::debugActive()
{
    debug = pq_debug_active;
}
