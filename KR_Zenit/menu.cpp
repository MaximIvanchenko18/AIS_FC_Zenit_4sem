#include "menu.h"
#include "ui_menu.h"
#include "mainwindow.h"
#include <QStyle>
#include <QDesktopWidget>

Menu::Menu(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Menu)
{
    ui->setupUi(this);

    // центрирование
    this->setGeometry(QStyle::alignedRect(Qt::LeftToRight,Qt::AlignCenter,this->size(),qApp->desktop()->availableGeometry()));

    // Поля
    matchesinsquads = new MatchesInSquads();
    playersinsquads = new PlayersInSquads(matchesinsquads);
    footballclubs = new FootballClubs();
    offeredplayers = new OfferedPlayers(footballclubs);
    coachstaff = new CoachStaff(playersinsquads);
    players = new Players(playersinsquads);
    matches = new Matches(matchesinsquads);
    transfers = new Transfers(offeredplayers);

    // связи для полей
    // coachstaff
    connect(coachstaff,&CoachStaff::open_menu,this,&Menu::show); // возвращение в меню
    connect(this,SIGNAL(open_coachstaff_signal(QString)),coachstaff,SLOT(this_open(QString))); // для открытия формы coachstaff

    // players
    connect(players,&Players::open_menu,this,&Menu::show); // возвращение в меню
    connect(this,SIGNAL(open_players_signal(QString)),players,SLOT(this_open(QString))); // для открытия формы players

    // matches
    connect(matches,&Matches::open_menu,this,&Menu::show); // возвращение назад
    connect(this,SIGNAL(open_matches_signal(QString)),matches,SLOT(this_open(QString))); // для открытия формы

    // transfers
    connect(transfers,&Transfers::open_menu,this,&Menu::show); // возвращение назад
    connect(this,&Menu::open_transfers_signal,transfers,&Transfers::this_open); // для открытия формы

    // playersinsquads
    connect(playersinsquads,SIGNAL(open_all_players_signal(QString)),players,SLOT(this_open(QString)));
    connect(playersinsquads,SIGNAL(open_coach_staff_signal(QString)),coachstaff,SLOT(this_open(QString)));

    // matchesinsquads
    connect(matchesinsquads,SIGNAL(open_players_in_squads_signal(int)),playersinsquads,SLOT(this_open(int)));
    connect(matchesinsquads,SIGNAL(open_coach_staff_signal(QString)),coachstaff,SLOT(this_open(QString)));
    connect(matchesinsquads,SIGNAL(open_matches_signal(QString)),matches,SLOT(this_open(QString)));

    // слоты меню
    connect(ui->btn_CoachStaff,SIGNAL(clicked(bool)),this,SLOT(open_coachstaff()));
    connect(ui->btn_Players,SIGNAL(clicked(bool)),this,SLOT(open_players()));
    connect(ui->btn_Matches,SIGNAL(clicked(bool)),this,SLOT(open_matches()));
    connect(ui->btn_Transfers,SIGNAL(clicked(bool)),this,SLOT(open_transfers()));
    connect(ui->btn_Exit,SIGNAL(clicked(bool)),this,SLOT(exit()));
}

Menu::~Menu()
{
    delete ui;
}

void Menu::this_open()
{
    // если пользователь, то скрываем трансферы
    if (MainWindow::dbconn.userName() == "my_user")
    {
        ui->btn_Transfers->setVisible(false);
        ui->lb_Transfers->setVisible(false);
    }
    else
    {
        ui->btn_Transfers->setVisible(true);
        ui->lb_Transfers->setVisible(true);
    }
    this->show();
}

void Menu::open_coachstaff()
{
    this->close();
    emit open_coachstaff_signal();
}

void Menu::open_players()
{
    this->close();
    emit open_players_signal();
}

void Menu::open_matches()
{
    this->close();
    emit open_matches_signal();
}

void Menu::open_transfers()
{
    this->close();
    emit open_transfers_signal();
}

void Menu::exit()
{
    this->close();
    emit open_mainwindow();
}
