#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QFileSystemModel>
#include <QStandardItemModel>
#include "QFileSystemModelDatum.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void on_pushButton_up_clicked();

    void on_pushButton_down_clicked();

private:
    Ui::MainWindow* ui;

//    QFileSystemModel* model_newEvent;
//    QFileSystemModel* model_known;
    QFileSystemModel* _model;


//    QString str_learningRootDir { QDir::currentPath() + "/../learningFile/" };
//    QString str_newEventDir { str_learningRootDir + "newEvent/" };
//    QString str_knownDir { str_learningRootDir + "known/" };
};

#endif // MAINWINDOW_H
