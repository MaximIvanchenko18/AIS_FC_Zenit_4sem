#ifndef PLAYERSINSQUADS_H
#define PLAYERSINSQUADS_H

#include <QWidget>
#include "matchesinsquads.h"

namespace Ui {
class PlayersInSquads;
}

class PlayersInSquads : public QWidget
{
    Q_OBJECT

public:
    explicit PlayersInSquads(MatchesInSquads* value = nullptr, QWidget *parent = nullptr);
    ~PlayersInSquads();
    void setButtonToHide(bool); // false - прячем кнопку "все игроки", true - прячем кнопку "тренеры состава"
    void setOnlyCurrentPlayers(bool); // установка checkbox

public slots:
    void this_open(int);
    void exit();
    void change_tables();

    void open_coach_staff();
    void open_all_players();
    void open_all_players_change();
    void open_matches_in_squads();

    void add_squad();
    void delete_squad();

signals:
    void open_all_players_signal(QString str = "");
    void open_coach_staff_signal(QString str = "");
    void open_matches_in_squads_signal(int value = 0);

private:
    Ui::PlayersInSquads *ui;
    MatchesInSquads* matchesinsquads;
    int squad_num; // номер выбранного состава
    void fill_squad_table();
    void fill_players_table();
};

#endif // PLAYERSINSQUADS_H
