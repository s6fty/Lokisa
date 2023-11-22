#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <iostream>
#include <QFileDialog>
#include <QString>
#include <QDebug>
#include <QDir>
#include <QListWidget>
#include <QListWidgetItem>
#include <QIcon>
#include <QtSql>
#include <QSqlDatabase>
#include <QTextStream>
#include <QInputDialog>
#include <QDesktopServices>


using namespace std;

// public variables

class DatabaseOperations{

public : void DatabaseConnection(QString IO){
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE"); // using sqlite as backend
        QDir databasePath;
        QString path = databasePath.currentPath()+"/myDB.db";   // setting database path as the current path because it made sense
        // create database
        db.setHostName("Lokisa");
        db.setDatabaseName(path);
        db.setUserName("LokisaUser");
        db.setPassword("Lokisa");   // maybe in the future i'll add something to change this but for now it will stay like this

        if (IO.toLower() == "open")
            db.open();

        else if (IO.toLower() == "close")
            db.close();


    };

public: void AddToDatabase(QString FilePath, QString FileName)
    {
        QSqlQuery query;
        bool ifDbExists = query.exec("SELECT * FROM Files WHERE id=1");

        if (!ifDbExists)
        {
            query.prepare("CREATE TABLE IF NOT EXISTS Files(ID INTEGER PRIMARY KEY, filePath TEXT, fileName TEXT)");    // create table for newly created databases
            if (!query.exec()) {
                qDebug() << "Query execution error:" << query.lastError().text();
            } else {
                qDebug() << "Table created or already exists.";
            }
        }
        else
        {
            query.prepare("SELECT * FROM Files WHERE filePath = ?");    // do i need to explain?
            query.addBindValue(FilePath);
            query.exec();

            query.prepare("INSERT INTO Files(filePath, fileName) VALUES(?, ?)");
            query.addBindValue(FilePath);
            query.addBindValue(FileName);
            query.exec();
        }
    }
};



MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    DatabaseOperations().DatabaseConnection("Open");
}


MainWindow::~MainWindow()
{
    delete ui;
    DatabaseOperations().DatabaseConnection("Close");
}

void MainWindow::on_actionAdd_folder_triggered()
{
    QDir folderName;
    folderName = QFileDialog::getExistingDirectory(this, tr("Open Directory"),"",QFileDialog::ShowDirsOnly);    // it says deprecated maybe change in future
    QStringList images = folderName.entryList(QStringList() << "*.jpg" << "*.JPG" << "*.jpeg" << "*.JPEG" << "*.png" << "*.PNG",QDir::Files); // only jpg, jpeg and png now webm and mp4 in future
    foreach (const QString file, images){
        QString Path = folderName.absolutePath().append("/" + file); // idk how to make this better it works so i wont touch that
        ui->listWidget_2->addItem(new QListWidgetItem(QIcon(Path), file));
        DatabaseOperations().AddToDatabase(Path, file);
        // plan is add everything to database while its loading. thats what we will work with. -- if someone can achieve this it will be very nice thanks
    }
}


void MainWindow::on_listWidget_2_itemDoubleClicked(QListWidgetItem *item)
{
    QString fileName = ui->listWidget_2->currentItem()->text();
    QSqlQuery query;
    query.prepare("SELECT * FROM Files WHERE filePath LIKE ?");
    query.addBindValue("%" + fileName + "%");
    qDebug() << query.exec();
    qDebug() << query.lastError();  // failing to get the filePath
    QDesktopServices::openUrl(QUrl::fromLocalFile(fileName));
}




void MainWindow::on_actionLoad_Database_triggered()
{
    QSqlQueryModel readData;

    readData.setQuery("SELECT * FROM Files");

    while (readData.canFetchMore())
        readData.fetchMore();

    for (int i = 0; i < readData.rowCount(); i++) {
        QString filePath = readData.record(i).field(1).value().toString();
        QString fileName = readData.record(i).field(2).value().toString();
        ui->listWidget_2->addItem(new QListWidgetItem(QIcon(filePath), fileName));
    }
}


