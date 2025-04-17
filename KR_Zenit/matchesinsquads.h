#ifndef MATCHESINSQUADS_H
#define MATCHESINSQUADS_H

#include <QWidget>

namespace Ui {
class MatchesInSquads;
}

class MatchesInSquads : public QWidget
{
    Q_OBJECT

public:
    explicit MatchesInSquads(QWidget *parent = nullptr);
    ~MatchesInSquads();
    void setButtonToHide(bool); // false - прячем кнопку "все матчи", true - прячем кнопку "игроки состава"

public slots:
    void this_open(int);
    void change_tables();
    void exit();

    void open_players_in_squads();
    void open_matches();
    void open_coach_staff();
    void open_matches_change();

    void add_squad();
    void delete_squad();

signals:
    void open_coach_staff_signal(QString str = "");
    void open_players_in_squads_signal(int value = 0);
    void open_matches_signal(QString str = "");

private:
    Ui::MatchesInSquads *ui;
    int squad_num; // номер выбранного состава
    void fill_squad_table();
    void fill_matches_table();
};

#endif // MATCHESINSQUADS_H
