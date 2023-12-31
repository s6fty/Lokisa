#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QFileDialog>
#include <QString>
#include <QDebug>
#include <QDir>
#include <QListWidget>
#include <QListWidgetItem>
#include <QIcon>
#include <QtSql>
#include <QFile>
#include <QSqlDatabase>
#include <QTextStream>
#include <QInputDialog>
#include <QDesktopServices>
#include <QMessageBox>

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
            query.prepare("CREATE TABLE IF NOT EXISTS Files(ID INTEGER PRIMARY KEY, filePath TEXT, fileName TEXT, tags TEXT)");    // create table for newly created databases
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


void MainWindow::LoadTags() {
    QSqlQuery query;
    ui->listWidget->clear();
    query.exec("SELECT * FROM Files WHERE tags IS NOT NULL");
    while (query.next()) {
        QString tagsString = query.value("tags").toString();

        // Split the tags string into a list of individual tags
        QStringList tagsList = tagsString.split(" ", Qt::SkipEmptyParts);

        // Add each tag as a separate item to the listWidget
        for (const QString &tag : tagsList) {
            // Check if the tag already exists in the listWidget
            bool tagExists = false;
            for (int i = 0; i < ui->listWidget->count(); ++i) {
                QListWidgetItem *existingItem = ui->listWidget->item(i);
                if (existingItem && existingItem->text() == tag) {
                    tagExists = true;
                    break;
                }
            }

            // If the tag doesn't exist, add it to the listWidget
            if (!tagExists) {
                QListWidgetItem *item = new QListWidgetItem(tag);
                ui->listWidget->addItem(item);
            }
        }
    }
}



MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    DatabaseOperations().DatabaseConnection("Open");
    LoadTags();
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
    QStringList images = folderName.entryList(QStringList() << "*.jpg" << "*.JPG" << "*.jpeg" << "*.JPEG" << "*.png" << "*.PNG" << "*.mp4" << "*.MP4" << "*.webm" << "*.WEBM", QDir::Files); // only jpg, jpeg and png now webm and mp4 in future
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
    query.prepare("SELECT * FROM Files WHERE filePath LIKE :filename");
    query.bindValue(":filename", "%" + fileName + "%");

    if (query.exec()) {
        while (query.next()) {
            QString filePath = query.value("filePath").toString();
            qDebug() << "Found file path:" << filePath;
            QDesktopServices::openUrl(QUrl::fromLocalFile(filePath));
        }
    } else {
        qDebug() << "Query failed:" << query.lastError();
    }
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

        bool fileExists = false;

        for (int j; j < ui->listWidget_2->count(); j++){
            if (ui->listWidget_2->item(j)->text() == fileName)
            {
                fileExists = true;
                break;
            }
        }

        if (!fileExists)
            ui->listWidget_2->addItem(new QListWidgetItem(QIcon(filePath), fileName));
    }
}



void MainWindow::on_actionAdd_files_2_triggered()   // chatgpt generated code i cba
{
    QStringList selectedFiles = QFileDialog::getOpenFileNames(this, tr("Open file(s)"), "", tr("Image Files (*.jpg *.JPG *.jpeg *.JPEG *.png *.PNG);;All Files (*)"));

    foreach (const QString &filePath, selectedFiles) {
        QString fileName = QFileInfo(filePath).fileName();
        ui->listWidget_2->addItem(new QListWidgetItem(QIcon(filePath), fileName));
        DatabaseOperations().AddToDatabase(filePath, fileName);
    }
}


void MainWindow::on_pushButton_clicked()
{
    QString tag = ui->lineEdit->text();
    QString fileName = ui->listWidget_2->currentItem()->text();
    QSqlQuery query;

    query.prepare("SELECT * FROM Files WHERE fileName LIKE ?");
    query.addBindValue(fileName);

    if (query.exec()) {
        if (query.next()) {
            if (query.isNull("tags")) {
                query.prepare("UPDATE Files SET tags = ? WHERE fileName = ?");
            } else {
                query.prepare("UPDATE Files SET tags = tags || ' ' || ? WHERE fileName = ?");
            }

            query.addBindValue(tag);
            query.addBindValue(fileName);

            if (query.exec()) {
                qDebug() << "Update successful.";
            } else {
                qDebug() << "Update failed: " << query.lastError().text();
            }
        } else {
            qDebug() << "No matching file found.";
        }
    } else {
        qDebug() << "Query failed: " << query.lastError().text();
    }
    LoadTags();
}

void MainWindow::on_pushButton_2_clicked()
{
    QString selectedTag = ui->lineEdit_2->text();

    // Check if an item is selected in the listWidget
    QListWidgetItem* currentItem = ui->listWidget_2->currentItem();
    if (!currentItem) {
        QMessageBox::warning(this, "Warning", "No item selected!");
        return;
    }

    QString fileName = currentItem->text();

    QSqlQuery query;
    query.prepare("SELECT * FROM Files WHERE filePath LIKE ?");
    query.addBindValue("%" + fileName + "%");

    if (query.exec()) {
        if (query.next()) {
            QString tag = query.value(3).toString();

            // Check if the selected tag is attached before removing
            if (!tag.contains(selectedTag)) {
                QMessageBox::warning(this, "Warning", "The selected tag is not attached to this item!");
                return;
            }

            // Remove the selectedTag from the existing tags
            tag.remove(selectedTag);

            qDebug() << "Updated Tags:" << tag << "File Name:" << fileName;

            // Update the database with the modified tags
            QSqlQuery updateQuery;
            updateQuery.prepare("UPDATE Files SET TAGS = ? WHERE filePath LIKE ?");
            updateQuery.addBindValue(tag);
            updateQuery.addBindValue("%" + fileName + "%");

            if (updateQuery.exec()) {
                qDebug() << "Tags updated successfully!";
            } else {
                qDebug() << "Failed to update tags:" << updateQuery.lastError().text();
            }
        }
    }

    LoadTags();
}



void MainWindow::on_listWidget_itemDoubleClicked(QListWidgetItem *item)
{
    ui->listWidget_2->clear();
    QString tag = ui->listWidget->currentItem()->text();
    QSqlQuery query;
    query.prepare("SELECT * FROM Files WHERE tags LIKE ?");
    query.addBindValue("%" + tag + "%");

    if (!query.exec()) {
        qDebug() << "Query failed: " << query.lastError().text();
    } else {
        while (query.next()) {
            QString filePath = query.value("filePath").toString();
            QString fileName = query.value("fileName").toString();

            // check if a similar item already exists in widget
            bool duplicateItem = false;
            for (int i = 0; i < ui->listWidget_2->count(); ++i) {
                QListWidgetItem *existingItem = ui->listWidget_2->item(i);
                if (existingItem->text() == fileName) {
                    duplicateItem = true;
                    break;
                }
            }

            // if the item isnt a duplicate add it to widget
            if (!duplicateItem) {
                QListWidgetItem *newItem = new QListWidgetItem(QIcon(filePath), fileName);
                ui->listWidget_2->addItem(newItem);
            }
        }
    }
}


void MainWindow::on_lineEdit_2_editingFinished()
{
    QString tag = ui->lineEdit_2->text();
    QSqlQuery query;
    ui->listWidget_2->clear();

    if (tag.isEmpty()) {
        // If the line edit is empty, add all items back to the list
        query.prepare("SELECT * FROM Files");
    } else {
        // If the line edit is not empty, filter by tag
        query.prepare("SELECT * FROM Files WHERE tags LIKE ?");
        query.addBindValue("%" + tag + "%");
    }

    if (!query.exec()) {
        qDebug() << "Query failed: " << query.lastError().text();
    } else {
        while (query.next()) {
            QString filePath = query.value("filePath").toString();
            QString fileName = query.value("fileName").toString();

            // Ensure filePath is a valid path before using it as an icon
            if (!filePath.isEmpty() && QFile::exists(filePath)) {
                QListWidgetItem *item = new QListWidgetItem(QIcon(filePath), fileName);
                ui->listWidget_2->addItem(item);
            } else {
                qDebug() << "Invalid file path: " << filePath;
            }
        }
    }
}

void MainWindow::on_listWidget_2_customContextMenuRequested(const QPoint &pos)
{
    qDebug() << "Custom context menu requested.";
    // Get the item at the clicked position
    QListWidgetItem *item = ui->listWidget_2->itemAt(pos);

    if (item) {
        // Create a context menu
        QMenu contextMenu(this);

        // Add actions to the context menu
        QAction *deleteAction = new QAction("Delete", this);

        // Connect actions to slots
        connect(deleteAction, &QAction::triggered, this, &MainWindow::deleteSelectedItem);

        // Add actions to the context menu
        contextMenu.addAction(deleteAction);

        // Show the context menu at the cursor position
        contextMenu.exec(ui->listWidget_2->mapToGlobal(pos));
    }
}

void MainWindow::deleteSelectedItem()
{
    // Handle deleting the selected item
    QListWidgetItem *item = ui->listWidget_2->currentItem();
    if (item) {
        // Get information about the selected item
        QString fileName = item->text();

        // Remove the item from the listWidget
        int row = ui->listWidget_2->row(item);
        delete ui->listWidget_2->takeItem(row);

        // Delete the corresponding entry from the database
        QSqlQuery query;
        query.prepare("DELETE FROM Files WHERE fileName = ?");
        query.addBindValue(fileName);

        if (query.exec()) {
            qDebug() << "Item deleted from the database.";
        } else {
            qDebug() << "Failed to delete item from the database:" << query.lastError().text();
        }
    }
}


