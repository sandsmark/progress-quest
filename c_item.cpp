#include "c_item.h"
#include <QJsonObject>
#include <QJsonArray>

c_Item::c_Item()
{
    c_Item::clear();
}

void c_Item::clear()
{
    basename.clear();
    basegrade = 0;
    modifiers.clear();
    modgrades.clear();
    modprefix.clear();
    itemBonus = 0;
    Weight =0;
    itemType = Equipment::Any;
    price = 0;
    armorSlot = 0;
}

// TODO: allviate processing on multiple calls
QString c_Item::Name()
{
    QString rtn;
    rtn.clear();

    // start with bonus
    if (itemBonus != 0) {
        if (itemBonus > 0) {
            rtn += "+";
        }
        rtn += QString().number(itemBonus);
        rtn += " ";
    }

    // apply prefix modifiers
    for (int i = 0; i < modifiers.size(); i++) {
        if (modprefix.at(i)) {
            rtn += modifiers.at(i);
            rtn += " ";
        }
    }

    // add basename
    rtn += basename;

    // apply postfix mods
    for (int i = 0; i < modifiers.size(); i++) {
        if (! modprefix.at(i)) {
            rtn += " ";
            rtn += modifiers.at(i);
        }
    }

    return rtn;
}

void c_Item::setName(const QString &itemName)
{
    basename = itemName;
}


int c_Item::Bonus()
{
    return itemBonus;
}

void c_Item::setBonus(int bonus)
{
    itemBonus = bonus;
}

int c_Item::Appraisal()
{

    // factored appraisal
    if (price == 0) {
        int factor(0);
        int g = c_Item::Grade();

        // set super low grade
        if (g < 0) {
            g = 1;
            factor = 3;
        }
        // factor by grade
        else if (g < 20) {
            factor = 7;
        } else if (g < 50) {
            factor = 5;
        } else if (g < 100) {
            factor = 3;
        } else {
            factor = 1;
        }

        // factor by type (any = by grade only)
//        if      (itemType == pq_equip_weapon) factor += 8;
//        else if (itemType == pq_equip_shield) factor += 6;
//        else if (itemType == pq_equip_armor)  factor += 7;

        if (itemType == Equipment::Weapon) {
            factor += 18;
        } else if (itemType == Equipment::Shield) {
            factor += 26;
        } else if (itemType == Equipment::Armoy) {
            factor += 22;
        }

        // set price
        price = g * factor;
    }

    // appraised or set price
    return price;
}

void c_Item::setPrice(int gold)
{
    price = gold;
}

int c_Item::Grade()
{
    int g(0);

    // base value
    g += basegrade;

    // assess mods
    for (int i(0); i < modgrades.size(); i++) {
        //g += (float)g * ((float)modgrades.at(i)/5.0) ;
        g += modgrades.at(i);
    }

    // add bonus
    g += (float)itemBonus;

    return g;
}

Equipment c_Item::Type()
{
    return itemType;
}

void c_Item::setType(Equipment eqType)
{
    itemType = eqType;
}


// armor slot access is used by equip purchase
void c_Item::setASlot(int slot)
{
    armorSlot=slot;
}

int c_Item::getASlot()
{
    return armorSlot;
}

void c_Item::makeWeapon()
{
    QStringList cdata;
    cdata = gConfig->Weapons.at(rand() % gConfig->Weapons.size()).split("|");
    basename  = cdata.at(0);            //name
    basegrade = cdata.at(1).toInt();    //value
    Weight = 1;
    itemType = Equipment::Weapon;
}

void c_Item::addWeaponMod()
{
    QStringList cdata;
    cdata = gConfig->OffenseAttrib.at(rand() % gConfig->OffenseAttrib.size()).split("|");
    modifiers << cdata.at(0);            //mod
    modgrades << cdata.at(1).toInt();    //value
    modprefix << true;
}

void c_Item::addWeaponNegMod()
{
    QStringList cdata;
    cdata = gConfig->OffenseBad.at(rand() % gConfig->OffenseBad.size()).split("|");
    modifiers << cdata.at(0);            //mod
    modgrades << cdata.at(1).toInt();    //value
    modprefix << true;
}

void c_Item::makeSheild()
{
    QStringList cdata;
    cdata = gConfig->Shields.at(rand() % gConfig->Shields.size()).split("|");
    basename  = cdata.at(0);            //name
    basegrade = cdata.at(1).toInt();    //value
    Weight = 1;
    itemType = Equipment::Shield;
}

void c_Item::makeArmor()
{
    QStringList cdata;
    cdata = gConfig->Armors.at(rand() % gConfig->Armors.size()).split("|");
    basename  = cdata.at(0);            //name
    basegrade = cdata.at(1).toInt();    //value
    Weight = 1;
    itemType = Equipment::Armoy;
}

void c_Item::addDefMod()
{
    QStringList cdata;
    cdata = gConfig->DefenseAttrib.at(rand() % gConfig->DefenseAttrib.size()).split("|");
    modifiers << cdata.at(0);            //mod
    modgrades << cdata.at(1).toInt();    //value
    modprefix << true;
}

void c_Item::addDefNegMod()
{
    QStringList cdata;
    cdata = gConfig->DefenseBad.at(rand() % gConfig->DefenseBad.size()).split("|");
    modifiers << cdata.at(0);            //mod
    modgrades << cdata.at(1).toInt();    //value
    modprefix << true;
}


void c_Item::makeBitem()
{
    basename = gConfig->BoringItems.at(rand() % gConfig->BoringItems.size());
    basegrade = 1;
    Weight = 1;
    itemType = Equipment::Any;
}

void c_Item::makeSpecial()
{
    basename = gConfig->Specials.at(rand() % gConfig->Specials.size());
    //basegrade = 25 + (rand() % 20 - 10); // [15..34]
    basegrade = 15 + (rand() % 20 - 10); // [5..19]
    Weight = 1;
    itemType = Equipment::Any;
}

void c_Item::addOfMod()
{
    QString item_of;
    item_of = "of ";
    item_of += gConfig->ItemOfs.at(rand() % gConfig->ItemOfs.size());
    modifiers << item_of;
    modgrades << 250 + (rand() % 100 - 200); // [150..349]
    modprefix << false;
}

void c_Item::addAdjMod()
{
    modifiers <<  gConfig->ItemAttrib.at(rand() % gConfig->ItemAttrib.size());
    modgrades << 500 + (rand() % 100 - 200); // [400..599]
    modprefix << true;
}

void c_Item::makeClosestGrade(Equipment iType, int grade)
{
    QStringList *itemList;
    Equipment eqSelect = iType;

    // handle "any" selection
    if (eqSelect == Equipment::Any) {
        eqSelect = Equipment(rand() % 3); //random type
    }

    // choose list by type
    switch (eqSelect) {
    case Equipment::Weapon:
        itemList = &gConfig->Weapons;
        break;
    case Equipment::Shield:
        itemList = &gConfig->Shields;
        break;
    case Equipment::Armoy:
        itemList = &gConfig->Armors;
        break;
    case Equipment::Any:
        return;
    }

    bool found(false);
    int index(0), curGrade(0), lastDiff(0), closestGrade(0);
    QStringList cdata, listFound;

    // traverse all weapons - list exacts / find closest
    do {
        // extract grade from current item
        cdata = itemList->at(index).split("|");
        curGrade = cdata.at(1).toInt();

        // check equals - toggle found / add name
        if (curGrade == grade) {
            found = true;
            listFound.append(cdata.at(0)); // add name
        } else {
            // prime lastDiff (make larger than 0)
            if (lastDiff == 0) {
                lastDiff = abs(grade - curGrade);
            }

            // find closest grade
            if (abs(grade - curGrade) <= lastDiff) {
                lastDiff = abs(grade - curGrade);
                closestGrade = curGrade;
            }
        }
        index++;

    } while (index < itemList->size());

    // no exact grades found
    if (! found) {
        // use closest grade - find all -> listFound
        for (index=0; index < itemList->size(); index++) {
            cdata = itemList->at(index).split("|");
            if (cdata.at(1).toInt() == closestGrade) {
                listFound.append(cdata.at(0));
            }
        }
    }

    // randomize from names list (closest or exact)
    basename = listFound.at(rand() % listFound.size());
    if (found) {
        basegrade = grade;
    } else {
        basegrade = closestGrade;
    }
    Weight = 1;
    itemType = eqSelect;

}

QJsonObject c_Item::save()
{
    QJsonObject item;

    item["Type"] = int(itemType);
    item["Name"] = basename;
    item["BaseGrade"] = basegrade;
    item["Modifiers"] = c_Item::modListToArray(modifiers, modprefix, modgrades);
    item["Bonus"] = itemBonus;
    item["Price"] = price;
    item["ArmorSlot"] = armorSlot;
    item["Weight"] = Weight;

    QJsonObject root;
    root["Item"] = item;
    return root;
}

void c_Item::load(QJsonObject itemRoot)
{
    itemRoot = itemRoot["Item"].toObject();
    itemType = Equipment(itemRoot["Type"].toInt());
    basename = itemRoot["Name"].toString();
    basegrade = itemRoot["BaseGrade"].toInt();
    c_Item::arrayToModList(itemRoot["Modifiers"].toArray(),
                           modifiers,
                           modprefix,
                           modgrades
                          );
    itemBonus = itemRoot["Bonus"].toInt();
    price = itemRoot["Price"].toInt();
    armorSlot = itemRoot["ArmorSlot"].toInt();
    Weight = itemRoot["Weight"].toInt(1);
}

QJsonArray c_Item::modListToArray(QStringList &mList, QList<bool> &pList, QList<int> &gList)
{
    QJsonArray array;

    for (int i=0; i < mList.size(); i++) {
        QJsonArray set;

        set.append(mList.at(i));
        set.append(pList.at(i));
        set.append(gList.at(i));

        array.append(set);
    }
    return array;
}

void c_Item::arrayToModList(QJsonArray array, QStringList &mList, QList<bool> &pList, QList<int> &gList)
{
    // if not already - wipe lists
    mList.clear();
    pList.clear();
    gList.clear();

    // cycle through items
    for (int i=0; i < array.size(); i++) {
        // read in ordered set of values
        QJsonArray set = array[i].toArray();
        mList.append(set[0].toString());
        pList.append(set[1].toBool());
        gList.append(set[2].toInt());
    }
}
