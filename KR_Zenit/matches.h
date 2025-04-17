#ifndef MATCHES_H
#define MATCHES_H

#include <QWidget>
#include "matchesinsquads.h"

namespace Ui {
class Matches;
}

class Matches : public QWidget
{
    Q_OBJECT

public:
    explicit Matches(MatchesInSquads* value = nullptr, QWidget *parent = nullptr);
    ~Matches();

public slots:
    void this_open(QString);
    void exit();
    void fill_selected();
    void add();
    void del();
    void edit();
    void filter();
    void save_selection(); // при переходе внутри tabwidget
    void open_matches_in_squads();
    void cancel_filter();

signals:
    void open_menu();
    void open_matches_in_squads_signal(int value = 0);

private:
    Ui::Matches *ui;
    MatchesInSquads* matchesinsquads;
    void fill_squads();
    void fill_opponents();
    void selectAll();
};

#endif // MATCHES_H
