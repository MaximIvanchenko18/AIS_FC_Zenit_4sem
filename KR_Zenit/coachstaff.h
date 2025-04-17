#ifndef COACHSTAFF_H
#define COACHSTAFF_H

#include <QWidget>
#include "playersinsquads.h"

namespace Ui {
class CoachStaff;
}

class CoachStaff : public QWidget
{
    Q_OBJECT

public:
    explicit CoachStaff(PlayersInSquads* value = nullptr, QWidget *parent = nullptr);
    ~CoachStaff();

public slots:
    void this_open(QString);
    void exit();
    void fill_selected();
    void add();
    void del();
    void edit();
    void squad_filter();
    void open_players_in_squads(); // кнопка перехода на новую таблицу
    void save_selection(); // при переходе внутри tabwidget

signals:
    void open_menu();
    void open_players_in_squads_signal(int value = 0);

private:
    Ui::CoachStaff *ui;
    PlayersInSquads* playersinsquad;
    void fill_squads();
    void selectAll();
};

#endif // COACHSTAFF_H
