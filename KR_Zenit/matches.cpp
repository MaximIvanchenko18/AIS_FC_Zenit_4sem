#include "matches.h"
#include "ui_matches.h"
#include "mainwindow.h"
#include <QStyle>
#include <QDesktopWidget>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlField>
#include <QSqlRecord>
#include <QMessageBox>

Matches::Matches(MatchesInSquads* value, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Matches)
{
    ui->setupUi(this);

    // центрирование
    this->setGeometry(QStyle::alignedRect(Qt::LeftToRight,Qt::AlignCenter,this->size(),qApp->desktop()->availableGeometry()));

    // связи
    connect(ui->btn_Exit,SIGNAL(clicked(bool)),this,SLOT(exit()));
    connect(ui->tw_Matches,SIGNAL(itemSelectionChanged()),this,SLOT(fill_selected()));
    connect(ui->btn_Add,SIGNAL(clicked(bool)),this,SLOT(add()));
    connect(ui->btn_Delete,SIGNAL(clicked(bool)),this,SLOT(del()));
    connect(ui->btn_Edit,SIGNAL(clicked(bool)),this,SLOT(edit()));
    connect(ui->cb_Result_Filter,SIGNAL(currentIndexChanged(int)),this,SLOT(filter()));
    connect(ui->le_Opponent_Filter,SIGNAL(textEdited(QString)),this,SLOT(filter()));
    connect(ui->de_Date_Filter,SIGNAL(dateChanged(QDate)),this,SLOT(filter()));
    connect(ui->tabWidget,SIGNAL(currentChanged(int)),this,SLOT(save_selection()));
    connect(ui->btn_Matches_in_Squads,SIGNAL(clicked(bool)),this,SLOT(open_matches_in_squads()));
    connect(ui->btn_CancelFilter,SIGNAL(clicked(bool)),this,SLOT(cancel_filter()));

    // Параметры таблицы
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

    // Параметры полей
    ui->le_ZenitGoals->setValidator(new QIntValidator(0,99,this));
    ui->le_OtherGoals->setValidator(new QIntValidator(0,99,this));

    matchesinsquads = value;
    connect(this,SIGNAL(open_matches_in_squads_signal(int)),matchesinsquads,SLOT(this_open(int))); // для открытия формы
}

Matches::~Matches()
{
    delete ui;
}

void Matches::exit()
{
    this->close();
    emit open_menu();
}

void Matches::this_open(QString str)
{
    // Заполняем список составов
    fill_squads();
    // Заполняем список противников
    fill_opponents();
    // Очистка фильтров
    ui->de_Date_Filter->setDate(QDate::currentDate());
    ui->le_Opponent_Filter->clear();
    ui->cb_Result_Filter->setCurrentIndex(0);
    // Заполнение таблицы
    selectAll();
    if (str != "")
    {
        ui->de_Date_Filter->setDate(QDate::fromString(str, "yyyy-MM-dd"));
        ui->tw_Matches->setCurrentCell(0,0);
    }
    this->show();
}

void Matches::fill_squads()
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

void Matches::fill_opponents()
{
    QSqlQuery query(MainWindow::dbconn); // объект запроса с привязкой к соединению
    QString sqlstr = "select * from football_clubs"; // строка запроса
    if(!query.exec(sqlstr))
    {
        QMessageBox::critical(this,"Error",query.lastError().text());
        return;
    }

    ui->cb_Opponent->clear();
    // заполняем список
    int i = 0;
    ui->cb_Opponent->addItem(" "); // первый пустой
    while (query.next())
    {
        ui->cb_Opponent->addItem(query.value("fc_name").toString());
        i++;
    }
}

void Matches::selectAll()
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
    QString sqlstr = "select match_date,squad_id,fc.fc_name,match_result,scored_goals,missed_goals from matches join football_clubs as fc on(fc.fc_id=matches.opponent_id)"; // строка запроса
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
    // Очистить поля ввода
    ui->de_Date->setDate(QDate::currentDate());
    ui->cb_Squad->setCurrentIndex(0);
    ui->cb_Opponent->setCurrentIndex(0);
    ui->cb_Result->setCurrentIndex(0);
    ui->le_ZenitGoals->clear();
    ui->le_OtherGoals->clear();
}

void Matches::fill_selected()
{
    int cur_row = ui->tw_Matches->currentRow(); // номер выбранной строки
    if (cur_row < 0) return;
    ui->de_Date->setDate(QDate::fromString(ui->tw_Matches->item(cur_row,0)->text(), "yyyy-MM-dd"));
    ui->cb_Squad->setCurrentText(ui->tw_Matches->item(cur_row,1)->text());
    ui->cb_Opponent->setCurrentText(ui->tw_Matches->item(cur_row,2)->text());
    ui->cb_Result->setCurrentText(ui->tw_Matches->item(cur_row,3)->text());
    ui->le_ZenitGoals->setText(ui->tw_Matches->item(cur_row,4)->text());
    ui->le_OtherGoals->setText(ui->tw_Matches->item(cur_row,5)->text());
}

void Matches::add()
{
    if(!MainWindow::dbconn.isOpen())
    {
        QMessageBox::critical(this,"Error",MainWindow::dbconn.lastError().text());
        return;
    }

    QSqlQuery query(MainWindow::dbconn); // объект запроса с привязкой к соединению
    QString sqlstr = "insert into matches(match_date,squad_id,opponent_id,match_result,scored_goals,missed_goals) values(?,?,?,?,?,?)"; // строка запроса
    query.prepare(sqlstr); // подготовка запроса

    // передаем параметры в запрос
    query.bindValue(0,ui->de_Date->text());
    query.bindValue(1,ui->cb_Squad->currentText().toInt());

    if (ui->cb_Opponent->currentIndex() == 0)
    {
        QMessageBox::critical(this,"Error","Opponent field is wrong!");
        return;
    }
    QSqlQuery query_2(MainWindow::dbconn);
    QString sqlstr_2 = "select * from football_clubs where fc_name='" + ui->cb_Opponent->currentText() + "'";
    query_2.exec(sqlstr_2);
    query_2.next();
    query.bindValue(2,query_2.value("fc_id").toInt());

    query.bindValue(3,ui->cb_Result->currentText());
    query.bindValue(4,ui->le_ZenitGoals->text() != "" ? ui->le_ZenitGoals->text().toInt() : MainWindow::dbconn.record("matches").field("scored_goals").defaultValue());
    query.bindValue(5,ui->le_OtherGoals->text() != "" ? ui->le_OtherGoals->text().toInt() : MainWindow::dbconn.record("matches").field("missed_goals").defaultValue());

    if(!query.exec())
    {
        QMessageBox::critical(this,"Error",query.lastError().text());
        return;
    }
    QMessageBox::about(this,"Success",QString("INSERT 0 %1").arg(query.numRowsAffected()));

    selectAll(); // обновляем таблицу
}

void Matches::del()
{
    if(!MainWindow::dbconn.isOpen())
    {
        QMessageBox::critical(this,"Error",MainWindow::dbconn.lastError().text());
        return;
    }

    int cur_row = ui->tw_Matches->currentRow(); // номер выбранной строки
    if(cur_row < 0) // не выбрана
    {
        QMessageBox::critical(this,"Error","Row is not selected!");
        return;
    }

    // Подтверждение удаления
    if(QMessageBox::question(this,"Delete","Delete selected row?",QMessageBox::Cancel,QMessageBox::Ok)==QMessageBox::Cancel)
        return;

    QSqlQuery query(MainWindow::dbconn);
    QString sqlstr = "delete from matches where match_date='" + ui->tw_Matches->item(cur_row,0)->text() + "'";

    if(!query.exec(sqlstr))
    {
        QMessageBox::critical(this,"Error",query.lastError().text());
        return;
    }
    QMessageBox::about(this,"Success",QString("DELETE %1").arg(query.numRowsAffected()));

    selectAll(); // обновляем таблицу
}

void Matches::edit()
{
    if(!MainWindow::dbconn.isOpen())
    {
        QMessageBox::critical(this,"Error",MainWindow::dbconn.lastError().text());
        return;
    }

    int cur_row = ui->tw_Matches->currentRow(); // номер выбранной строки
    if(cur_row < 0) // не выбрана
    {
        QMessageBox::critical(this,"Error","Row is not selected!");
        return;
    }

    QSqlQuery query(MainWindow::dbconn);
    QString sqlstr = "update matches set (match_date,squad_id,opponent_id,match_result,scored_goals,missed_goals) = (?,?,?,?,?,?) where match_date='"
            + ui->tw_Matches->item(cur_row,0)->text() + "'";
    query.prepare(sqlstr);

    // передаем параметры в запрос
    query.bindValue(0,ui->de_Date->text());
    query.bindValue(1,ui->cb_Squad->currentText().toInt());

    if (ui->cb_Opponent->currentIndex() == 0)
    {
        QMessageBox::critical(this,"Error","Opponent field is wrong!");
        return;
    }
    QSqlQuery query_2(MainWindow::dbconn);
    QString sqlstr_2 = "select * from football_clubs where fc_name='" + ui->cb_Opponent->currentText() + "'";
    query_2.exec(sqlstr_2);
    query_2.next();
    query.bindValue(2,query_2.value("fc_id").toInt());

    query.bindValue(3,ui->cb_Result->currentText());
    query.bindValue(4,ui->le_ZenitGoals->text() != "" ? ui->le_ZenitGoals->text().toInt() : MainWindow::dbconn.record("matches").field("scored_goals").defaultValue());
    query.bindValue(5,ui->le_OtherGoals->text() != "" ? ui->le_OtherGoals->text().toInt() : MainWindow::dbconn.record("matches").field("missed_goals").defaultValue());

    if(!query.exec())
    {
        QMessageBox::critical(this,"Error",query.lastError().text());
        return;
    }
    QMessageBox::about(this,"Success",QString("UPDATE %1").arg(query.numRowsAffected()));

    selectAll(); // обновляем таблицу
}

void Matches::filter()
{
    if (ui->de_Date_Filter->date() == QDate::currentDate() && ui->le_Opponent_Filter->text() == "" && ui->cb_Result_Filter->currentIndex() == 0)
    {
        selectAll();
        return;
    }
    ui->tw_Matches->clearContents();
    // Сортировка выкл
    ui->tw_Matches->setSortingEnabled(false);

    QSqlQuery query(MainWindow::dbconn); // объект запроса с привязкой к соединению
    QString sqlstr = "select match_date,squad_id,fc.fc_name,match_result,scored_goals,missed_goals from matches join football_clubs as fc on(fc.fc_id=matches.opponent_id) where "; // строка запроса
    bool got_params = false;
    if (ui->de_Date_Filter->date() != QDate::currentDate())
    {
        sqlstr += "match_date='" + ui->de_Date_Filter->text() + "'";
        got_params = true;
    }
    QString value = ui->le_Opponent_Filter->text();
    if (value != "")
    {
        if (got_params)
            sqlstr += " and ";
        sqlstr += "fc.fc_name like '" + value + "%'";
        got_params = true;
    }
    if (ui->cb_Result_Filter->currentIndex() != 0)
    {
        value = ui->cb_Result_Filter->currentText();
        if (got_params)
            sqlstr += " and ";
        sqlstr += "match_result='" + value + "'";
    }

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

void Matches::save_selection()
{
    ui->tw_Matches->setCurrentCell(ui->tw_Matches->currentRow(),0);
}

void Matches::open_matches_in_squads()
{
    this->close();
    emit open_matches_in_squads_signal(1);
    matchesinsquads->setButtonToHide(false);
}

void Matches::cancel_filter()
{
    ui->de_Date_Filter->setDate(QDate::currentDate());
    ui->le_Opponent_Filter->clear();
    ui->cb_Result_Filter->setCurrentIndex(0);
}
