#include "mainwindow.h"
#include "ui_mainwindow.h"

// global super classes
c_Config *gConfig;
c_World *game;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{

    // timer drive
    pb_action_timer = new QTimer(this);
    // connect timer drive
    connect(pb_action_timer, &QTimer::timeout, this, &MainWindow::incr_pb_action_value);

    // new main classes
    game = new c_World;
    gConfig = new c_Config;

    // setup / show ui
    ui->setupUi(this);

    // focus policies
    ui->centralWidget->setFocusPolicy(Qt::StrongFocus);
    ui->centralWidget->setFocus();

    ui->tbl_inventory->horizontalHeader()->setStretchLastSection(true);


    // connect the save / load buttons (debugging)
    //connect(ui->btn_save, SIGNAL(released()), this, SLOT(gameSave()));
    //connect(ui->btn_load, SIGNAL(released()), this, SLOT(gameLoad()));

    // set current plot act (before initFrames)
    game->Act = 0;

    // inits for data to the ui
    //MainWindow::initPlayer();
    game->initPlayer();
    MainWindow::initFrames();

    //set state must be after initPlayer (for canBuy.. to work)
    if (MainWindow::canBuyEquipment(Equipment::Any)) {
        game->currentState = State::BuyingNewEquipment;
    } else {
        startAct();
    }

    // startup dialog
    Dialog_Opening startup;
    startup.exec();

    // if game was loaded then update ui throughly
    if (game->isLoaded) {
        MainWindow::postLoadUpdates();
    }

    // start actions
    MainWindow::setAction();
    pb_action_timer->start();

    ui->tbl_equipment->setGridStyle(Qt::SolidLine);
    ui->tbl_inventory->setGridStyle(Qt::SolidLine);
    ui->tbl_traits->setGridStyle(Qt::SolidLine);
    ui->tbl_stats->setGridStyle(Qt::SolidLine);
    ui->tbl_spells->setGridStyle(Qt::SolidLine);
}

MainWindow::~MainWindow()
{
    //MainWindow::gameSave();
    pb_action_timer->stop();
    game->save(game->Player->Name + QString::fromStdString(".pqd"));

    // cut - that's lunch.
    delete ui;
}

void MainWindow::incr_pb_action_value()
{
    if (game->isDebugFlagSet(pq_debug_always_trigger_action)) {
        ui->pb_action->setValue(99);
    }

    int value = ui->pb_action->value();
    if (++value < ui->pb_action->maximum()) {
        ui->pb_action->setValue(value);
    } else {
        ui->pb_action->setValue(value % ui->pb_action->maximum());

        switch (game->currentState) {
        case State::Reserved1:
            qWarning() << "increase Invalid state" << int(game->currentState);
            break;
        case State::HeadingToKillingFields:
            break;
        case State::Fighting:
            MainWindow::increaseExperience();

            MainWindow::progressQuest();

            MainWindow::updateEncumberance();
            MainWindow::addMonsterDrop();
            break;
        case State::HeadingToTown:
            break;
        case State::SellingOff:
            MainWindow::removeInventoryItem();
            MainWindow::updateEncumberance();
            break;
        case State::BuyingNewEquipment:
            MainWindow::buyEquipment();
            break;
        case State::InterplotCinematic:
            break;
        }

        MainWindow::transitionState();
    }

    if (game->isDebugFlagSet(pq_debug_action_triggers_save)) {
        MainWindow::gameSave();
    }

    game->pb_action = ui->pb_action->value();
}

void MainWindow::updateEncumberance()
{
    // get percent value of encumbrance
    int value = int(((float)game->Player->Encumbrance() / (float)game->Player->maxEncumbrance() * 100.0));

    // set value to bar
    ui->pb_encumbrance->setValue(value);

    game->pb_encumbrance = ui->pb_encumbrance->value();
}

void MainWindow::progressPlot()
{
    int value = ui->pb_plot->value() + (rand() % 6 + 1);
    if (value < ui->pb_plot->maximum()) {
        // progress plot
        ui->pb_plot->setValue(value);

    } else {
        // completed plot act - add new one

        //      reset progress bar
        ui->pb_plot->setValue(value % ui->pb_plot->maximum());

        //      check current act
        ui->lst_plot->setCurrentRow(ui->lst_plot->count() - 1);
        ui->lst_plot->currentItem()->setCheckState(Qt::Checked);

        //      clear quests and start new one
        ui->lst_quests->clear();
        game->quests.clear();
        MainWindow::addQuest(MainWindow::generateQuest());

        //      add new act
        game->Act++;
        MainWindow::addAct();

        startAct();
    }
    game->pb_plot = ui->pb_plot->value();
}

void MainWindow::progressQuest()
{
    int value = ui->pb_quest->value() + (rand() % 4 + 1);
    if (value < ui->pb_quest->maximum()) {
        // progress quest
        ui->pb_quest->setValue(value);

    } else {
        // completed quest - add new one

        //      reset progress bar
        ui->pb_quest->setValue(value % ui->pb_quest->maximum());

        //      check last item in list
        ui->lst_quests->setCurrentRow(ui->lst_quests->count() - 1);
        ui->lst_quests->currentItem()->setCheckState(Qt::Checked);

        //      add new and progress plot
        game->quests.push_back(MainWindow::generateQuest());
        MainWindow::addQuest(game->quests.at(game->quests.size() - 1));
        MainWindow::progressPlot();
    }
    game->pb_quest = ui->pb_quest->value();
}

void MainWindow::increaseExperience()
{

    if (game->currentState == State::Fighting) {

        // incr player xp
        game->Player->XP = game->Player->XP.number(game->Player->XP.toULongLong() + game->Monster->winXP());

        // create pb value from player percentage
        int value = (int)(gConfig->fnPercentOf(game->Player->XP.toULongLong(), game->Player->maxXP()));

        // is < 100% ?
        if (value < ui->pb_experience->maximum()) {
            ui->pb_experience->setValue(value);

        } else {
            // level up
            //      incr level
            game->Player->increaseLevel();
            ui->tbl_traits->setCurrentCell(3,1);
            ui->tbl_traits->currentItem()->setText(game->Player->Level);

            //      reset progress bar
            value = (value % ui->pb_experience->maximum());
            ui->pb_experience->setValue((int)(gConfig->fnPercentOf((qulonglong)value,game->Player->maxXP())));


            // win better stats
            MainWindow::winStats();

            // win new spells
            MainWindow::winSpells();

        }
    }
    game->pb_experience = ui->pb_experience->value();
}

void MainWindow::initFrames()
{
    QListWidgetItem lstItemChecked, lstItemUnchecked;
    lstItemChecked.setForeground(Qt::gray);
    lstItemChecked.setCheckState(Qt::Checked);
    lstItemUnchecked.setCheckState(Qt::Unchecked);


    // traits
    for (int i = 0; i < gConfig->Traits.size(); i++) {
        ui->tbl_traits->setItem(i, 0, new QTableWidgetItem(gConfig->Traits.at(i)));
    }

    ui->tbl_traits->setItem(0, 1, new QTableWidgetItem(game->Player->Name));
    ui->tbl_traits->setItem(1, 1, new QTableWidgetItem(game->Player->Race));
    ui->tbl_traits->setItem(2, 1, new QTableWidgetItem(game->Player->Voc));
    ui->tbl_traits->setItem(3, 1, new QTableWidgetItem(game->Player->Level));
    ui->tbl_traits->resizeColumnsToContents();


    // stats
    MainWindow::updateStatsTable();

    // spells
    MainWindow::updateSpellTable();

    // equipment
    MainWindow::updateEquipmentTable();

    // list inventory
    MainWindow::updateInventoryTable();

    // list plotline values
    if (game->Act == 0) {
        //do two to succeed prologue
        MainWindow::addAct();
        ui->lst_plot->currentItem()->setCheckState(Qt::Checked);
        game->Act++;
        MainWindow::addAct();
    } else {
        /*
            If loaded with value higher from old save
            this will just start the list from last Act.
            This effectively cleans the act list over time
            unless the user never shutsdown / saves and
            reloads.
        */
        MainWindow::addAct();
    }

    // quests update
    MainWindow::updateQuestList();

    game->interplotCinematic = gConfig->InterplotCinematics[rand() % gConfig->InterplotCinematics.count()];
    game->currentState = State::InterplotCinematic;

}

QString MainWindow::generateQuest()
{
    /*
       3 types of quests:
            Hunt, Fetch, Seek

       Hunt are for monsters. Fetch is for boring items. Seek is some special item.
    */

    const int questType = rand() % 3 + 1;

    QString questText;
    QStringList cdata;

    c_Item special;

    switch (questType) {
    case 1:
        // Hunt quest

        switch (rand() % 3) {
        case 1:
            questText = tr("Quell the %1");
            break;
        case 2:
            questText = tr("Placate the %1");
            break;
        default:
            questText = tr("Hunt the %1");
        }

        //      object
        cdata = gConfig->Monsters.at(rand() % gConfig->Monsters.size()).split("|");
        questText = questText.arg(cdata.at(0)); // name
        break;

    case 2:
        // Fetch quest

        switch (rand() % 4) {
        case 1:
            questText = tr("Deliver this %1");
            break;
        case 2:
            questText = tr("Collect payment for this %1");
            break;
        case 3:
            questText = tr("Hide this %1");
            break;
        default:
            questText = tr("Fetch my %1");
        }

        //      object
        cdata << gConfig->BoringItems.at(rand() % gConfig->BoringItems.size());
        questText = questText.arg(cdata.at(0));
        break;

    case 3:
        // Seek quest

        switch (rand() % 3) {
        case 1:
            questText = tr("Inquire after the %1");
            break;
        case 2:
            questText = tr("Aspire for the %1");
            break;
        default:
            questText = tr("Seek the %1");
        }

        //      object
        special.makeSpecial();
        if (rand() % 3 == 0) {
            special.addAdjMod();
        }
        questText = questText.arg(special.Name());
        break;

    default:
        questText = "?";
    }

    return questText;
}

void MainWindow::addQuest(QString name)
{
    // add to world object
    //game->quests.push_back(name);

    // add to quests listbox
    ui->lst_quests->addItem(new QListWidgetItem());
    ui->lst_quests->setCurrentRow(ui->lst_quests->count() - 1);
    ui->lst_quests->currentItem()->setText(name);
    ui->lst_quests->currentItem()->setCheckState(Qt::PartiallyChecked);
}

void MainWindow::addAct()
{
    QString actName;

    if (game->Act == 0) {
        actName = tr("Prologue");
    } else {
        actName = tr("Act %1").arg(gConfig->toRoman(game->Act));
    }

    ui->lst_plot->addItem(new QListWidgetItem());
    ui->lst_plot->setCurrentRow(ui->lst_plot->count() - 1);
    ui->lst_plot->currentItem()->setText(actName);
    ui->lst_plot->currentItem()->setCheckState(Qt::PartiallyChecked);
}

void MainWindow::startAct()
{
    if (game->Nemesis) {
        game->Nemesis->deleteLater();
        game->Nemesis = nullptr;
    }

    game->Nemesis = new c_Monster;
    game->Nemesis->makeByLevel(game->Player->Level.toInt() + 3);

    game->interplotCinematic = gConfig->InterplotCinematics[rand() % gConfig->InterplotCinematics.count()];
    game->currentState = State::InterplotCinematic;
    qDebug() << "starting plot" << game->interplotCinematic.first();
}

void MainWindow::setAction()
{
    game->Action.clear();
    switch (game->currentState) {
    case State::Reserved1:
        qWarning() << "set action invalid state" << int(game->currentState);
        // reserved
        break;
    case State::HeadingToKillingFields:
        // heading to killing fields
        game->Action = tr("Heading to the Killing Fields");
        //pb_action_timer->setInterval(75);
        game->actionTime = 75;
        break;
    case State::Fighting:
        // fight
        MainWindow::setMonster();
        game->Action = tr("Executing %1").arg(gConfig->Indefinite(game->Monster->description()));
        game->actionTime = 50;
        break;
    case State::HeadingToTown:
        // back to Town
        game->Action = tr("Going back to Town to sell off");
        game->actionTime = 75;
        break;
    case State::SellingOff:
        // selling off
        game->Action = tr("Selling %1").arg(MainWindow::sellInventoryItem());
        game->actionTime = 20;
        break;
    case State::BuyingNewEquipment:
        // shopping!!
        game->Action = tr("Negotiating purchase of %1 %2 ").arg(
                       game->Player->purchaseType()).arg(
                       game->Player->Purchase->Name());

        switch (game->Player->Purchase->Type()) {
        case Equipment::Weapon:
            game->Action += tr("(weapon)");
            break;
        case Equipment::Shield:
            game->Action += tr("(shield)");
            break;
        case Equipment::Armoy:
            game->Action += tr("(armor)");
            break;
        case Equipment::Any:
            break;
        }
        game->actionTime = 35;
        break;
    case State::InterplotCinematic: {
        const QStringList plot =  game->interplotCinematic.takeFirst().split('|');
        Q_ASSERT(plot.count() == 3);
        game->Action = plot[2];
        game->Action.replace('*', game->Nemesis->description());
        game->actionTime = plot[1].toInt() * 10;
        break;
    }
    default:
        game->Action = tr("You are lost in another plane of existance");
        game->actionTime = 1000;
    }

    // update final conditions
    ui->lbl_action->setText(game->Action);
    if (game->isDebugFlagSet(pq_debug_zero_action_timer)) {
        pb_action_timer->setInterval(1);
    } else {
        pb_action_timer->setInterval(game->actionTime);
    }
}

void MainWindow::transitionState()
{
    switch (game->currentState) {
    case State::Reserved1:
        game->currentState = State::HeadingToKillingFields;
        break;

    case State::HeadingToKillingFields:
        game->currentState = State::Fighting;
        break;

    case State::Fighting:
        if (game->Player->Encumbrance() > game->Player->maxEncumbrance()) {
            game->currentState = State::HeadingToTown;
        }
        break;

    case State::HeadingToTown:
        game->currentState = State::SellingOff;
        break;

    case State::SellingOff:
        if (game->Player->Inventory.empty()) {
            if (MainWindow::canBuyEquipment(Equipment::Any)) {
                game->currentState = State::BuyingNewEquipment;
            } else {
                game->currentState = State::HeadingToKillingFields;
            }
        }
        break;

    case State::BuyingNewEquipment:
        if (! MainWindow::canBuyEquipment(Equipment::Any)) {
            game->currentState = State::HeadingToKillingFields;
        }
        break;

    case State::InterplotCinematic:
        if (game->interplotCinematic.isEmpty()) {
            game->currentState = State::HeadingToKillingFields;
        }
        break;

    default:
        qWarning() << "Invalid state" << int(game->currentState);
        game->currentState = State::Reserved1;
    }

    MainWindow::setAction();
}

void MainWindow::setMonster()
{
    bool success = false;

    // determine monster level from player's
    int level = game->Player->Level.toInt();
    level = level - (rand() % 10); // reduce monster lv up to -9 levels
    if (level < 0) {
        level = 0;    // no less than 0
    }

    // fill out monster
    do {
        game->Monster->clear();

        if (game->Monster->makeByLevel(level) ||
                game->Monster->makeMounted(level) ||
                game->Monster->makeGroup(level)
           ) {
            success = true;
        }
    } while (! success); // potential infinite
}

void MainWindow::addMonsterDrop()
{

    c_Item *drop;

    // traverse all drop for this monster
    for (int i = 0; i < game->Monster->Drops().size(); i++) {

        drop = new c_Item;
        bool found(false);

        // create a monster drop item
        drop->setName(game->Monster->Drops().at(i));
        drop->Weight = 1;
        drop->setType(Equipment::Any);
        if (game->Monster->Level().toInt() < 0) {
            drop->setPrice(0);
        } else {
            drop->setPrice(game->Monster->Level().toInt());
        }

        // if found in inventory (match name and price), add another
        for (int t = 0; t < game->Player->Inventory.size(); t++) {
            if (game->Player->Inventory.at(t)->Name() == drop->Name() &&
                    game->Player->Inventory.at(t)->Appraisal() == drop->Appraisal()
               ) {
                game->Player->Quantity[t] = game->Player->Quantity.at(t) + 1;
                ui->tbl_inventory->setCurrentCell(t+1, 1);
                found = true;
            }
        }

        // add if not
        if (! found) {
            game->Player->Inventory.append(drop);
            game->Player->Quantity.append(1);
            ui->tbl_inventory->setCurrentCell(ui->tbl_inventory->rowCount(), 1);
        }
    }

    // add special - 30% chance
    if ((game->Monster->isSpecial) && (rand() % 3 == 0)) {
        drop = new c_Item;
        drop->makeSpecial();
        if (rand() % 2 == 0) {
            drop->addAdjMod();
        }
        game->Player->Inventory.append(drop);
        game->Player->Quantity.append(1);
        ui->tbl_inventory->setCurrentCell(ui->tbl_inventory->rowCount(), 1);
        //delete drop;
    }

    MainWindow::updateInventoryTable();
}

void MainWindow::removeInventoryItem()
{
    // total sale gold value
    int saleValue = game->Player->Quantity.last() * game->Player->Inventory.last()->Appraisal();

    // trade items for gold
    game->Player->Gold += saleValue;
    game->Player->Inventory.removeLast();
    game->Player->Quantity.removeLast();
    ui->tbl_inventory->removeRow(ui->tbl_inventory->rowCount());

    MainWindow::updateInventoryTable();
}

QString MainWindow::sellInventoryItem()
{
    // reverse through player inventory and remove table
    // contents as you go

    QString build;
    int saleValue = game->Player->Quantity.last() * game->Player->Inventory.last()->Appraisal();

    // build a return: "q whatever[s] for n gold"
    build = QString().number(game->Player->Quantity.last()) + tr(" ");

    if (game->Player->Quantity.last() > 1) {
        build += gConfig->sufPlural(game->Player->Inventory.last()->Name()) + tr(" ");
    } else {
        build += game->Player->Inventory.last()->Name() + tr(" ");
    }

    build += tr("for ") + QString().number(saleValue) + tr(" gold");

    return build;
}

void MainWindow::winStats()
{
    int stat_bonus = rand() % 4 + 4;
    do {
        if ((rand() % 3 == 0) && (stat_bonus > 0)) {
            game->Player->STR = game->Player->STR.number(game->Player->STR.toInt() + 1);
            stat_bonus--;
        }
        if ((rand() % 3 == 0) && (stat_bonus > 0)) {
            game->Player->INT = game->Player->INT.number(game->Player->INT.toInt() + 1);
            stat_bonus--;
        }
        if ((rand() % 3 == 0) && (stat_bonus > 0)) {
            game->Player->WIS = game->Player->WIS.number(game->Player->WIS.toInt() + 1);
            stat_bonus--;
        }
        if ((rand() % 3 == 0) && (stat_bonus > 0)) {
            game->Player->DEX = game->Player->DEX.number(game->Player->DEX.toInt() + 1);
            stat_bonus--;
        }
        if ((rand() % 3 == 0) && (stat_bonus > 0)) {
            game->Player->CON = game->Player->CON.number(game->Player->CON.toInt() + 1);
            stat_bonus--;
        }
        if ((rand() % 3 == 0) && (stat_bonus > 0)) {
            game->Player->CHA = game->Player->CHA.number(game->Player->CHA.toInt() + 1);
            stat_bonus--;
        }
    } while (stat_bonus > 0);

    game->Player->HPMax = game->Player->HPMax.number(game->Player->HPMax.toInt() + (rand() % 12 + 1));
    game->Player->MPMax = game->Player->MPMax.number(game->Player->MPMax.toInt() + (rand() % 8 + 1));

    MainWindow::updateStatsTable();
}

void MainWindow::winSpells()
{
    if (ui->tbl_spells->rowCount() >= gConfig->Spells.size()) {
        return;    // guard vs unneeded
    }

    bool found = false;

    c_Spell *spell = new c_Spell;

    // incr current spell levels
    for (int i(0); i < game->Player->Spells.size(); i++) {
        if (rand() % 3 == 0) {
            game->Player->Spells.at(i)->incrSpellLevel();
        }
    }

    // maybe a new spell?
    spell->setRandName();

    // find if new spell exists
    for (int i(0); i < game->Player->Spells.size(); i++) {
        if (game->Player->Spells.at(i)->Name() == spell->Name()) {
            found = true;
        }
    }

    // discard if found
    if (found) {
        delete spell;
    } else {
        //add to list
        game->Player->Spells.append(spell);
    }

    MainWindow::updateSpellTable();
}

bool MainWindow::canBuyEquipment(Equipment eqtype)
{
    // chk for spend limit must populate Player's purchase item
    int spendCap = gConfig->fnPercent(game->Player->Gold, 40); // 40% total gold
    game->Player->Purchase = MainWindow::getPurchaseItem(eqtype);
    return (game->Player->Purchase->Appraisal() <= spendCap);
}

c_Item *MainWindow::getPurchaseItem(Equipment eqtype)
{
    c_Item *itemForPurchase{};
    Equipment eqSelect = eqtype;
    int pick(0);

    // randomize "any" type to weapon, shield, or armor
    if (eqtype == Equipment::Any) {
        int r = rand() % 11; // total peices of eq
        if (r == 0) {
            eqSelect = Equipment::Weapon;
        } else if (r == 1) {
            eqSelect = Equipment::Shield;
        } else {
            eqSelect = Equipment::Armoy;
        }
    }

    // set item to purchase (slot used for armor slots)
    switch (eqSelect) {

    case Equipment::Weapon:
        // if not filled - buy first one
        if (game->Player->Weapon->Name() == tr("")) {
            itemForPurchase = game->makeEqByGrade(eqSelect, -4);
            game->Player->setPurchNew(true);
        } else {
            // upgrade
            itemForPurchase = MainWindow::upgradeEquipment(eqSelect, game->Player->Weapon->Grade());
            game->Player->setPurchNew(false);
        }
        break;

    case Equipment::Shield:
        if (game->Player->Shield->Name() == tr("")) {
            itemForPurchase = game->makeEqByGrade(Equipment::Shield, game->Player->Level.toInt());
            game->Player->setPurchNew(true);
        } else {
            itemForPurchase = MainWindow::upgradeEquipment(eqSelect, game->Player->Shield->Grade());
            game->Player->setPurchNew(false);
        }
        break;
    case Equipment::Armoy:

        // pick random armor
        pick = rand() % game->Player->Armor.size();

        if (game->Player->Armor.at(pick)->Name() == tr("")) {
            itemForPurchase = game->makeEqByGrade(Equipment::Armoy, game->Player->Level.toInt());
            itemForPurchase->setASlot(pick);
            game->Player->setPurchNew(true);
        } else {
            itemForPurchase = MainWindow::upgradeEquipment(eqSelect, game->Player->Armor.at(pick)->Grade());
            itemForPurchase->setASlot(pick);
            game->Player->setPurchNew(false);
        }
        break;
    case Equipment::Any:
        Q_ASSERT(false);
        return nullptr;
    }
    return itemForPurchase;
}

void MainWindow::buyEquipment()
{
    // buy purchase item
    game->Player->Gold -= game->Player->Purchase->Appraisal();

    // drop old and equip new
    switch (game->Player->Purchase->Type()) {
    case Equipment::Weapon:
        delete game->Player->Weapon;
        game->Player->Weapon = game->Player->Purchase;
        break;
    case Equipment::Shield:
        delete game->Player->Shield;
        game->Player->Shield = game->Player->Purchase;
        break;
    case Equipment::Armoy:
        delete game->Player->Armor[game->Player->Purchase->getASlot()];
        game->Player->Armor[game->Player->Purchase->getASlot()] = game->Player->Purchase;
        break;
    case Equipment::Any:
        //fault - shouldn't happen
        break;
    }

    //delete game->Player->Purchase;

    MainWindow::updateInventoryTable();
    MainWindow::updateEquipmentTable();
}


c_Item *MainWindow::upgradeEquipment(Equipment eqtype, int grade)
{
    c_Item *equip = new c_Item;
    Equipment eqSelect = eqtype;

    if (eqSelect == Equipment::Any) {
        eqSelect = Equipment(rand() % 3);
    }

    switch (eqSelect) {
    case Equipment::Weapon:
        equip = game->makeEqByGrade(eqSelect, grade + 1);
        break;
    case Equipment::Shield:
        equip = game->makeEqByGrade(eqSelect, grade + 1);
        break;
    case Equipment::Armoy:
        equip = game->makeEqByGrade(eqSelect, grade + 1);
        break;
    case Equipment::Any:
        break;
    }
    return equip;
}

void MainWindow::updateInventoryTable()
{
    ui->tbl_inventory->clearContents();
    ui->tbl_inventory->setRowCount(1 + game->Player->Inventory.size());

    // gold first
    ui->tbl_inventory->setItem(0, 0, new QTableWidgetItem("Gold"));
    ui->tbl_inventory->setItem(0, 1, new QTableWidgetItem(QString::number(game->Player->Gold)));

    // remaining inv list
    for (int i(0); i < game->Player->Inventory.size(); i++) {
        ui->tbl_inventory->setItem(i+1, 0, new QTableWidgetItem(game->Player->Inventory.at(i)->Name()));
        ui->tbl_inventory->setItem(i+1, 1, new QTableWidgetItem(QString::number(game->Player->Quantity.at(i))));
    }

    ui->tbl_inventory->resizeColumnsToContents();

    ui->lbl_inventory->setText(tr("Inventory %1 units").arg(game->Player->Encumbrance()));
}


void MainWindow::updateEquipmentTable()
{
    // wipe
    ui->tbl_equipment->clearContents();
    ui->tbl_equipment->setRowCount(2 + game->Player->Armor.size());

    // reload equipment from Player and config
    ui->tbl_equipment->setRowCount(gConfig->Equips.size());
    for (int i = 0; i < gConfig->Equips.size(); i++) {
        ui->tbl_equipment->setItem(i, 0, new QTableWidgetItem(gConfig->Equips.at(i)));
    }
    ui->tbl_equipment->setItem(0, 1, new QTableWidgetItem(game->Player->Weapon->Name()));
    ui->tbl_equipment->setItem(1, 1, new QTableWidgetItem(game->Player->Shield->Name()));
    for (int i(0); i < game->Player->Armor.size(); i++) {
        ui->tbl_equipment->setItem(i + 2, 1, new QTableWidgetItem(game->Player->Armor.at(i)->Name()));
    }

    ui->tbl_equipment->resizeColumnsToContents();
}

void MainWindow::updateStatsTable()
{
    // wipe
    ui->tbl_stats->clearContents();
    ui->tbl_stats->setRowCount(2 + gConfig->PrimeStats.size());

    // stats
    for (int i = 0; i < gConfig->PrimeStats.size(); i++) {
        ui->tbl_stats->setItem(i, 0, new QTableWidgetItem(gConfig->PrimeStats.at(i)));
    }
    ui->tbl_stats->setItem(ui->tbl_stats->rowCount() - 2, 0, new QTableWidgetItem(tr("HP Max")));
    ui->tbl_stats->setItem(ui->tbl_stats->rowCount() - 1, 0, new QTableWidgetItem(tr("MP Max")));

    //      Load stats from player
    ui->tbl_stats->setItem(0, 1, new QTableWidgetItem(game->Player->STR));
    ui->tbl_stats->setItem(1, 1, new QTableWidgetItem(game->Player->INT));
    ui->tbl_stats->setItem(2, 1, new QTableWidgetItem(game->Player->WIS));
    ui->tbl_stats->setItem(3, 1, new QTableWidgetItem(game->Player->DEX));
    ui->tbl_stats->setItem(4, 1, new QTableWidgetItem(game->Player->CON));
    ui->tbl_stats->setItem(5, 1, new QTableWidgetItem(game->Player->CHA));
    ui->tbl_stats->setItem(6, 1, new QTableWidgetItem(game->Player->HPMax));
    ui->tbl_stats->setItem(7, 1, new QTableWidgetItem(game->Player->MPMax));

    ui->tbl_stats->resizeColumnsToContents();
}

void MainWindow::updateSpellTable()
{
    // clear table
    ui->tbl_spells->clearContents();

    // spell list
    ui->tbl_spells->setRowCount(game->Player->Spells.size());
    for (int i = 0; i < game->Player->Spells.size(); i++) {
        ui->tbl_spells->setItem(i, 0, new QTableWidgetItem(game->Player->Spells.at(i)->Name()));
        ui->tbl_spells->setItem(i, 1, new QTableWidgetItem(game->Player->Spells.at(i)->Level()));
    }

    ui->tbl_spells->resizeColumnsToContents();
}

void MainWindow::gameSave()
{
    /*
     *      Send world info to file
     */

    pb_action_timer->stop();

    game->save();

    /*
     * this is unnecessary for "save on quit" but is left
     * because it doesn't interfere and is needed for "save
     * while running"
    */
    pb_action_timer->start();
}

//void MainWindow::gameLoad()
//{
//    /*
//     *      Reverse save data back into world instance
//     */
//    pb_action_timer->stop();

//    game->load();

//    MainWindow::postLoadUpdates();

//    pb_action_timer->start();
//}

void MainWindow::postLoadUpdates()
{
    /*
     *      Updates for ui / modal after load
     *      info is poped into game.
     */

    // pb updates drive the game data - so pb's must be
    // filled from modal dataspace on load return
    ui->pb_action->setValue(game->pb_action);
    ui->pb_encumbrance->setValue(game->pb_encumbrance);
    ui->pb_experience->setValue(game->pb_experience);
    ui->pb_plot->setValue(game->pb_plot);
    ui->pb_quest->setValue(game->pb_quest);

    // handle quests
    MainWindow::updateQuestList();

    // update action conditions
    ui->lbl_action->setText(game->Action);
    pb_action_timer->setInterval(game->actionTime);

    // update char sht label for debug mode
    if (!game->isDebugFlagSet(pq_debug_none)) {
        ui->lbl_char->setText("Character Sheet [debug mode]");
        ui->lbl_char->update();
    } else {
        ui->lbl_char->setText("Character Sheet");
        ui->lbl_char->update();
    }

    ui->lst_plot->clear(); // wipe old plot list before initFrames

    // handle most ui update functions
    MainWindow::initFrames();
}

void MainWindow::updateQuestList()
{
    // update the quest list box from game data
    ui->lst_quests->clear();
    if (game->quests.size() > 0) {
        // handle full quests list in game data
        MainWindow::addQuest(game->quests.at(0)); // prime the list
        for (int i=1; i < game->quests.size(); i++) {
            //      check last item in list
            ui->lst_quests->setCurrentRow(ui->lst_quests->count() - 1);
            ui->lst_quests->currentItem()->setCheckState(Qt::Checked);

            // add current to end of listbox
            MainWindow::addQuest(game->quests.at(i));
        }
    } else {
        // no quests in game data - add one
        game->quests.append(MainWindow::generateQuest());
        MainWindow::addQuest(game->quests.at(0));
    }
}


void MainWindow::keyPressEvent(QKeyEvent *k)
{
    /*
     *      QT 4.8 Docs specify that unhandled keys must be
     *      sent back to the QWidget handler so certain default
     *      behaviors will occur
     */
    if (game->isDebugFlagSet(pq_debug_active)) {
        // debug is active here
        if (k->key() == Qt::Key_Escape) {
            // deactivate debug
            game->debugClear();
            ui->lbl_char->setText("Character Sheet");
            ui->lbl_char->update();
            return;
        }

        // we are already in debug mode
        if (k->key() == Qt::Key_R) { // reset debug - without losing mode
            game->debugActive();
            return;
        }

        if (k->key() == Qt::Key_0) { // toggle zero action time
            game->toggleDebugFlag(pq_debug_zero_action_timer);
            return;
        }

        if (k->key() == Qt::Key_S) { // toggle save on action
            game->toggleDebugFlag(pq_debug_action_triggers_save);
            return;
        }

        if (k->key() == Qt::Key_A) { // toggle always trigger action
            game->toggleDebugFlag(pq_debug_always_trigger_action);
            return;
        }
    } else {
        // not in debug mode - yet
        if (k->key() == Qt::Key_Escape) {
            // activate debug
            game->debugActive();
            ui->lbl_char->setText("Character Sheet [debug mode]");
            ui->lbl_char->update();
        }
    }
}
