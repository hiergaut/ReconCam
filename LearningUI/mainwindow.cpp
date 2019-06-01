#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QtDebug>
#include "QStyledItemDelegateThumbnail.h"
#include <QStandardItem>
#include "QToolTipper.h"
#include "QToolTipperKnown.h"
#include "global.h"

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // --------------------------- NEW EVENT
    model_newEvent = new QFileSystemModel(this);
    model_newEvent->setRootPath(str_newEventDir);
    QStringList filter;
    filter << "*.jpg";
    model_newEvent->setNameFilterDisables(0);
    model_newEvent->setNameFilters(filter);

    ui->listView_newEvent->setViewMode(QListView::IconMode);
    ui->listView_newEvent->setMovement(QListView::Static);
    ui->listView_newEvent->setModel(model_newEvent);
    ui->listView_newEvent->setRootIndex(model_newEvent->index(str_newEventDir));
    ui->listView_newEvent->setItemDelegate(new QStyledItemDelegateThumbnail(model_newEvent, str_newEventDir, ui->listView_newEvent));
    ui->listView_newEvent->viewport()->installEventFilter(new QToolTipper(ui->listView_newEvent));
//    ui->listView_newEvent->viewport()->setToolTipDuration(10);

    // --------------------------- KNOWN
    model_known = new QFileSystemModel(this);
    model_known->setNameFilterDisables(0);
    model_known->setRootPath(str_knownDir);
    model_known->setFilter(QDir::Dirs | QDir::AllEntries);
    model_known->setNameFilters(filter);

//    qDebug() << model_known->rowCount() << model_known->columnCount();

    ui->listView_known->setViewMode(QListView::IconMode);
    ui->listView_known->setMovement(QListView::Static);
    ui->listView_known->setModel(model_known);
    ui->listView_known->setRootIndex(model_known->index(str_knownDir));
    ui->listView_known->setItemDelegate(new QStyledItemDelegateThumbnail(model_known, str_knownDir, ui->listView_known));
    ui->listView_known->viewport()->installEventFilter(new QToolTipperKnown(ui->listView_known, ui->listView_knownEvent, model_known));

//    ui->listView_known->viewport()->setBackgroundRole(QPalette::Dark);


    // --------------------------- KNOWN EVENT
    ui->listView_knownEvent->setModel(model_known);
//    ui->listView_knownEvent->setRootIndex(model_known->index(str_knownDir + "gauthier/"));


}

MainWindow::~MainWindow()
{
    delete ui;
}
