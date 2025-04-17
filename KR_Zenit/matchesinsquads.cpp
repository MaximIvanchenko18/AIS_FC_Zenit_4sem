#include "matchesinsquads.h"
#include "ui_matchesinsquads.h"
#include "mainwindow.h"
#include <QStyle>
#include <QDesktopWidget>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlField>
#include <QSqlRecord>
#include <QMessageBox>

MatchesInSquads::MatchesInSquads(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MatchesInSquads)
{
    ui->setupUi(this);

    // центрирование
    this->setGeometry(QStyle::alignedRect(Qt::LeftToRight,Qt::AlignCenter,this->size(),qApp->desktop()->availableGeometry()));

    // связи
    connect(ui->btn_Exit,SIGNAL(clicked(bool)),this,SLOT(exit()));
    connect(ui->tw_Squads,SIGNAL(itemSelectionChanged()),this,SLOT(change_tables()));
    connect(ui->btn_PlayersInSquads,SIGNAL(clicked(bool)),this,SLOT(open_players_in_squads()));
    connect(ui->btn_AllMatches,SIGNAL(clicked(bool)),this,SLOT(open_matches()));
    connect(ui->btn_CoachStaff,SIGNAL(clicked(bool)),this,SLOT(open_coach_staff()));
    connect(ui->btn_ChangeMatch,SIGNAL(clicked(bool)),this,SLOT(open_matches_change()));
    connect(ui->btn_AddSquad,SIGNAL(clicked(bool)),this,SLOT(add_squad()));
    connect(ui->btn_DeleteSquad,SIGNAL(clicked(bool)),this,SLOT(delete_squad()));

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

    // Параметры таблицы Matches
    ui->tw_Matches->setColumnCount(6); // число столбцов
    ui->tw_Matches->setAutoScroll(true); // скрол
    // Выделение только одной строки
    ui->tw_Matches->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tw_Matches->setSelectionBehavior(QAbstractItemView::SelectRows);
    // Заголовки
    ui->tw_Matches->setHorizontalHeaderItem(0,new QTableWidgetItem("Дата"));
    ui->tw_Matches->setHorizontalHeaderItem(1,new QTableWidgetItem("Номер состава"));
    ui->tw_Matches->setHorizontalHeaderItem(2,new QTableWidgetItem("Соперник"));
    ui->tw_Matches->setHorizontalHeaderItem(3,new QTableWidgetItem("Результат"));
    ui->tw_Matches->setHorizontalHeaderItem(4,new QTableWidgetItem("Забитые мячи"));
    ui->tw_Matches->setHorizontalHeaderItem(5,new QTableWidgetItem("Пропущенные мячи"));
    // Растягиваем последний столбец при растягивании формы
    ui->tw_Matches->horizontalHeader()->setStretchLastSection(true);
    // Запрет на изменение
    ui->tw_Matches->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // поля
    squad_num = 1; // по умолчанию
}

MatchesInSquads::~MatchesInSquads()
{
    delete ui;
}

void MatchesInSquads::setButtonToHide(bool value)
{
    if (value)
        ui->btn_PlayersInSquads->setVisible(false);
    else
        ui->btn_AllMatches->setVisible(false);
}

void MatchesInSquads::this_open(int squad_number)
{
    if (squad_number == 0)
    {
        fill_squad_table();
        //fill_players_table(); - выполнится из-за изменения выбора в таблице Squads
    }
    else
    {
        ui->btn_PlayersInSquads->setVisible(true);
        ui->btn_AllMatches->setVisible(true);
        this->squad_num = squad_number;
        fill_squad_table();
        //fill_players_table(); - выполнится из-за изменения выбора в таблице Squads
    }
    this->show();
}

void MatchesInSquads::exit()
{
    if (ui->btn_PlayersInSquads->isVisible())
        emit open_matches_signal();
    else
        emit open_players_in_squads_signal();
    this->close();
}

void MatchesInSquads::fill_squad_table()
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

void MatchesInSquads::fill_matches_table()
{
    ui->tw_Matches->clearContents();
    // Сортировка выкл
    ui->tw_Matches->setSortingEnabled(false);

    if(!MainWindow::dbconn.isOpen())
    {
        QMessageBox::critical(this,"Error",MainWindow::dbconn.lastError().text());
        return;
    }

    QSqlQuery query(MainWindow::dbconn); // объект запроса с привязкой к соединению
    QString sqlstr = "select match_date,squad_id,fc.fc_name,match_result,scored_goals,missed_goals from matches join football_clubs as fc on(fc.fc_id=matches.opponent_id) where squad_id=" + QString("%1").arg(squad_num); // строка запроса
    if(!query.exec(sqlstr))
    {
        QMessageBox::critical(this,"Error",query.lastError().text());
        return;
    }

    if(query.isActive())
        ui->tw_Matches->setRowCount(query.size());
    else
        ui->tw_Matches->setRowCount(0);

    // Запись данных в таблицу
    int i = 0;
    while (query.next())
    {
        ui->tw_Matches->setItem(i,0,new QTableWidgetItem(query.value("match_date").toString()));
        ui->tw_Matches->setItem(i,1,new QTableWidgetItem(query.value("squad_id").toString()));
        ui->tw_Matches->setItem(i,2,new QTableWidgetItem(query.value("fc_name").toString()));
        ui->tw_Matches->setItem(i,3,new QTableWidgetItem(query.value("match_result").toString()));
        ui->tw_Matches->setItem(i,4,new QTableWidgetItem(query.value("scored_goals").toString()));
        ui->tw_Matches->setItem(i,5,new QTableWidgetItem(query.value("missed_goals").toString()));
        i++;
    }
    // Размер столбцов
    ui->tw_Matches->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->tw_Matches->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui->tw_Matches->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    ui->tw_Matches->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    ui->tw_Matches->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    ui->tw_Matches->horizontalHeader()->setSectionResizeMode(5, QHeaderView::ResizeToContents);
    // Сортировка вкл
    ui->tw_Matches->setSortingEnabled(true);
}

void MatchesInSquads::change_tables()
{
    squad_num = ui->tw_Squads->currentRow() + 1;
    fill_matches_table();
}

void MatchesInSquads::open_players_in_squads()
{
    this->close();
    emit open_players_in_squads_signal(squad_num);
}

void MatchesInSquads::open_matches()
{
    this->close();
    emit open_matches_signal();
}

void MatchesInSquads::open_coach_staff()
{
    this->close();
    emit open_coach_staff_signal(ui->tw_Squads->item(ui->tw_Squads->currentRow(),0)->text());
}

void MatchesInSquads::open_matches_change()
{
    int cur_row = ui->tw_Matches->currentRow(); // номер выбранной строки
    if(cur_row < 0) // не выбрана
    {
        QMessageBox::critical(this,"Error","Row is not selected!");
        return;
    }

    this->close();
    emit open_matches_signal(ui->tw_Matches->item(cur_row,0)->text());
}

void MatchesInSquads::add_squad()
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

void MatchesInSquads::delete_squad()
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
