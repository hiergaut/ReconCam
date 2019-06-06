#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QFileSystemModel>
#include <QStandardItemModel>
#include "QFileSystemModelDatum.h"
#include <QItemSelection>

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

    void on_pushButton_deleteAllNewEvent_clicked();

public slots:
    void on_moveNewEventSelectedToKnown();
    void on_deleteNewEventSelected();

    void on_deleteKnownSelected();
    void on_changeKnownSelected(QItemSelection index);

    void on_moveKnownEventSelectedToNewEvent();
    void on_deleteKnownEventSelected();

    void on_modelChanged();

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
