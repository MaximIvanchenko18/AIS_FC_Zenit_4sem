#include "offeredplayers.h"
#include "ui_offeredplayers.h"
#include "mainwindow.h"
#include <QStyle>
#include <QDesktopWidget>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlField>
#include <QSqlRecord>
#include <QMessageBox>

OfferedPlayers::OfferedPlayers(FootballClubs* value, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::OfferedPlayers)
{
    ui->setupUi(this);

    // центрирование
    this->setGeometry(QStyle::alignedRect(Qt::LeftToRight,Qt::AlignCenter,this->size(),qApp->desktop()->availableGeometry()));

    // связи
    connect(ui->btn_Exit,SIGNAL(clicked(bool)),this,SLOT(exit()));
    connect(ui->tw_OfferedPlayers,SIGNAL(itemSelectionChanged()),this,SLOT(fill_selected()));
    connect(ui->btn_Add,SIGNAL(clicked(bool)),this,SLOT(add()));
    connect(ui->btn_Delete,SIGNAL(clicked(bool)),this,SLOT(del()));
    connect(ui->btn_Edit,SIGNAL(clicked(bool)),this,SLOT(edit()));
    connect(ui->le_Surname_Filter,SIGNAL(textEdited(QString)),this,SLOT(filter()));
    connect(ui->cb_Position_Filter,SIGNAL(currentIndexChanged(int)),this,SLOT(filter()));
    connect(ui->tabWidget,SIGNAL(currentChanged(int)),this,SLOT(save_selection()));
    connect(ui->btn_FC,SIGNAL(clicked(bool)),this,SLOT(open_fc()));

    // Параметры таблицы
    ui->tw_OfferedPlayers->setColumnCount(4); // число столбцов
    ui->tw_OfferedPlayers->setAutoScroll(true); // скрол
    // Выделение только одной строки
    ui->tw_OfferedPlayers->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tw_OfferedPlayers->setSelectionBehavior(QAbstractItemView::SelectRows);
    // Заголовки
    ui->tw_OfferedPlayers->setHorizontalHeaderItem(0,new QTableWidgetItem("Фамилия"));
    ui->tw_OfferedPlayers->setHorizontalHeaderItem(1,new QTableWidgetItem("Возраст"));
    ui->tw_OfferedPlayers->setHorizontalHeaderItem(2,new QTableWidgetItem("Позиция"));
    ui->tw_OfferedPlayers->setHorizontalHeaderItem(3,new QTableWidgetItem("Футбольный клуб"));
    // Растягиваем последний столбец при растягивании формы
    ui->tw_OfferedPlayers->horizontalHeader()->setStretchLastSection(true);
    // Запрет на изменение
    ui->tw_OfferedPlayers->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // Параметры полей
    ui->le_Age->setValidator(new QIntValidator(0,99,this));

    footballclubs = value;
    connect(this,SIGNAL(open_fc_signal()),footballclubs,SLOT(this_open()));
    connect(footballclubs,SIGNAL(go_back(QString)),this,SLOT(this_open(QString)));
}

OfferedPlayers::~OfferedPlayers()
{
    delete ui;
}

void OfferedPlayers::exit()
{
    this->close();
    emit go_back();
}

void OfferedPlayers::this_open(QString str)
{
    // Заполнение фк
    fill_fc();
    // Очистка фильтра
    ui->le_Surname_Filter->clear();
    ui->cb_Position_Filter->setCurrentIndex(0);
    // Заполнение таблицы
    selectAll();
    if (str != "")
    {
        ui->le_Surname_Filter->setText(str);
        filter();
        ui->tw_OfferedPlayers->setCurrentCell(0,0);
    }
    this->show();
}

void OfferedPlayers::fill_fc()
{
    QSqlQuery query(MainWindow::dbconn); // объект запроса с привязкой к соединению
    QString sqlstr = "select * from football_clubs"; // строка запроса
    if(!query.exec(sqlstr))
    {
        QMessageBox::critical(this,"Error",query.lastError().text());
        return;
    }

    ui->cb_FC->clear();
    // заполняем список
    int i = 0;
    ui->cb_FC->addItem(" "); // первый пустой
    while (query.next())
    {
        ui->cb_FC->addItem(query.value("fc_name").toString());
        i++;
    }
}

void OfferedPlayers::selectAll()
{
    ui->tw_OfferedPlayers->clearContents();
    // Сортировка выкл
    ui->tw_OfferedPlayers->setSortingEnabled(false);

    if(!MainWindow::dbconn.isOpen())
    {
        QMessageBox::critical(this,"Error",MainWindow::dbconn.lastError().text());
        return;
    }

    QSqlQuery query(MainWindow::dbconn); // объект запроса с привязкой к соединению
    QString sqlstr = "select surname,age,playing_position,fc.fc_name from offered_players join football_clubs as fc on(fc.fc_id=offered_players.fc_id)"; // строка запроса
    if(!query.exec(sqlstr))
    {
        QMessageBox::critical(this,"Error",query.lastError().text());
        return;
    }

    if(query.isActive())
        ui->tw_OfferedPlayers->setRowCount(query.size());
    else
        ui->tw_OfferedPlayers->setRowCount(0);

    // Запись данных в таблицу
    int i = 0;
    while (query.next())
    {
        ui->tw_OfferedPlayers->setItem(i,0,new QTableWidgetItem(query.value("surname").toString()));
        ui->tw_OfferedPlayers->setItem(i,1,new QTableWidgetItem(query.value("age").toString()));
        ui->tw_OfferedPlayers->setItem(i,2,new QTableWidgetItem(query.value("playing_position").toString()));
        ui->tw_OfferedPlayers->setItem(i,3,new QTableWidgetItem(query.value("fc_name").toString()));
        i++;
    }
    // Размер столбцов
    ui->tw_OfferedPlayers->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->tw_OfferedPlayers->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui->tw_OfferedPlayers->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    ui->tw_OfferedPlayers->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    // Сортировка вкл
    ui->tw_OfferedPlayers->setSortingEnabled(true);
    // Очистить поля ввода
    ui->le_Surname->clear();
    ui->le_Age->clear();
    ui->cb_Position->setCurrentIndex(0);
    ui->cb_FC->setCurrentIndex(0);
}

void OfferedPlayers::fill_selected()
{
    int cur_row = ui->tw_OfferedPlayers->currentRow(); // номер выбранной строки
    if (cur_row < 0) return;
    ui->le_Surname->setText(ui->tw_OfferedPlayers->item(cur_row,0)->text());
    ui->le_Age->setText(ui->tw_OfferedPlayers->item(cur_row,1)->text());
    ui->cb_Position->setCurrentText(ui->tw_OfferedPlayers->item(cur_row,2)->text());
    ui->cb_FC->setCurrentText(ui->tw_OfferedPlayers->item(cur_row,3)->text());
}

void OfferedPlayers::add()
{
    if(!MainWindow::dbconn.isOpen())
    {
        QMessageBox::critical(this,"Error",MainWindow::dbconn.lastError().text());
        return;
    }

    QSqlQuery query(MainWindow::dbconn); // объект запроса с привязкой к соединению
    QString sqlstr = "insert into offered_players(surname,age,playing_position,fc_id) values(?,?,?,?)"; // строка запроса
    query.prepare(sqlstr); // подготовка запроса

    // передаем параметры в запрос
    if (ui->le_Surname->text() == "" || ui->le_Age->text() == "")
    {
        QMessageBox::critical(this,"Error","Surname or age field is wrong!");
        return;
    }
    query.bindValue(0,ui->le_Surname->text());
    query.bindValue(1,ui->le_Age->text().toInt());

    if (ui->cb_Position->currentIndex() == 0)
    {
        QMessageBox::critical(this,"Error","Position field is wrong!");
        return;
    }
    query.bindValue(2,ui->cb_Position->currentText());

    if (ui->cb_FC->currentIndex() == 0)
    {
        QMessageBox::critical(this,"Error","FC field is wrong!");
        return;
    }
    QSqlQuery query_2(MainWindow::dbconn);
    QString sqlstr_2 = "select * from football_clubs where fc_name='" + ui->cb_FC->currentText() + "'";
    query_2.exec(sqlstr_2);
    query_2.next();
    query.bindValue(3,query_2.value("fc_id").toInt());

    if(!query.exec())
    {
        QMessageBox::critical(this,"Error",query.lastError().text());
        return;
    }
    QMessageBox::about(this,"Success",QString("INSERT 0 %1").arg(query.numRowsAffected()));

    selectAll(); // обновляем таблицу
}

void OfferedPlayers::del()
{
    if(!MainWindow::dbconn.isOpen())
    {
        QMessageBox::critical(this,"Error",MainWindow::dbconn.lastError().text());
        return;
    }

    int cur_row = ui->tw_OfferedPlayers->currentRow(); // номер выбранной строки
    if(cur_row < 0) // не выбрана
    {
        QMessageBox::critical(this,"Error","Row is not selected!");
        return;
    }

    // Подтверждение удаления
    if(QMessageBox::question(this,"Delete","Delete selected row?",QMessageBox::Cancel,QMessageBox::Ok)==QMessageBox::Cancel)
        return;

    QSqlQuery query(MainWindow::dbconn);
    QString sqlstr = "delete from offered_players where surname='" + ui->tw_OfferedPlayers->item(cur_row,0)->text() + "' and age="
            + ui->tw_OfferedPlayers->item(cur_row,1)->text();

    if(!query.exec(sqlstr))
    {
        QMessageBox::critical(this,"Error",query.lastError().text());
        return;
    }
    QMessageBox::about(this,"Success",QString("DELETE %1").arg(query.numRowsAffected()));

    selectAll(); // обновляем таблицу
}

void OfferedPlayers::edit()
{
    if(!MainWindow::dbconn.isOpen())
    {
        QMessageBox::critical(this,"Error",MainWindow::dbconn.lastError().text());
        return;
    }

    int cur_row = ui->tw_OfferedPlayers->currentRow(); // номер выбранной строки
    if(cur_row < 0) // не выбрана
    {
        QMessageBox::critical(this,"Error","Row is not selected!");
        return;
    }

    QSqlQuery query(MainWindow::dbconn); // объект запроса с привязкой к соединению
    QString sqlstr = "update offered_players set (surname,age,playing_position,fc_id) = (?,?,?,?) where surname='" + ui->tw_OfferedPlayers->item(cur_row,0)->text() + "' and age="
            + ui->tw_OfferedPlayers->item(cur_row,1)->text(); // строка запроса
    query.prepare(sqlstr); // подготовка запроса

    // передаем параметры в запрос
    if (ui->le_Surname->text() == "" || ui->le_Age->text() == "")
    {
        QMessageBox::critical(this,"Error","Surname or age field is wrong!");
        return;
    }
    query.bindValue(0,ui->le_Surname->text());
    query.bindValue(1,ui->le_Age->text().toInt());

    if (ui->cb_Position->currentIndex() == 0)
    {
        QMessageBox::critical(this,"Error","Position field is wrong!");
        return;
    }
    query.bindValue(2,ui->cb_Position->currentText());

    if (ui->cb_FC->currentIndex() == 0)
    {
        QMessageBox::critical(this,"Error","FC field is wrong!");
        return;
    }
    QSqlQuery query_2(MainWindow::dbconn);
    QString sqlstr_2 = "select * from football_clubs where fc_name='" + ui->cb_FC->currentText() + "'";
    query_2.exec(sqlstr_2);
    query_2.next();
    query.bindValue(3,query_2.value("fc_id").toInt());

    if(!query.exec())
    {
        QMessageBox::critical(this,"Error",query.lastError().text());
        return;
    }
    QMessageBox::about(this,"Success",QString("UPDATE %1").arg(query.numRowsAffected()));

    selectAll(); // обновляем таблицу
}

void OfferedPlayers::save_selection()
{
    ui->tw_OfferedPlayers->setCurrentCell(ui->tw_OfferedPlayers->currentRow(),0);
}

void OfferedPlayers::filter()
{
    if (ui->le_Surname_Filter->text() == "" && ui->cb_Position_Filter->currentIndex() == 0)
    {
        selectAll();
        return;
    }
    ui->tw_OfferedPlayers->clearContents();
    // Сортировка выкл
    ui->tw_OfferedPlayers->setSortingEnabled(false);

    QSqlQuery query(MainWindow::dbconn); // объект запроса с привязкой к соединению
    QString sqlstr = "select surname,age,playing_position,fc.fc_name from offered_players join football_clubs as fc on(fc.fc_id=offered_players.fc_id) where "; // строка запроса

    bool got_params = false;
    QString value = ui->le_Surname_Filter->text();
    if (value != "")
    {
        sqlstr += "surname like '" + value + "%'";
        got_params = true;
    }
    if (ui->cb_Position_Filter->currentIndex() != 0)
    {
        value = ui->cb_Position_Filter->currentText();
        if (got_params)
            sqlstr += " and ";
        sqlstr += "playing_position='" + value + "'";
    }

    if(!query.exec(sqlstr))
    {
        QMessageBox::critical(this,"Error",query.lastError().text());
        return;
    }

    if(query.isActive())
        ui->tw_OfferedPlayers->setRowCount(query.size());
    else
        ui->tw_OfferedPlayers->setRowCount(0);

    // Запись данных в таблицу
    int i = 0;
    while (query.next())
    {
        ui->tw_OfferedPlayers->setItem(i,0,new QTableWidgetItem(query.value("surname").toString()));
        ui->tw_OfferedPlayers->setItem(i,1,new QTableWidgetItem(query.value("age").toString()));
        ui->tw_OfferedPlayers->setItem(i,2,new QTableWidgetItem(query.value("playing_position").toString()));
        ui->tw_OfferedPlayers->setItem(i,3,new QTableWidgetItem(query.value("fc_name").toString()));
        i++;
    }
    // Размер столбцов
    ui->tw_OfferedPlayers->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->tw_OfferedPlayers->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui->tw_OfferedPlayers->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    ui->tw_OfferedPlayers->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    // Сортировка вкл
    ui->tw_OfferedPlayers->setSortingEnabled(true);
}

void OfferedPlayers::open_fc()
{
    this->close();
    emit open_fc_signal();
}
