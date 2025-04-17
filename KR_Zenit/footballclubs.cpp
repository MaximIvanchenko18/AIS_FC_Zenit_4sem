#include "footballclubs.h"
#include "ui_footballclubs.h"
#include "mainwindow.h"
#include <QStyle>
#include <QDesktopWidget>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlField>
#include <QSqlRecord>
#include <QMessageBox>

FootballClubs::FootballClubs(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FootballClubs)
{
    ui->setupUi(this);

    // центрирование
    this->setGeometry(QStyle::alignedRect(Qt::LeftToRight,Qt::AlignCenter,this->size(),qApp->desktop()->availableGeometry()));

    // связи
    connect(ui->btn_Exit,SIGNAL(clicked(bool)),this,SLOT(exit()));
    connect(ui->tw_FC,SIGNAL(itemSelectionChanged()),this,SLOT(fill_selected()));
    connect(ui->btn_Add,SIGNAL(clicked(bool)),this,SLOT(add()));
    connect(ui->btn_Delete,SIGNAL(clicked(bool)),this,SLOT(del()));
    connect(ui->btn_Edit,SIGNAL(clicked(bool)),this,SLOT(edit()));
    connect(ui->le_Name_Filter,SIGNAL(textEdited(QString)),this,SLOT(filter()));
    connect(ui->tabWidget,SIGNAL(currentChanged(int)),this,SLOT(save_selection()));

    // Параметры таблицы
    ui->tw_FC->setColumnCount(1); // число столбцов
    ui->tw_FC->setAutoScroll(true); // скрол
    // Выделение только одной строки
    ui->tw_FC->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tw_FC->setSelectionBehavior(QAbstractItemView::SelectRows);
    // Заголовки
    ui->tw_FC->setHorizontalHeaderItem(0,new QTableWidgetItem("Название клуба"));
    // Растягиваем последний столбец при растягивании формы
    ui->tw_FC->horizontalHeader()->setStretchLastSection(true);
    // Запрет на изменение
    ui->tw_FC->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

FootballClubs::~FootballClubs()
{
    delete ui;
}

void FootballClubs::exit()
{
    this->close();
    emit go_back();
}

void FootballClubs::this_open()
{
    // Очистка фильтра
    ui->le_Name_Filter->clear();
    // Заполнение таблицы
    selectAll();
    this->show();
}

void FootballClubs::selectAll()
{
    ui->tw_FC->clearContents();
    // Сортировка выкл
    ui->tw_FC->setSortingEnabled(false);

    if(!MainWindow::dbconn.isOpen())
    {
        QMessageBox::critical(this,"Error",MainWindow::dbconn.lastError().text());
        return;
    }

    QSqlQuery query(MainWindow::dbconn); // объект запроса с привязкой к соединению
    QString sqlstr = "select * from football_clubs"; // строка запроса
    if(!query.exec(sqlstr))
    {
        QMessageBox::critical(this,"Error",query.lastError().text());
        return;
    }

    if(query.isActive())
        ui->tw_FC->setRowCount(query.size());
    else
        ui->tw_FC->setRowCount(0);

    // Запись данных в таблицу
    int i = 0;
    while (query.next())
    {
        ui->tw_FC->setItem(i,0,new QTableWidgetItem(query.value("fc_name").toString()));
        i++;
    }
    // Размер столбца
    ui->tw_FC->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    // Сортировка вкл
    ui->tw_FC->setSortingEnabled(true);
    // Очистить поля ввода
    ui->le_Name->clear();
}

void FootballClubs::fill_selected()
{
    int cur_row = ui->tw_FC->currentRow(); // номер выбранной строки
    if (cur_row < 0) return;
    ui->le_Name->setText(ui->tw_FC->item(cur_row,0)->text());
}

void FootballClubs::add()
{
    if(!MainWindow::dbconn.isOpen())
    {
        QMessageBox::critical(this,"Error",MainWindow::dbconn.lastError().text());
        return;
    }

    QSqlQuery query(MainWindow::dbconn); // объект запроса с привязкой к соединению
    QString sqlstr = "insert into football_clubs(fc_name) values(?)"; // строка запроса
    query.prepare(sqlstr); // подготовка запроса

    // передаем параметры в запрос
    if (ui->le_Name->text() == "")
    {
        QMessageBox::critical(this,"Error","Name field is empty!");
        return;
    }
    query.bindValue(0,ui->le_Name->text());

    if(!query.exec())
    {
        QMessageBox::critical(this,"Error",query.lastError().text());
        return;
    }
    QMessageBox::about(this,"Success",QString("INSERT 0 %1").arg(query.numRowsAffected()));

    selectAll(); // обновляем таблицу
}

void FootballClubs::del()
{
    if(!MainWindow::dbconn.isOpen())
    {
        QMessageBox::critical(this,"Error",MainWindow::dbconn.lastError().text());
        return;
    }

    int cur_row = ui->tw_FC->currentRow(); // номер выбранной строки
    if(cur_row < 0) // не выбрана
    {
        QMessageBox::critical(this,"Error","Row is not selected!");
        return;
    }

    // Подтверждение удаления
    if(QMessageBox::question(this,"Delete","Delete selected row?",QMessageBox::Cancel,QMessageBox::Ok)==QMessageBox::Cancel)
        return;

    QSqlQuery query(MainWindow::dbconn);
    QString sqlstr = "delete from football_clubs where fc_name='" + ui->tw_FC->item(cur_row,0)->text() + "'";

    if(!query.exec(sqlstr))
    {
        QMessageBox::critical(this,"Error",query.lastError().text());
        return;
    }
    QMessageBox::about(this,"Success",QString("DELETE %1").arg(query.numRowsAffected()));

    selectAll(); // обновляем таблицу
}

void FootballClubs::edit()
{
    if(!MainWindow::dbconn.isOpen())
    {
        QMessageBox::critical(this,"Error",MainWindow::dbconn.lastError().text());
        return;
    }

    int cur_row = ui->tw_FC->currentRow(); // номер выбранной строки
    if(cur_row < 0) // не выбрана
    {
        QMessageBox::critical(this,"Error","Row is not selected!");
        return;
    }

    QSqlQuery query(MainWindow::dbconn);
    QString sqlstr = "update football_clubs set fc_name = ? where fc_name='" + ui->tw_FC->item(cur_row,0)->text() + "'";
    query.prepare(sqlstr);

    if (ui->le_Name->text() == "")
    {
        QMessageBox::critical(this,"Error","Name field is empty!");
        return;
    }
    query.bindValue(0,ui->le_Name->text());

    if(!query.exec())
    {
        QMessageBox::critical(this,"Error",query.lastError().text());
        return;
    }
    QMessageBox::about(this,"Success",QString("UPDATE %1").arg(query.numRowsAffected()));

    selectAll(); // обновляем таблицу
}

void FootballClubs::filter()
{
    QString value = ui->le_Name_Filter->text();
    if (value == "")
    {
        selectAll();
        return;
    }
    ui->tw_FC->clearContents();
    // Сортировка выкл
    ui->tw_FC->setSortingEnabled(false);

    QSqlQuery query(MainWindow::dbconn); // объект запроса с привязкой к соединению
    QString sqlstr = "select * from football_clubs where fc_name like '" + value + "%'"; // строка запроса

    if(!query.exec(sqlstr))
    {
        QMessageBox::critical(this,"Error",query.lastError().text());
        return;
    }

    if(query.isActive())
        ui->tw_FC->setRowCount(query.size());
    else
        ui->tw_FC->setRowCount(0);

    // Запись данных в таблицу
    int i = 0;
    while (query.next())
    {
        ui->tw_FC->setItem(i,0,new QTableWidgetItem(query.value("fc_name").toString()));
        i++;
    }
    // Размер столбца
    ui->tw_FC->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    // Сортировка вкл
    ui->tw_FC->setSortingEnabled(true);
}

void FootballClubs::save_selection()
{
    ui->tw_FC->setCurrentCell(ui->tw_FC->currentRow(),0);
}
