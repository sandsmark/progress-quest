#include "dialog_charsheet.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QButtonGroup>
#include <QGroupBox>
#include <QRadioButton>
#include <QDebug>

#include <algorithm>
#include <random>

Dialog_charSheet::Dialog_charSheet(QWidget *parent) :
    QDialog(parent),
    m_randGen(std::random_device()())
{
    setWindowTitle("Progress Quest - New Character");
    QHBoxLayout *topLayout = new QHBoxLayout;

    m_nameEdit = new QLineEdit;
    connect(m_nameEdit, &QLineEdit::textChanged, this, &Dialog_charSheet::onNameChanged);
    topLayout->addWidget(m_nameEdit);

    QPushButton *generateNameButton = new QPushButton("?");
    connect(generateNameButton, &QPushButton::clicked, this, &Dialog_charSheet::generateName);
    topLayout->addWidget(generateNameButton);

    QHBoxLayout *paramLayout = new QHBoxLayout;

    m_raceButtons = createGroup("Race", gConfig->Races, paramLayout);
    m_classButtons = createGroup("Class", gConfig->Klasses, paramLayout);

    QGroupBox *statsGroupBox = new QGroupBox("Stats");
    QGridLayout *statsLayout = new QGridLayout;
    statsGroupBox->setLayout(statsLayout);

    const QStringList stats({"STR", "INT", "WIS", "DEX", "CON", "CHA"});

    int statItemNum = 0;
    for (statItemNum=0; statItemNum<stats.count(); statItemNum++) {
        const QString &stat = stats[statItemNum];
        statsLayout->addWidget(new QLabel(stat), statItemNum, 0);

        QLineEdit *statEdit = new QLineEdit("0");
        statEdit->setReadOnly(true);
        statsLayout->addWidget(statEdit, statItemNum, 1);

        m_statsEdits[stat] = statEdit;
    }
    statsLayout->setRowMinimumHeight(statItemNum++, 50);

    m_totalStatCount = new QLineEdit("0");
    m_totalStatCount->setReadOnly(true);
    statsLayout->addWidget(new QLabel("Total"), statItemNum, 0);
    statsLayout->addWidget(m_totalStatCount, statItemNum++, 1);

    QPushButton *rerollButton = new QPushButton("Roll");
    statsLayout->addWidget(rerollButton, statItemNum++, 0, 1, 2);
    m_unrollButton = new QPushButton("Unroll");
    statsLayout->addWidget(m_unrollButton, statItemNum++, 0, 1, 2);

    connect(rerollButton, &QPushButton::clicked, this, &Dialog_charSheet::doRoll);
    connect(m_unrollButton, &QPushButton::clicked, this, &Dialog_charSheet::doUnroll);

    statsLayout->addItem(new QSpacerItem(10, 10, QSizePolicy::Minimum, QSizePolicy::Expanding), statItemNum++, 0, 1, 2);

    paramLayout->addWidget(statsGroupBox);


    QHBoxLayout *bottomLayout = new QHBoxLayout;
    bottomLayout->addItem(new QSpacerItem(10, 10, QSizePolicy::Expanding, QSizePolicy::Minimum));
    m_soldButton = new QPushButton("Sold!");
    connect(m_soldButton, &QPushButton::clicked, this, &Dialog_charSheet::transferCharToGame);
    bottomLayout->addWidget(m_soldButton);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(topLayout);
    mainLayout->addLayout(paramLayout);
    mainLayout->addLayout(bottomLayout);
    setLayout(mainLayout);



    generateName();
    onClassOrRaceChanged();
    doRoll();
    m_unrollButton->setEnabled(false);
}

Dialog_charSheet::~Dialog_charSheet()
{
}

void Dialog_charSheet::transferCharToGame()
{
    // silent ignore non-named
    //if (ui->lne_name->text().isEmpty())
    //    return;

    //      Traits
    game->Player->Name  = m_nameEdit->text();
    game->Player->Race  = m_raceButtons->checkedButton()->text();
    game->Player->Voc   = m_classButtons->checkedButton()->text();

    //      Stats
    game->Player->STR   = m_statsEdits["STR"]->text();
    game->Player->INT   = m_statsEdits["INT"]->text();
    game->Player->WIS   = m_statsEdits["WIS"]->text();
    game->Player->DEX   = m_statsEdits["DEX"]->text();
    game->Player->CON   = m_statsEdits["CON"]->text();
    game->Player->CHA   = m_statsEdits["CHA"]->text();

    // set isLoaded to let everyone know we accepted a new char
    game->isLoaded = true;

    // shutdown dialog
    this->close();
}

void Dialog_charSheet::onNameChanged(const QString &name)
{
    m_soldButton->setEnabled(!name.isEmpty());
}

void Dialog_charSheet::generateName()
{
    const QList<QStringList> namePartGroups({
        {"br", "cr", "dr", "fr", "gr", "j", "kr", "l", "m", "n", "pr", "", "", "", "r", "sh", "tr", "v", "wh", "x", "y", "z",},
        {"a", "a", "e", "e", "i", "i", "o", "o", "u", "u", "ae", "ie", "oo", "ou",},
        {"b", "ck", "d", "g", "k", "m", "n", "p", "t", "v", "x", "z"},
    });


    QStringList newNameParts;
    std::uniform_int_distribution<> groupDis(0, namePartGroups.count() - 1);
    for (int i=0; i<5; i++) {
        const QStringList &nameParts = namePartGroups[groupDis(m_randGen)];
        std::sample(nameParts.begin(), nameParts.end(), std::back_inserter(newNameParts), 1, m_randGen);
    }

    QString newName = newNameParts.join("");
    newName[0] = newName[0].toUpper();
    m_nameEdit->setText(newName);
}

void Dialog_charSheet::onStatsChanged()
{
    int total = 0;
    for (QLineEdit *statEdit : m_statsEdits.values()) {
        total += statEdit->text().toInt();
    }

    m_totalStatCount->setText(QString::number(total));

    QString bgColor;
    if (total > 63 + 18) {
        bgColor = "red";
    } else if (total > 4 * 18) {
        bgColor = "yellow";
    } else if (total <= 63 - 18) {
        bgColor = "gray";
    } else if (total < 3 * 18) {
        bgColor = "silver";
    } else {
        bgColor = "white";
    }

    m_totalStatCount->setStyleSheet("QLineEdit { background-color: " + bgColor + "; color: black }");
}

void Dialog_charSheet::onClassOrRaceChanged()
{
    QMap<QString, int> selectedTraits;
    if (m_raceButtons->checkedButton()) {
        const QString traitsProperty = m_raceButtons->checkedButton()->property("Traits").toString();
        for (const QString &trait : traitsProperty.split(',')) {
            selectedTraits[trait]++;
        }
//        selectedTraits.append(traitsProperty.split(','));
    }
    if (m_classButtons->checkedButton()) {
        const QString traitsProperty = m_classButtons->checkedButton()->property("Traits").toString();
//        selectedTraits.append(traitsProperty.split(','));
        for (const QString &trait : traitsProperty.split(',')) {
            selectedTraits[trait]++;
        }
    }


    for (const QString &trait : m_statsEdits.keys()) {
        if (selectedTraits[trait] > 1) {
            m_statsEdits[trait]->setStyleSheet("QLineEdit { background-color: lightgreen; color: white }");
        } else if (selectedTraits[trait] > 0) {
            m_statsEdits[trait]->setStyleSheet("QLineEdit { background-color: green; color: white }");
        } else {
            m_statsEdits[trait]->setStyleSheet("");
        }
    }
}

void Dialog_charSheet::doRoll()
{
    std::uniform_int_distribution<> die(1, 6);
    QMap<QString, int> rolls;
    for (const QString &stat : m_statsEdits.keys()) {
        int roll = die(m_randGen) + die(m_randGen) + die(m_randGen);
        m_statsEdits[stat]->setText(QString::number(roll));
        rolls[stat] = roll;
    }

    m_rollHistory.push(rolls);
    m_unrollButton->setEnabled(true);

    onStatsChanged();
}

void Dialog_charSheet::doUnroll()
{
    if (m_rollHistory.isEmpty()) {
        m_unrollButton->setEnabled(false);
        return;
    }

    m_rollHistory.pop(); // current that just was pushed

    QMap<QString, int> rolls = m_rollHistory.pop();
    for (const QString &stat : m_statsEdits.keys()) {
        m_statsEdits[stat]->setText(QString::number(rolls[stat]));
    }

    m_unrollButton->setEnabled(!m_rollHistory.isEmpty());

    m_rollHistory.push(rolls);

    onStatsChanged();
}

QButtonGroup *Dialog_charSheet::createGroup(const QString &name, const QStringList &elements, QLayout *layout)
{
    QButtonGroup *group = new QButtonGroup;

    QGroupBox *groupBox = new QGroupBox(name);
    groupBox->setLayout(new QVBoxLayout);

    std::uniform_int_distribution<> dist(0, elements.count() - 1);
    const QString selected = elements[dist(m_randGen)];

    for (const QString &text : elements) {
        const QStringList parts = text.split('|');

        const QString title = parts.first();
        const QString traits = parts.last();
        QRadioButton *button = new QRadioButton(title);
        if (text == selected) {
            button->setChecked(true);
        }
        button->setProperty("Traits", traits);

        group->addButton(button);
        groupBox->layout()->addWidget(button);

        connect(button, &QRadioButton::clicked, this, &Dialog_charSheet::onClassOrRaceChanged);
    }


    layout->addWidget(groupBox);

    return group;
}
