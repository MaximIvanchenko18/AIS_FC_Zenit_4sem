#include "playersinsquads.h"
#include "ui_playersinsquads.h"
#include "mainwindow.h"
#include <QStyle>
#include <QDesktopWidget>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlField>
#include <QSqlRecord>
#include <QMessageBox>

PlayersInSquads::PlayersInSquads(MatchesInSquads* value, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PlayersInSquads)
{
    ui->setupUi(this);

    // центрирование
    this->setGeometry(QStyle::alignedRect(Qt::LeftToRight,Qt::AlignCenter,this->size(),qApp->desktop()->availableGeometry()));

    // связи
    connect(ui->btn_Exit,SIGNAL(clicked(bool)),this,SLOT(exit()));
    connect(ui->tw_Squads,SIGNAL(itemSelectionChanged()),this,SLOT(change_tables()));
    connect(ui->checkBox_OnlyCurrent,SIGNAL(stateChanged(int)),this,SLOT(change_tables()));
    connect(ui->btn_CoachStaff,SIGNAL(clicked(bool)),this,SLOT(open_coach_staff()));
    connect(ui->btn_AllPlayers,SIGNAL(clicked(bool)),this,SLOT(open_all_players()));
    connect(ui->btn_ChangePlayer,SIGNAL(clicked(bool)),this,SLOT(open_all_players_change()));
    connect(ui->btn_AddSquad,SIGNAL(clicked(bool)),this,SLOT(add_squad()));
    connect(ui->btn_DeleteSquad,SIGNAL(clicked(bool)),this,SLOT(delete_squad()));
    connect(ui->btn_MatchesInSquads,SIGNAL(clicked(bool)),this,SLOT(open_matches_in_squads()));

    // Параметры таблицы Squads
    ui->tw_Squads->setColumnCount(1); // число столбцов
    ui->tw_Squads->setAutoScroll(true); // скрол
    // Выделение только одной строки
    ui->tw_Squads->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tw_Squads->setSelectionBehavior(QAbstractItemView::SelectRows);
    // Заголовки
    ui->tw_Squads->setHorizontalHeaderItem(0,new QTableWidgetItem("Номер состава"));
    // Растягиваем последний столбец при растягивании формы
    ui->tw_Squads->horizontalHeader()->setStretchLastSection(true);
    // Запрет на изменение
    ui->tw_Squads->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // Параметры таблицы Players
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

    // поля
    squad_num = 1; // по умолчанию
    matchesinsquads = value;
    connect(this,SIGNAL(open_matches_in_squads_signal(int)),matchesinsquads,SLOT(this_open(int))); // для открытия формы
}

PlayersInSquads::~PlayersInSquads()
{
    delete ui;
}

void PlayersInSquads::setButtonToHide(bool value)
{
    if (value)
        ui->btn_CoachStaff->setVisible(false);
    else
        ui->btn_AllPlayers->setVisible(false);
    ui->btn_MatchesInSquads->setVisible(true);
}

void PlayersInSquads::setOnlyCurrentPlayers(bool value)
{
    ui->checkBox_OnlyCurrent->setChecked(value);
}

void PlayersInSquads::exit()
{
    if (!ui->btn_CoachStaff->isVisible())
        emit open_coach_staff_signal();
    else if (!ui->btn_AllPlayers->isVisible())
        emit open_all_players_signal();
    else
        emit open_matches_in_squads_signal();
    this->close();
}

void PlayersInSquads::this_open(int squad_number)
{
    if (squad_number == 0)
    {
        fill_squad_table();
        //fill_players_table(); - выполнится из-за изменения выбора в таблице Squads
    }
    else
    {
        // случай, когда идем из matchesinsquads
        ui->btn_CoachStaff->setVisible(true);
        ui->btn_AllPlayers->setVisible(true);
        ui->btn_MatchesInSquads->setVisible(false);

        this->squad_num = squad_number;
        fill_squad_table();
        //fill_players_table(); - выполнится из-за изменения выбора в таблице Squads
        ui->checkBox_OnlyCurrent->setChecked(true);
    }
    this->show();
}

void PlayersInSquads::fill_squad_table()
{
    int save_squad_num = this->squad_num;
    ui->tw_Squads->clearContents();
    this->squad_num = save_squad_num;
    // Сортировка выкл
    ui->tw_Squads->setSortingEnabled(false);

    if(!MainWindow::dbconn.isOpen())
    {
        QMessageBox::critical(this,"Error",MainWindow::dbconn.lastError().text());
        return;
    }

    QSqlQuery query(MainWindow::dbconn); // объект запроса с привязкой к соединению
    QString sqlstr = "select * from squads"; // строка запроса
    if(!query.exec(sqlstr))
    {
        QMessageBox::critical(this,"Error",query.lastError().text());
        return;
    }

    if(query.isActive())
        ui->tw_Squads->setRowCount(query.size());
    else
        ui->tw_Squads->setRowCount(0);

    // Запись данных в таблицу
    int i = 0;
    while (query.next())
    {
        ui->tw_Squads->setItem(i,0,new QTableWidgetItem(query.value("squad_number").toString()));
        i++;
    }
    // Размер столбца surname
    ui->tw_Squads->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    // Сортировка вкл
    ui->tw_Squads->setSortingEnabled(true);

    ui->tw_Squads->setCurrentCell(squad_num-1,0);
}

void PlayersInSquads::fill_players_table()
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
    QString sqlstr = "select * from players where squad_id=" + QString("%1").arg(squad_num); // строка запроса
    if (ui->checkBox_OnlyCurrent->isChecked())
        sqlstr += " and if_left=false"; // только нынешние игроки состава
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

void PlayersInSquads::change_tables()
{
    squad_num = ui->tw_Squads->currentRow() + 1;
    fill_players_table();
}

void PlayersInSquads::open_coach_staff()
{
    this->close();
    emit open_coach_staff_signal(ui->tw_Squads->item(ui->tw_Squads->currentRow(),0)->text());
}

void PlayersInSquads::open_all_players()
{
    this->close();
    emit open_all_players_signal();
}

void PlayersInSquads::open_all_players_change()
{
    int cur_row = ui->tw_Players->currentRow(); // номер выбранной строки
    if(cur_row < 0) // не выбрана
    {
        QMessageBox::critical(this,"Error","Row is not selected!");
        return;
    }

    this->close();
    emit open_all_players_signal(ui->tw_Players->item(cur_row,0)->text());
}

void PlayersInSquads::add_squad()
{
    if(!MainWindow::dbconn.isOpen())
    {
        QMessageBox::critical(this,"Error",MainWindow::dbconn.lastError().text());
        return;
    }

    QSqlQuery query(MainWindow::dbconn); // объект запроса с привязкой к соединению
    QString sqlstr = "insert into squads values(?,?)"; // строка запроса
    query.prepare(sqlstr); // подготовка запроса

    // передаем параметры в запрос
    query.bindValue(0,ui->tw_Squads->rowCount()+1);
    query.bindValue(1,ui->tw_Squads->rowCount()+1);

    if(!query.exec())
    {
        QMessageBox::critical(this,"Error",query.lastError().text());
        return;
    }
    QMessageBox::about(this,"Success",QString("INSERT 0 %1").arg(query.numRowsAffected()));

    fill_squad_table();
}

void PlayersInSquads::delete_squad()
{
    if(!MainWindow::dbconn.isOpen())
    {
        QMessageBox::critical(this,"Error",MainWindow::dbconn.lastError().text());
        return;
    }

    // Подтверждение удаления
    if(QMessageBox::question(this,"Delete","Delete last squad?",QMessageBox::Cancel,QMessageBox::Ok)==QMessageBox::Cancel)
        return;

    QSqlQuery query(MainWindow::dbconn);
    QString sqlstr = "delete from squads where squad_id=" + QString("%1").arg(ui->tw_Squads->rowCount());

    if(!query.exec(sqlstr))
    {
        QMessageBox::critical(this,"Error",query.lastError().text());
        return;
    }
    QMessageBox::about(this,"Success",QString("DELETE %1").arg(query.numRowsAffected()));

    if (squad_num == ui->tw_Squads->rowCount())
        squad_num--;
    fill_squad_table();
}

void PlayersInSquads::open_matches_in_squads()
{
    this->close();
    emit open_matches_in_squads_signal(squad_num);
    matchesinsquads->setButtonToHide(true);
}
