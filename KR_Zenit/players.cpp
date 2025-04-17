#include "players.h"
#include "ui_players.h"
#include "mainwindow.h"
#include <QStyle>
#include <QDesktopWidget>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlField>
#include <QSqlRecord>
#include <QMessageBox>

Players::Players(PlayersInSquads* value, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Players)
{
    ui->setupUi(this);

    // центрирование
    this->setGeometry(QStyle::alignedRect(Qt::LeftToRight,Qt::AlignCenter,this->size(),qApp->desktop()->availableGeometry()));

    // связи
    connect(ui->btn_Exit,SIGNAL(clicked(bool)),this,SLOT(exit()));
    connect(ui->tw_Players,SIGNAL(itemSelectionChanged()),this,SLOT(fill_selected()));
    connect(ui->btn_Add,SIGNAL(clicked(bool)),this,SLOT(add()));
    connect(ui->btn_Delete,SIGNAL(clicked(bool)),this,SLOT(del()));
    connect(ui->btn_Edit,SIGNAL(clicked(bool)),this,SLOT(edit()));
    connect(ui->le_Surname_Filter,SIGNAL(textEdited(QString)),this,SLOT(filter()));
    connect(ui->cb_Position_Filter,SIGNAL(currentIndexChanged(int)),this,SLOT(filter()));
    connect(ui->cb_isleft_Filter,SIGNAL(currentIndexChanged(int)),this,SLOT(filter()));
    connect(ui->btn_Players_in_Squads,SIGNAL(clicked(bool)),this,SLOT(open_players_in_squads()));
    connect(ui->tabWidget,SIGNAL(currentChanged(int)),this,SLOT(save_selection()));

    // Параметры таблицы
    ui->tw_Players->setColumnCount(10); // число столбцов
    ui->tw_Players->setAutoScroll(true); // скрол
    // Выделение только одной строки
    ui->tw_Players->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tw_Players->setSelectionBehavior(QAbstractItemView::SelectRows);
    // Заголовки
    ui->tw_Players->setHorizontalHeaderItem(0,new QTableWidgetItem("Фамилия"));
    ui->tw_Players->setHorizontalHeaderItem(1,new QTableWidgetItem("Возраст"));
    ui->tw_Players->setHorizontalHeaderItem(2,new QTableWidgetItem("Позиция"));
    ui->tw_Players->setHorizontalHeaderItem(3,new QTableWidgetItem("Состав"));
    ui->tw_Players->setHorizontalHeaderItem(4,new QTableWidgetItem("Рейтинг"));
    ui->tw_Players->setHorizontalHeaderItem(5,new QTableWidgetItem("Голы"));
    ui->tw_Players->setHorizontalHeaderItem(6,new QTableWidgetItem("Ассисты"));
    ui->tw_Players->setHorizontalHeaderItem(7,new QTableWidgetItem("Желтые карты"));
    ui->tw_Players->setHorizontalHeaderItem(8,new QTableWidgetItem("Красные карты"));
    ui->tw_Players->setHorizontalHeaderItem(9,new QTableWidgetItem("Покинул клуб?"));
    // Растягиваем последний столбец при растягивании формы
    ui->tw_Players->horizontalHeader()->setStretchLastSection(true);
    // Запрет на изменение
    ui->tw_Players->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // Параметры полей
    ui->le_Age->setValidator(new QIntValidator(0,99,this));
    ui->le_Goals->setValidator(new QIntValidator(0,99,this));
    ui->le_Assists->setValidator(new QIntValidator(0,99,this));
    ui->le_YellowCards->setValidator(new QIntValidator(0,99,this));
    ui->le_RedCards->setValidator(new QIntValidator(0,99,this));

    playersinsquad = value;
    connect(this,SIGNAL(open_players_in_squads_signal(int)),playersinsquad,SLOT(this_open(int))); // для открытия формы
}

Players::~Players()
{
    delete ui;
}

void Players::exit()
{
    this->close();
    emit open_menu();
}

void Players::this_open(QString str)
{
    // Заполнение составов
    fill_squads();
    // Очистка фильтра
    ui->le_Surname_Filter->clear();
    ui->cb_Position_Filter->setCurrentIndex(0);
    ui->cb_isleft_Filter->setCurrentIndex(0);
    // Заполнение таблицы
    selectAll();
    if (str != "")
    {
        ui->le_Surname_Filter->setText(str);
        filter();
        ui->tw_Players->setCurrentCell(0,0);
    }
    this->show();
}

void Players::fill_squads()
{
    QSqlQuery query(MainWindow::dbconn); // объект запроса с привязкой к соединению
    QString sqlstr = "select * from squads"; // строка запроса
    if(!query.exec(sqlstr))
    {
        QMessageBox::critical(this,"Error",query.lastError().text());
        return;
    }

    ui->cb_Squad->clear();
    // заполняем список
    int i = 0;
    while (query.next())
    {
        ui->cb_Squad->addItem(query.value("squad_number").toString());
        i++;
    }
}

void Players::selectAll()
{
    ui->tw_Players->clearContents();
    // Сортировка выкл
    ui->tw_Players->setSortingEnabled(false);

    if(!MainWindow::dbconn.isOpen())
    {
        QMessageBox::critical(this,"Error",MainWindow::dbconn.lastError().text());
        return;
    }

    QSqlQuery query(MainWindow::dbconn); // объект запроса с привязкой к соединению
    QString sqlstr = "select * from players"; // строка запроса
    if(!query.exec(sqlstr))
    {
        QMessageBox::critical(this,"Error",query.lastError().text());
        return;
    }

    if(query.isActive())
        ui->tw_Players->setRowCount(query.size());
    else
        ui->tw_Players->setRowCount(0);

    // Запись данных в таблицу
    int i = 0;
    while (query.next())
    {
        ui->tw_Players->setItem(i,0,new QTableWidgetItem(query.value("surname").toString()));
        ui->tw_Players->setItem(i,1,new QTableWidgetItem(query.value("age").toString()));
        ui->tw_Players->setItem(i,2,new QTableWidgetItem(query.value("playing_position").toString()));
        ui->tw_Players->setItem(i,3,new QTableWidgetItem(query.value("squad_id").toString()));
        ui->tw_Players->setItem(i,4,new QTableWidgetItem(query.value("rating").toString()));
        ui->tw_Players->setItem(i,5,new QTableWidgetItem(query.value("goals").toString()));
        ui->tw_Players->setItem(i,6,new QTableWidgetItem(query.value("assists").toString()));
        ui->tw_Players->setItem(i,7,new QTableWidgetItem(query.value("yellow_cards").toString()));
        ui->tw_Players->setItem(i,8,new QTableWidgetItem(query.value("red_cards").toString()));
        ui->tw_Players->setItem(i,9,new QTableWidgetItem(query.value("if_left").toString()));
        i++;
    }
    // Размер столбцов
    ui->tw_Players->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->tw_Players->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    ui->tw_Players->horizontalHeader()->setSectionResizeMode(7, QHeaderView::ResizeToContents);
    ui->tw_Players->horizontalHeader()->setSectionResizeMode(8, QHeaderView::ResizeToContents);
    ui->tw_Players->horizontalHeader()->setSectionResizeMode(9, QHeaderView::ResizeToContents);
    // Сортировка вкл
    ui->tw_Players->setSortingEnabled(true);
    // Очистить поля ввода
    ui->le_Surname->clear();
    ui->le_Age->clear();
    ui->cb_Position->setCurrentIndex(0);
    ui->cb_Squad->setCurrentIndex(0);
    ui->le_Rating->clear();
    ui->le_Goals->clear();
    ui->le_Assists->clear();
    ui->le_YellowCards->clear();
    ui->le_RedCards->clear();
    ui->cb_isleft->setCurrentIndex(0);
}

void Players::fill_selected()
{
    int cur_row = ui->tw_Players->currentRow(); // номер выбранной строки
    if (cur_row < 0) return;
    ui->le_Surname->setText(ui->tw_Players->item(cur_row,0)->text());
    ui->le_Age->setText(ui->tw_Players->item(cur_row,1)->text());
    ui->cb_Position->setCurrentText(ui->tw_Players->item(cur_row,2)->text());
    ui->cb_Squad->setCurrentText(ui->tw_Players->item(cur_row,3)->text());
    ui->le_Rating->setText(ui->tw_Players->item(cur_row,4)->text());
    ui->le_Goals->setText(ui->tw_Players->item(cur_row,5)->text());
    ui->le_Assists->setText(ui->tw_Players->item(cur_row,6)->text());
    ui->le_YellowCards->setText(ui->tw_Players->item(cur_row,7)->text());
    ui->le_RedCards->setText(ui->tw_Players->item(cur_row,8)->text());
    ui->cb_isleft->setCurrentText(ui->tw_Players->item(cur_row,9)->text());
}

void Players::add()
{
    if(!MainWindow::dbconn.isOpen())
    {
        QMessageBox::critical(this,"Error",MainWindow::dbconn.lastError().text());
        return;
    }

    QSqlQuery query(MainWindow::dbconn); // объект запроса с привязкой к соединению
    QString sqlstr = "insert into players(surname,age,playing_position,squad_id,rating,goals,assists,yellow_cards,red_cards,if_left) values(?,?,?,?,?,?,?,?,?,?)"; // строка запроса
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
    else
        query.bindValue(2,ui->cb_Position->currentText());
    query.bindValue(3,ui->cb_Squad->currentText().toInt());

    bool ok;
    double val = ui->le_Rating->text().toDouble(&ok);
    if (ok)
        query.bindValue(4,val);
    else
    {
        QMessageBox::critical(this,"Error","Rating field is wrong! Using default value!");
        query.bindValue(4,MainWindow::dbconn.record("players").field("rating").defaultValue());
    }

    query.bindValue(5,ui->le_Goals->text() != "" ? ui->le_Goals->text().toInt() : MainWindow::dbconn.record("players").field("goals").defaultValue());
    query.bindValue(6,ui->le_Assists->text() != "" ? ui->le_Assists->text().toInt(): MainWindow::dbconn.record("players").field("assists").defaultValue());
    query.bindValue(7,ui->le_YellowCards->text() != "" ? ui->le_YellowCards->text().toInt() : MainWindow::dbconn.record("players").field("yellow_cards").defaultValue());
    query.bindValue(8,ui->le_RedCards->text() != "" ? ui->le_RedCards->text().toInt() : MainWindow::dbconn.record("players").field("red_cards").defaultValue());

    if (ui->cb_isleft->currentIndex() == 0)
        query.bindValue(9,false);
    else
        query.bindValue(9,true);

    if(!query.exec())
    {
        QMessageBox::critical(this,"Error",query.lastError().text());
        return;
    }
    QMessageBox::about(this,"Success",QString("INSERT 0 %1").arg(query.numRowsAffected()));

    selectAll(); // обновляем таблицу
}

void Players::del()
{
    if(!MainWindow::dbconn.isOpen())
    {
        QMessageBox::critical(this,"Error",MainWindow::dbconn.lastError().text());
        return;
    }

    int cur_row = ui->tw_Players->currentRow(); // номер выбранной строки
    if(cur_row < 0) // не выбрана
    {
        QMessageBox::critical(this,"Error","Row is not selected!");
        return;
    }

    // Подтверждение удаления
    if(QMessageBox::question(this,"Delete","Delete selected row?",QMessageBox::Cancel,QMessageBox::Ok)==QMessageBox::Cancel)
        return;

    QSqlQuery query(MainWindow::dbconn);
    QString sqlstr = "delete from players where surname='" + ui->tw_Players->item(cur_row,0)->text() + "' and age="
            + ui->tw_Players->item(cur_row,1)->text() + " and rating=" + ui->tw_Players->item(cur_row,4)->text();

    if(!query.exec(sqlstr))
    {
        QMessageBox::critical(this,"Error",query.lastError().text());
        return;
    }
    QMessageBox::about(this,"Success",QString("DELETE %1").arg(query.numRowsAffected()));

    selectAll(); // обновляем таблицу
}

void Players::edit()
{
    if(!MainWindow::dbconn.isOpen())
    {
        QMessageBox::critical(this,"Error",MainWindow::dbconn.lastError().text());
        return;
    }

    int cur_row = ui->tw_Players->currentRow(); // номер выбранной строки
    if(cur_row < 0) // не выбрана
    {
        QMessageBox::critical(this,"Error","Row is not selected!");
        return;
    }

    QSqlQuery query(MainWindow::dbconn);
    QString sqlstr = "update players set (surname,age,playing_position,squad_id,rating,goals,assists,yellow_cards,red_cards,if_left) = (?,?,?,?,?,?,?,?,?,?) where surname='" + ui->tw_Players->item(cur_row,0)->text() + "' and age="
            + ui->tw_Players->item(cur_row,1)->text() + " and rating=" + ui->tw_Players->item(cur_row,4)->text();
    query.prepare(sqlstr);

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
    else
        query.bindValue(2,ui->cb_Position->currentText());
    query.bindValue(3,ui->cb_Squad->currentText().toInt());

    bool ok;
    double val = ui->le_Rating->text().toDouble(&ok);
    if (ok)
        query.bindValue(4,val);
    else
    {
        QMessageBox::critical(this,"Error","Rating field is wrong! Using default value!");
        query.bindValue(4,MainWindow::dbconn.record("players").field("rating").defaultValue());
    }

    query.bindValue(5,ui->le_Goals->text() != "" ? ui->le_Goals->text().toInt() : MainWindow::dbconn.record("players").field("goals").defaultValue());
    query.bindValue(6,ui->le_Assists->text() != "" ? ui->le_Assists->text().toInt(): MainWindow::dbconn.record("players").field("assists").defaultValue());
    query.bindValue(7,ui->le_YellowCards->text() != "" ? ui->le_YellowCards->text().toInt() : MainWindow::dbconn.record("players").field("yellow_cards").defaultValue());
    query.bindValue(8,ui->le_RedCards->text() != "" ? ui->le_RedCards->text().toInt() : MainWindow::dbconn.record("players").field("red_cards").defaultValue());

    if (ui->cb_isleft->currentIndex() == 0)
        query.bindValue(9,false);
    else
        query.bindValue(9,true);

    if(!query.exec())
    {
        QMessageBox::critical(this,"Error",query.lastError().text());
        return;
    }
    QMessageBox::about(this,"Success",QString("UPDATE %1").arg(query.numRowsAffected()));

    selectAll(); // обновляем таблицу
}

void Players::filter()
{
    if (ui->le_Surname_Filter->text() == "" && ui->cb_Position_Filter->currentIndex() == 0 && ui->cb_isleft_Filter->currentIndex() == 0)
    {
        selectAll();
        return;
    }
    ui->tw_Players->clearContents();
    // Сортировка выкл
    ui->tw_Players->setSortingEnabled(false);

    QSqlQuery query(MainWindow::dbconn); // объект запроса с привязкой к соединению
    QString sqlstr = "select * from players where "; // строка запроса

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
        else
            got_params = true;
        sqlstr += "playing_position='" + value + "'";
    }
    if (ui->cb_isleft_Filter->currentIndex() != 0)
    {
        value = ui->cb_isleft_Filter->currentText();
        if (got_params)
            sqlstr += " and ";
        sqlstr += "if_left=" + value;
    }

    if(!query.exec(sqlstr))
    {
        QMessageBox::critical(this,"Error",query.lastError().text());
        return;
    }

    if(query.isActive())
        ui->tw_Players->setRowCount(query.size());
    else
        ui->tw_Players->setRowCount(0);

    // Запись данных в таблицу
    int i = 0;
    while (query.next())
    {
        ui->tw_Players->setItem(i,0,new QTableWidgetItem(query.value("surname").toString()));
        ui->tw_Players->setItem(i,1,new QTableWidgetItem(query.value("age").toString()));
        ui->tw_Players->setItem(i,2,new QTableWidgetItem(query.value("playing_position").toString()));
        ui->tw_Players->setItem(i,3,new QTableWidgetItem(query.value("squad_id").toString()));
        ui->tw_Players->setItem(i,4,new QTableWidgetItem(query.value("rating").toString()));
        ui->tw_Players->setItem(i,5,new QTableWidgetItem(query.value("goals").toString()));
        ui->tw_Players->setItem(i,6,new QTableWidgetItem(query.value("assists").toString()));
        ui->tw_Players->setItem(i,7,new QTableWidgetItem(query.value("yellow_cards").toString()));
        ui->tw_Players->setItem(i,8,new QTableWidgetItem(query.value("red_cards").toString()));
        ui->tw_Players->setItem(i,9,new QTableWidgetItem(query.value("if_left").toString()));
        i++;
    }
    // Размер столбцов
    ui->tw_Players->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->tw_Players->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    ui->tw_Players->horizontalHeader()->setSectionResizeMode(7, QHeaderView::ResizeToContents);
    ui->tw_Players->horizontalHeader()->setSectionResizeMode(8, QHeaderView::ResizeToContents);
    ui->tw_Players->horizontalHeader()->setSectionResizeMode(9, QHeaderView::ResizeToContents);
    // Сортировка вкл
    ui->tw_Players->setSortingEnabled(true);
}

void Players::open_players_in_squads()
{
    this->close();
    emit open_players_in_squads_signal(1);
    playersinsquad->setButtonToHide(false); // прячем лишнюю кнопку
    playersinsquad->setOnlyCurrentPlayers(false); // не только нынешние игроки
}

void Players::save_selection()
{
    ui->tw_Players->setCurrentCell(ui->tw_Players->currentRow(),0);
}
