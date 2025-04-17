#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSqlDatabase>
#include "menu.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    static QSqlDatabase dbconn; // объект подключения к БД
    ~MainWindow();

public slots:
    void enter();

signals:
    void open_menu();

private:
    Ui::MainWindow *ui;
    Menu* m = nullptr;
    QString admin_pass;
    QString user_pass;
    void dbconnect(); // подключение к БД
};
#endif // MAINWINDOW_H
