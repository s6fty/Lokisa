#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidgetItem>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_actionAdd_folder_triggered();

    void on_listWidget_2_itemDoubleClicked(QListWidgetItem *item);

    void on_actionLoad_Database_triggered();

    void on_actionAdd_files_2_triggered();

    void on_pushButton_clicked();

    void on_lineEdit_2_textChanged(const QString &arg1);

    void on_listWidget_itemClicked(QListWidgetItem *item);

    void LoadTags();

    void on_pushButton_2_clicked();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
