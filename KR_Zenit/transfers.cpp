#include "transfers.h"
#include "ui_transfers.h"
#include "mainwindow.h"
#include <QStyle>
#include <QDesktopWidget>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlField>
#include <QSqlRecord>
#include <QMessageBox>

Transfers::Transfers(OfferedPlayers* value, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Transfers)
{
    ui->setupUi(this);

    // центрирование
    this->setGeometry(QStyle::alignedRect(Qt::LeftToRight,Qt::AlignCenter,this->size(),qApp->desktop()->availableGeometry()));

    // связи
    connect(ui->btn_Exit,SIGNAL(clicked(bool)),this,SLOT(exit()));
    connect(ui->tw_Transfers,SIGNAL(itemSelectionChanged()),this,SLOT(fill_selected()));
    connect(ui->btn_Add,SIGNAL(clicked(bool)),this,SLOT(add()));
    connect(ui->btn_Delete,SIGNAL(clicked(bool)),this,SLOT(del()));
    connect(ui->btn_Edit,SIGNAL(clicked(bool)),this,SLOT(edit()));
    connect(ui->le_Surname_Filter,SIGNAL(textEdited(QString)),this,SLOT(filter()));
    connect(ui->tabWidget,SIGNAL(currentChanged(int)),this,SLOT(save_selection()));
    connect(ui->btn_AllOnMarket,SIGNAL(clicked(bool)),this,SLOT(open_offered_players()));
    connect(ui->btn_PlayerOnMarket,SIGNAL(clicked(bool)),this,SLOT(open_one_offered_player()));

    // Параметры таблицы
    ui->tw_Transfers->setColumnCount(4); // число столбцов
    ui->tw_Transfers->setAutoScroll(true); // скрол
    // Выделение только одной строки
    ui->tw_Transfers->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tw_Transfers->setSelectionBehavior(QAbstractItemView::SelectRows);
    // Заголовки
    ui->tw_Transfers->setHorizontalHeaderItem(0,new QTableWidgetItem("Фамилия"));
    ui->tw_Transfers->setHorizontalHeaderItem(1,new QTableWidgetItem("Дата"));
    ui->tw_Transfers->setHorizontalHeaderItem(2,new QTableWidgetItem("Стоимость (млн.евро)"));
    ui->tw_Transfers->setHorizontalHeaderItem(3,new QTableWidgetItem("Условия"));
    // Растягиваем последний столбец при растягивании формы
    ui->tw_Transfers->horizontalHeader()->setStretchLastSection(true);
    // Запрет на изменение
    ui->tw_Transfers->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // поля
    offeredplayers = value;
    connect(this,SIGNAL(open_offered_players_signal(QString)),offeredplayers,SLOT(this_open(QString)));
    connect(offeredplayers,SIGNAL(go_back()),this,SLOT(this_open()));
}

Transfers::~Transfers()
{
    delete ui;
}

void Transfers::exit()
{
    this->close();
    emit open_menu();
}

void Transfers::this_open()
{
    // Заполнение игроков
    fill_players();
    // Очистка фильтра
    ui->le_Surname_Filter->clear();
    // Заполнение таблицы
    selectAll();
    this->show();
}

void Transfers::fill_players()
{
    QSqlQuery query(MainWindow::dbconn); // объект запроса с привязкой к соединению
    QString sqlstr = "select * from offered_players"; // строка запроса
    if(!query.exec(sqlstr))
    {
        QMessageBox::critical(this,"Error",query.lastError().text());
        return;
    }

    ui->cb_Player->clear();
    // заполняем список
    int i = 0;
    ui->cb_Player->addItem(" "); // первый пустой
    while (query.next())
    {
        ui->cb_Player->addItem(query.value("surname").toString());
        i++;
    }
}

void Transfers::selectAll()
{
    ui->tw_Transfers->clearContents();
    // Сортировка выкл
    ui->tw_Transfers->setSortingEnabled(false);

    if(!MainWindow::dbconn.isOpen())
    {
        QMessageBox::critical(this,"Error",MainWindow::dbconn.lastError().text());
        return;
    }

    QSqlQuery query(MainWindow::dbconn); // объект запроса с привязкой к соединению
    QString sqlstr = "select pl.surname,transfer_date,transfer_cost,terms from transfers join offered_players as pl on(pl.player_id=transfers.player_id)"; // строка запроса
    if(!query.exec(sqlstr))
    {
        QMessageBox::critical(this,"Error",query.lastError().text());
        return;
    }

    if(query.isActive())
        ui->tw_Transfers->setRowCount(query.size());
    else
        ui->tw_Transfers->setRowCount(0);

    // Запись данных в таблицу
    int i = 0;
    while (query.next())
    {
        ui->tw_Transfers->setItem(i,0,new QTableWidgetItem(query.value("surname").toString()));
        ui->tw_Transfers->setItem(i,1,new QTableWidgetItem(query.value("transfer_date").toString()));
        ui->tw_Transfers->setItem(i,2,new QTableWidgetItem(query.value("transfer_cost").toString()));
        ui->tw_Transfers->setItem(i,3,new QTableWidgetItem(query.value("terms").toString()));
        i++;
    }
    // Размер столбцов
    ui->tw_Transfers->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->tw_Transfers->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui->tw_Transfers->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    ui->tw_Transfers->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    // Сортировка вкл
    ui->tw_Transfers->setSortingEnabled(true);
    // Очистить поля ввода
    ui->cb_Player->setCurrentIndex(0);
    ui->de_Date->setDate(QDate::currentDate());
    ui->le_Money->clear();
    ui->te_Terms->clear();
}

void Transfers::fill_selected()
{
    int cur_row = ui->tw_Transfers->currentRow(); // номер выбранной строки
    if (cur_row < 0) return;
    ui->cb_Player->setCurrentText(ui->tw_Transfers->item(cur_row,0)->text());
    ui->de_Date->setDate(QDate::fromString(ui->tw_Transfers->item(cur_row,1)->text(), "yyyy-MM-dd"));
    ui->le_Money->setText(ui->tw_Transfers->item(cur_row,2)->text());
    ui->te_Terms->setText(ui->tw_Transfers->item(cur_row,3)->text());
}

void Transfers::add()
{
    if(!MainWindow::dbconn.isOpen())
    {
        QMessageBox::critical(this,"Error",MainWindow::dbconn.lastError().text());
        return;
    }

    QSqlQuery query(MainWindow::dbconn); // объект запроса с привязкой к соединению
    QString sqlstr = "insert into transfers(player_id,transfer_date,transfer_cost,terms) values(?,?,?,?)"; // строка запроса
    query.prepare(sqlstr); // подготовка запроса

    // передаем параметры в запрос
    if (ui->cb_Player->currentIndex() == 0)
    {
        QMessageBox::critical(this,"Error","Opponent field is wrong!");
        return;
    }
    QSqlQuery query_2(MainWindow::dbconn);
    QString sqlstr_2 = "select * from offered_players where surname='" + ui->cb_Player->currentText() + "'";
    query_2.exec(sqlstr_2);
    query_2.next();
    query.bindValue(0,query_2.value("player_id").toInt());

    query.bindValue(1,ui->de_Date->text());

    bool ok;
    double val = ui->le_Money->text().toDouble(&ok);
    if (ok)
        query.bindValue(2,val);
    else
    {
        QMessageBox::critical(this,"Error","Cost field is wrong!");
        return;
    }

    query.bindValue(3,ui->te_Terms->toPlainText() != "" ? ui->te_Terms->toPlainText() : MainWindow::dbconn.record("transfers").field("terms").defaultValue());

    if(!query.exec())
    {
        QMessageBox::critical(this,"Error",query.lastError().text());
        return;
    }
    QMessageBox::about(this,"Success",QString("INSERT 0 %1").arg(query.numRowsAffected()));

    selectAll(); // обновляем таблицу
}

void Transfers::del()
{
    if(!MainWindow::dbconn.isOpen())
    {
        QMessageBox::critical(this,"Error",MainWindow::dbconn.lastError().text());
        return;
    }

    int cur_row = ui->tw_Transfers->currentRow(); // номер выбранной строки
    if(cur_row < 0) // не выбрана
    {
        QMessageBox::critical(this,"Error","Row is not selected!");
        return;
    }

    // Подтверждение удаления
    if(QMessageBox::question(this,"Delete","Delete selected row?",QMessageBox::Cancel,QMessageBox::Ok)==QMessageBox::Cancel)
        return;

    QSqlQuery query(MainWindow::dbconn);
    QString sqlstr = "delete from transfers where transfer_date='" + ui->tw_Transfers->item(cur_row,1)->text() + "'";

    if(!query.exec(sqlstr))
    {
        QMessageBox::critical(this,"Error",query.lastError().text());
        return;
    }
    QMessageBox::about(this,"Success",QString("DELETE %1").arg(query.numRowsAffected()));

    selectAll(); // обновляем таблицу
}

void Transfers::edit()
{
    if(!MainWindow::dbconn.isOpen())
    {
        QMessageBox::critical(this,"Error",MainWindow::dbconn.lastError().text());
        return;
    }

    int cur_row = ui->tw_Transfers->currentRow(); // номер выбранной строки
    if(cur_row < 0) // не выбрана
    {
        QMessageBox::critical(this,"Error","Row is not selected!");
        return;
    }

    QSqlQuery query(MainWindow::dbconn);
    QString sqlstr = "update transfers set (player_id,transfer_date,transfer_cost,terms) = (?,?,?,?) where transfer_date='"
            + ui->tw_Transfers->item(cur_row,1)->text() + "'";
    query.prepare(sqlstr);

    // передаем параметры в запрос
    if (ui->cb_Player->currentIndex() == 0)
    {
        QMessageBox::critical(this,"Error","Opponent field is wrong!");
        return;
    }
    QSqlQuery query_2(MainWindow::dbconn);
    QString sqlstr_2 = "select * from offered_players where surname='" + ui->cb_Player->currentText() + "'";
    query_2.exec(sqlstr_2);
    query_2.next();
    query.bindValue(0,query_2.value("player_id").toInt());

    query.bindValue(1,ui->de_Date->text());

    bool ok;
    double val = ui->le_Money->text().toDouble(&ok);
    if (ok)
        query.bindValue(2,val);
    else
    {
        QMessageBox::critical(this,"Error","Cost field is wrong!");
        return;
    }

    query.bindValue(3,ui->te_Terms->toPlainText() != "" ? ui->te_Terms->toPlainText() : MainWindow::dbconn.record("transfers").field("terms").defaultValue());

    if(!query.exec())
    {
        QMessageBox::critical(this,"Error",query.lastError().text());
        return;
    }
    QMessageBox::about(this,"Success",QString("UPDATE %1").arg(query.numRowsAffected()));

    selectAll(); // обновляем таблицу
}

void Transfers::filter()
{
    if (ui->le_Surname_Filter->text() == "")
    {
        selectAll();
        return;
    }
    ui->tw_Transfers->clearContents();
    // Сортировка выкл
    ui->tw_Transfers->setSortingEnabled(false);

    QSqlQuery query(MainWindow::dbconn); // объект запроса с привязкой к соединению
    QString sqlstr = "select pl.surname,transfer_date,transfer_cost,terms from transfers join offered_players as pl on(pl.player_id=transfers.player_id) where pl.surname like '"
            + ui->le_Surname_Filter->text() + "%'"; // строка запроса

    if(!query.exec(sqlstr))
    {
        QMessageBox::critical(this,"Error",query.lastError().text());
        return;
    }

    if(query.isActive())
        ui->tw_Transfers->setRowCount(query.size());
    else
        ui->tw_Transfers->setRowCount(0);

    // Запись данных в таблицу
    int i = 0;
    while (query.next())
    {
        ui->tw_Transfers->setItem(i,0,new QTableWidgetItem(query.value("surname").toString()));
        ui->tw_Transfers->setItem(i,1,new QTableWidgetItem(query.value("transfer_date").toString()));
        ui->tw_Transfers->setItem(i,2,new QTableWidgetItem(query.value("transfer_cost").toString()));
        ui->tw_Transfers->setItem(i,3,new QTableWidgetItem(query.value("terms").toString()));
        i++;
    }
    // Размер столбцов
    ui->tw_Transfers->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->tw_Transfers->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui->tw_Transfers->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    ui->tw_Transfers->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    // Сортировка вкл
    ui->tw_Transfers->setSortingEnabled(true);
}

void Transfers::save_selection()
{
    ui->tw_Transfers->setCurrentCell(ui->tw_Transfers->currentRow(),0);
}

void Transfers::open_offered_players()
{
    this->close();
    emit open_offered_players_signal();
}

void Transfers::open_one_offered_player()
{
    int cur_row = ui->tw_Transfers->currentRow(); // номер выбранной строки
    if(cur_row < 0) // не выбрана
    {
        QMessageBox::critical(this,"Error","Row is not selected!");
        return;
    }

    this->close();
    emit open_offered_players_signal(ui->tw_Transfers->item(cur_row,0)->text());
}
