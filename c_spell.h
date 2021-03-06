#ifndef C_SPELL_H
#define C_SPELL_H

#include <QString>
#include "pq7_config.h"
#include <iostream>
#include <fstream>

class c_Spell
{
public:
    c_Spell();
    c_Spell(int index);

    // accessors
    QString Name();
    QString Level();

    void incrSpellLevel();

    void setNameIndex(int index);
    void setRandName();
    void setSpellLevel(int level);

    // serialization methods
    QJsonObject save();
    void load(QJsonObject root);

private:
    int nameIndex, levelValue;

};

#endif // C_SPELL_H
