#ifndef FOOTBALLCLUBS_H
#define FOOTBALLCLUBS_H

#include <QWidget>

namespace Ui {
class FootballClubs;
}

class FootballClubs : public QWidget
{
    Q_OBJECT

public:
    explicit FootballClubs(QWidget *parent = nullptr);
    ~FootballClubs();

public slots:
    void this_open();
    void exit();
    void fill_selected();
    void add();
    void del();
    void edit();
    void filter();
    void save_selection(); // при переходе внутри tabwidget

signals:
    void go_back(QString str = "");

private:
    Ui::FootballClubs *ui;
    void selectAll();
};

#endif // FOOTBALLCLUBS_H
