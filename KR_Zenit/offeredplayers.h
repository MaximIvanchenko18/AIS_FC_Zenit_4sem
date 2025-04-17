#ifndef OFFEREDPLAYERS_H
#define OFFEREDPLAYERS_H

#include <QWidget>
#include "footballclubs.h"

namespace Ui {
class OfferedPlayers;
}

class OfferedPlayers : public QWidget
{
    Q_OBJECT

public:
    explicit OfferedPlayers(FootballClubs* value = nullptr, QWidget *parent = nullptr);
    ~OfferedPlayers();

public slots:
    void this_open(QString);
    void exit();
    void fill_selected();
    void add();
    void del();
    void edit();
    void filter();
    void save_selection(); // при переходе внутри tabwidget

    void open_fc();

signals:
    void go_back();
    void open_fc_signal();

private:
    Ui::OfferedPlayers *ui;
    FootballClubs* footballclubs;
    void fill_fc();
    void selectAll();
};

#endif // OFFEREDPLAYERS_H
