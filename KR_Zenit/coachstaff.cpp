#include "coachstaff.h"
#include "ui_coachstaff.h"
#include "mainwindow.h"
#include <QStyle>
#include <QDesktopWidget>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlField>
#include <QSqlRecord>
#include <QMessageBox>

CoachStaff::CoachStaff(PlayersInSquads* value, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CoachStaff)
{
    ui->setupUi(this);

    // центрирование
    this->setGeometry(QStyle::alignedRect(Qt::LeftToRight,Qt::AlignCenter,this->size(),qApp->desktop()->availableGeometry()));

    // связи
    connect(ui->btn_Exit,SIGNAL(clicked(bool)),this,SLOT(exit()));
    connect(ui->tw_CoachStaff,SIGNAL(itemSelectionChanged()),this,SLOT(fill_selected()));
    connect(ui->btn_Add,SIGNAL(clicked(bool)),this,SLOT(add()));
    connect(ui->btn_Delete,SIGNAL(clicked(bool)),this,SLOT(del()));
    connect(ui->btn_Edit,SIGNAL(clicked(bool)),this,SLOT(edit()));
    connect(ui->le_Squad_Filter,SIGNAL(textEdited(QString)),this,SLOT(squad_filter()));
    connect(ui->btn_Players,SIGNAL(clicked(bool)),this,SLOT(open_players_in_squads()));
    connect(ui->tabWidget,SIGNAL(currentChanged(int)),this,SLOT(save_selection()));

    // Параметры таблицы
    ui->tw_CoachStaff->setColumnCount(2); // число столбцов
    ui->tw_CoachStaff->setAutoScroll(true); // скрол
    // Выделение только одной строки
    ui->tw_CoachStaff->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tw_CoachStaff->setSelectionBehavior(QAbstractItemView::SelectRows);
    // Заголовки
    ui->tw_CoachStaff->setHorizontalHeaderItem(0,new QTableWidgetItem("Фамилия"));
    ui->tw_CoachStaff->setHorizontalHeaderItem(1,new QTableWidgetItem("Номер состава"));
    // Растягиваем последний столбец при растягивании формы
    ui->tw_CoachStaff->horizontalHeader()->setStretchLastSection(true);
    // Запрет на изменение
    ui->tw_CoachStaff->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // Параметры полей
    ui->le_Squad_Filter->setValidator(new QIntValidator(0,9,this));

    playersinsquad = value;
    connect(this,SIGNAL(open_players_in_squads_signal(int)),playersinsquad,SLOT(this_open(int))); // для открытия формы
}

CoachStaff::~CoachStaff()
{
    delete ui;
}

void CoachStaff::exit()
{
    this->close();
    emit open_menu();
}

void CoachStaff::this_open(QString squad_num)
{
    // Заполнение составов
    fill_squads();
    // Очистка фильтра
    ui->le_Squad_Filter->clear();
    // Заполнение таблицы
    selectAll();
    if (squad_num != "")
    {
        ui->le_Squad_Filter->setText(squad_num);
        squad_filter();
    }
    this->show();
}

void CoachStaff::fill_squads()
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

void CoachStaff::selectAll()
{
    ui->tw_CoachStaff->clearContents();
    // Сортировка выкл
    ui->tw_CoachStaff->setSortingEnabled(false);

    if(!MainWindow::dbconn.isOpen())
    {
        QMessageBox::critical(this,"Error",MainWindow::dbconn.lastError().text());
        return;
    }

    QSqlQuery query(MainWindow::dbconn); // объект запроса с привязкой к соединению
    QString sqlstr = "select * from coach_staff"; // строка запроса
    if(!query.exec(sqlstr))
    {
        QMessageBox::critical(this,"Error",query.lastError().text());
        return;
    }

    if(query.isActive())
        ui->tw_CoachStaff->setRowCount(query.size());
    else
        ui->tw_CoachStaff->setRowCount(0);

    // Запись данных в таблицу
    int i = 0;
    while (query.next())
    {
        ui->tw_CoachStaff->setItem(i,0,new QTableWidgetItem(query.value("surname").toString()));
        ui->tw_CoachStaff->setItem(i,1,new QTableWidgetItem(query.value("squad_id").toString()));
        i++;
    }
    // Размер столбца surname
    ui->tw_CoachStaff->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    // Сортировка вкл
    ui->tw_CoachStaff->setSortingEnabled(true);
    // Очистить поля ввода
    ui->le_Surname->clear();
    ui->cb_Squad->setCurrentIndex(0);
}

void CoachStaff::fill_selected()
{
    int cur_row = ui->tw_CoachStaff->currentRow(); // номер выбранной строки
    if (cur_row < 0) return;
    ui->le_Surname->setText(ui->tw_CoachStaff->item(cur_row,0)->text());
    ui->cb_Squad->setCurrentText(ui->tw_CoachStaff->item(cur_row,1)->text());
}

void CoachStaff::add()
{
    if(!MainWindow::dbconn.isOpen())
    {
        QMessageBox::critical(this,"Error",MainWindow::dbconn.lastError().text());
        return;
    }

    QSqlQuery query(MainWindow::dbconn); // объект запроса с привязкой к соединению
    QString sqlstr = "insert into coach_staff(surname,squad_id) values(?,?)"; // строка запроса
    query.prepare(sqlstr); // подготовка запроса

    // передаем параметры в запрос
    query.bindValue(0,ui->le_Surname->text());
    query.bindValue(1,ui->cb_Squad->currentText().toInt());
    if (ui->le_Surname->text() == "")
    {
        QMessageBox::critical(this,"Error","Surname field is empty!");
        return;
    }

    if(!query.exec())
    {
        QMessageBox::critical(this,"Error",query.lastError().text());
        return;
    }
    QMessageBox::about(this,"Success",QString("INSERT 0 %1").arg(query.numRowsAffected()));

    selectAll(); // обновляем таблицу
}

void CoachStaff::del()
{
    if(!MainWindow::dbconn.isOpen())
    {
        QMessageBox::critical(this,"Error",MainWindow::dbconn.lastError().text());
        return;
    }

    int cur_row = ui->tw_CoachStaff->currentRow(); // номер выбранной строки
    if(cur_row < 0) // не выбрана
    {
        QMessageBox::critical(this,"Error","Row is not selected!");
        return;
    }

    // Подтверждение удаления
    if(QMessageBox::question(this,"Delete","Delete selected row?",QMessageBox::Cancel,QMessageBox::Ok)==QMessageBox::Cancel)
        return;

    QSqlQuery query(MainWindow::dbconn);
    QString sqlstr = "delete from coach_staff where surname='" + ui->tw_CoachStaff->item(cur_row,0)->text() + "' and squad_id="
            + ui->tw_CoachStaff->item(cur_row,1)->text();

    if(!query.exec(sqlstr))
    {
        QMessageBox::critical(this,"Error",query.lastError().text());
        return;
    }
    QMessageBox::about(this,"Success",QString("DELETE %1").arg(query.numRowsAffected()));

    selectAll(); // обновляем таблицу
}

void CoachStaff::edit()
{
    if(!MainWindow::dbconn.isOpen())
    {
        QMessageBox::critical(this,"Error",MainWindow::dbconn.lastError().text());
        return;
    }

    int cur_row = ui->tw_CoachStaff->currentRow(); // номер выбранной строки
    if(cur_row < 0) // не выбрана
    {
        QMessageBox::critical(this,"Error","Row is not selected!");
        return;
    }

    QSqlQuery query(MainWindow::dbconn);
    QString sqlstr = "update coach_staff set (surname,squad_id) = (?,?) where surname='"
            + ui->tw_CoachStaff->item(cur_row,0)->text() + "' and squad_id=" + ui->tw_CoachStaff->item(cur_row,1)->text();
    query.prepare(sqlstr);

    query.bindValue(0,ui->le_Surname->text());
    query.bindValue(1,ui->cb_Squad->currentText().toInt());
    if (ui->le_Surname->text() == "")
    {
        QMessageBox::critical(this,"Error","Surname field is empty!");
        return;
    }

    if(!query.exec())
    {
        QMessageBox::critical(this,"Error",query.lastError().text());
        return;
    }
    QMessageBox::about(this,"Success",QString("UPDATE %1").arg(query.numRowsAffected()));

    selectAll(); // обновляем таблицу
}

void CoachStaff::squad_filter()
{
    QString value = ui->le_Squad_Filter->text();
    if (value == "")
    {
        selectAll();
        return;
    }
    ui->tw_CoachStaff->clearContents();
    // Сортировка выкл
    ui->tw_CoachStaff->setSortingEnabled(false);

    QSqlQuery query(MainWindow::dbconn); // объект запроса с привязкой к соединению
    QString sqlstr = "select * from coach_staff where squad_id=" + value; // строка запроса
    if(!query.exec(sqlstr))
    {
        QMessageBox::critical(this,"Error",query.lastError().text());
        return;
    }

    if(query.isActive())
        ui->tw_CoachStaff->setRowCount(query.size());
    else
        ui->tw_CoachStaff->setRowCount(0);

    // Запись данных в таблицу
    int i = 0;
    while (query.next())
    {
        ui->tw_CoachStaff->setItem(i,0,new QTableWidgetItem(query.value("surname").toString()));
        ui->tw_CoachStaff->setItem(i,1,new QTableWidgetItem(query.value("squad_id").toString()));
        i++;
    }
    // Размер столбца surname
    ui->tw_CoachStaff->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    // Сортировка вкл
    ui->tw_CoachStaff->setSortingEnabled(true);
}

void CoachStaff::open_players_in_squads()
{
    int cur_row = ui->tw_CoachStaff->currentRow(); // номер выбранной строки
    if(cur_row < 0) // не выбрана
    {
        QMessageBox::critical(this,"Error","Row is not selected!");
        return;
    }

    this->close();
    emit open_players_in_squads_signal(ui->tw_CoachStaff->item(cur_row,1)->text().toInt());
    playersinsquad->setButtonToHide(true); // прячем лишнюю кнопку
}

void CoachStaff::save_selection()
{
    ui->tw_CoachStaff->setCurrentCell(ui->tw_CoachStaff->currentRow(),0);
}
