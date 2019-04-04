#include "c_spell.h"

#include <QJsonObject>

// default constr
c_Spell::c_Spell()
{
    //c_Spell::setNameIndex(0);
    //c_Spell::setSpellLevel(1);
    nameIndex = 0;
    levelValue = 1;
}

// constr with name index
c_Spell::c_Spell(int index)
{
    c_Spell::setNameIndex(index);
    c_Spell::setSpellLevel(1);
}


// accessor methods
void c_Spell::setNameIndex(int index)
{
//    c_Config cfg;
    if (index >= gConfig->Spells.size()) {
        nameIndex = gConfig->Spells.size() - 1;
    }

    else {
        if (index < 0) {
            nameIndex = 0;
        }

        else {
            nameIndex = index;
        }
    }
}

void c_Spell::setSpellLevel(int level)
{
    if (level < 1) {
        levelValue = 1;
    }

    else {
        levelValue = level;
    }
}


void c_Spell::incrSpellLevel()
{
    levelValue++;
}

void c_Spell::setRandName()
{
//    c_Config cfg;
    nameIndex = rand() % gConfig->Spells.size();
}

QString c_Spell::Name()
{
//    c_Config cfg;
    return gConfig->Spells.at(nameIndex);
}

QString c_Spell::Level()
{
    return gConfig->toRoman(levelValue);
}

QJsonObject c_Spell::save()
{
    QJsonObject spell;
    spell["NameIndex"] = nameIndex;
    spell["Level"] = levelValue;
    return spell;
}

void c_Spell::load(QJsonObject root)
{
    nameIndex = root["NameIndex"].toInt();
    levelValue = root["Level"].toInt();
}
