#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "QFileSystemModelDatum.h"
#include <QFileSystemModel>
#include <QItemSelection>
#include <QStandardItemModel>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

    QVector3D vec3ReadFromBig(QString path);
    QVector3D vec3Read(QString path);

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

private:
    void updateKnownBestPicture();
    void newHashKnownDir(QString path);
    void on_modelChanged();

private:
    Ui::MainWindow* ui;

    //    QFileSystemModel* model_newEvent;
    //    QFileSystemModel* model_known;
    QFileSystemModel* _model;
    std::map<QString, QColor> m_colors;

    //    QString str_learningRootDir { QDir::currentPath() + "/../learningFile/" };
    //    QString str_newEventDir { str_learningRootDir + "newEvent/" };
    //    QString str_knownDir { str_learningRootDir + "known/" };
};

#endif // MAINWINDOW_H
