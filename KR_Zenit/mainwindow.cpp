#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QSqlError>
#include <QStyle>
#include <QDesktopWidget>

QSqlDatabase MainWindow::dbconn = QSqlDatabase::addDatabase("QPSQL"); // инициализация статического поля dbconn

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    // пароли
    admin_pass = "admin";
    user_pass = "user";
    // центрирование
    this->setGeometry(QStyle::alignedRect(Qt::LeftToRight,Qt::AlignCenter,this->size(),qApp->desktop()->availableGeometry()));
    // связи
    connect(ui->btn_Enter,SIGNAL(clicked(bool)),this,SLOT(enter()));
}

MainWindow::~MainWindow()
{
    if(dbconn.isOpen())
        dbconn.close();
    delete ui;
}

void MainWindow::enter()
{
    int index = ui->cb_User->currentIndex();
    QString pass = ui->le_Password->text();
    if ((index == 0 && pass == admin_pass) || (index == 1 && pass == user_pass))
    {
        dbconnect();
        if (m == nullptr)
            m = new Menu(); // создаем форму Menu
        connect(m,&Menu::open_mainwindow,this,&MainWindow::show); // возвращение назад
        connect(this,SIGNAL(open_menu()),m,SLOT(this_open())); // открываем меню
        this->close();
        emit open_menu();
    }
    else
        QMessageBox::critical(this,"Error","Неверный пароль!");
}

void MainWindow::dbconnect()
{
    if(dbconn.isOpen())
        dbconn.close();
    dbconn.setDatabaseName("FC_Zenit");
    dbconn.setHostName("localhost");
    if (ui->cb_User->currentIndex() == 0)
    {
        dbconn.setUserName("postgres");
        dbconn.setPassword("Messi2003");
    }
    else
    {
        dbconn.setUserName("my_user");
        dbconn.setPassword("qwerty");
    }
    if(dbconn.open())
        QMessageBox::about(this,"Success","Установлено подключение к БД!");
    else
        QMessageBox::critical(this,"Error",dbconn.lastError().text());

}
