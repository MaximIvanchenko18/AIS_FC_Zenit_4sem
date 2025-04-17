#ifndef TRANSFERS_H
#define TRANSFERS_H

#include <QWidget>
#include "offeredplayers.h"

namespace Ui {
class Transfers;
}

class Transfers : public QWidget
{
    Q_OBJECT

public:
    explicit Transfers(OfferedPlayers* value = nullptr, QWidget *parent = nullptr);
    ~Transfers();

public slots:
    void this_open();
    void exit();
    void fill_selected();
    void add();
    void del();
    void edit();
    void filter();
    void save_selection(); // при переходе внутри tabwidget

    void open_offered_players();
    void open_one_offered_player();

signals:
    void open_menu();
    void open_offered_players_signal(QString str = "");

private:
    Ui::Transfers *ui;
    OfferedPlayers* offeredplayers;
    void fill_players();
    void selectAll();
};

#endif // TRANSFERS_H
