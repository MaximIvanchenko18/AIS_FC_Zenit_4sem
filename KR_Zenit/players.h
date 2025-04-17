#ifndef PLAYERS_H
#define PLAYERS_H

#include <QWidget>
#include "playersinsquads.h"

namespace Ui {
class Players;
}

class Players : public QWidget
{
    Q_OBJECT

public:
    explicit Players(PlayersInSquads* value = nullptr, QWidget *parent = nullptr);
    ~Players();

public slots:
    void this_open(QString);
    void exit();
    void fill_selected();
    void add();
    void del();
    void edit();
    void filter();
    void open_players_in_squads(); // открытие новой формы
    void save_selection(); // при переходе внутри tabwidget

signals:
    void open_menu();
    void open_players_in_squads_signal(int value = 0);

private:
    Ui::Players *ui;
    PlayersInSquads* playersinsquad;
    void fill_squads();
    void selectAll();
};

#endif // PLAYERS_H
