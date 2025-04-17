#ifndef MENU_H
#define MENU_H

#include <QWidget>
#include "coachstaff.h"
#include "players.h"
#include "matches.h"
#include "transfers.h"

namespace Ui {
class Menu;
}

class Menu : public QWidget
{
    Q_OBJECT

public:
    explicit Menu(QWidget *parent = nullptr);
    ~Menu();

public slots:
    void this_open();
    void open_coachstaff();
    void open_players();
    void open_matches();
    void open_transfers();
    void exit();

signals:
    void open_mainwindow();
    void open_coachstaff_signal(QString str = "");
    void open_players_signal(QString str = "");
    void open_matches_signal(QString str = "");
    void open_transfers_signal();

private:
    Ui::Menu *ui;
    CoachStaff* coachstaff;
    Players* players;
    Matches* matches;
    Transfers* transfers;

    PlayersInSquads* playersinsquads;
    MatchesInSquads* matchesinsquads;
    OfferedPlayers* offeredplayers;
    FootballClubs* footballclubs;
};

#endif // MENU_H
