#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "QStyledItemDelegateKnown.h"
#include "QStyledItemDelegateThumbnail.h"
#include "QStyledItemDelegateThumbnailDown.h"
#include "QToolTipper.h"
#include "QToolTipperKnown.h"
#include "global.h"
#include <QInputDialog>
#include <QModelIndex>
#include <QStandardItem>
#include <QtDebug>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    _model = new QFileSystemModel(this);
    //    _model->setRootPath(str_learningRootDir);
    _model->setRootPath(str_learningRootDir);
    QStringList filter;
    filter << "*.jpg";
    _model->setNameFilterDisables(0);
    _model->setNameFilters(filter);

    // --------------------------- NEW EVENT
    ui->listView_newEvent->setViewMode(QListView::IconMode);
    ui->listView_newEvent->setMovement(QListView::Static);
    ui->listView_newEvent->setModel(_model);
    ui->listView_newEvent->setRootIndex(_model->index(str_newEventDir));
    ui->listView_newEvent->setItemDelegate(new QStyledItemDelegateThumbnail(_model, ui->listView_newEvent));
    //    ui->listView_newEvent->setDragDropMode()
    //    ui->listView_newEvent->selectAll();
    ui->listView_newEvent->setSelectionMode(QAbstractItemView::ExtendedSelection);
    //    ui->listView_newEvent->setDragEnabled(true);
    //    ui->listView_newEvent->setAcceptDrops(true);
    //    ui->listView_newEvent->viewport()->setAcceptDrops(true);
    //    ui->listView_newEvent->setDropIndicatorShown(true);
    //    ui->listView_newEvent->SelectRows;
    //    ui->listView_newEvent->viewport()->installEventFilter(new QToolTipper(ui->listView_newEvent));
    //    ui->listView_newEvent->viewport()->setToolTipDuration(10);

    // --------------------------- KNOWN
    //    _model = new QFileSystemModel(this);
    //    //    _model->setNameFilterDisables(0);
    //    _model->setRootPath(str_knownDir);
    //    _model->setFilter(QDir::AllDirs | QDir::NoDotAndDotDot | QDir::AllEntries);
    //    _model->setNameFilters(filter);

    //    qDebug() << _model->rowCount() << _model->columnCount();

    ui->listView_known->setViewMode(QListView::IconMode);
    ui->listView_known->setMovement(QListView::Static);
    ui->listView_known->setModel(_model);
    ui->listView_known->setRootIndex(_model->index(str_knownDir));
    ui->listView_known->setItemDelegate(new QStyledItemDelegateKnown(_model, ui->listView_knownEvent, ui->listView_known));
    //    ui->listView_known->viewport()->installEventFilter(new QToolTipperKnown(ui->listView_known, ui->listView_knownEvent, _model));

    //    ui->listView_known->viewport()->setBackgroundRole(QPalette::Dark);

    // --------------------------- KNOWN EVENT
    ui->listView_knownEvent->setViewMode(QListView::IconMode);
    ui->listView_knownEvent->setMovement(QListView::Static);
    ui->listView_knownEvent->setModel(_model);
    ui->listView_knownEvent->setItemDelegate(new QStyledItemDelegateThumbnailDown(_model, ui->listView_knownEvent, ui->listView_knownEvent));
    //    ui->listView_knownEvent->setRootIndex(_model->index(str_knownDir + "gauthier/"));

    ui->listView_knownEvent->setSelectionMode(QAbstractItemView::ExtendedSelection);
    //    ui->listView_knownEvent->setDragEnabled(true);
    //    ui->listView_knownEvent->setAcceptDrops(true);
    //    ui->listView_knownEvent->viewport()->setAcceptDrops(true);
    //    ui->listView_knownEvent->setDropIndicatorShown(true);
    ////    ui->listView_knownEvent->showDropIndicator()

    //    _model->directoryLoaded(str_knownDir + "gauthier/");
    //    _model->setRootPath(str_learningRootDir);
    //    ui->listView_knownEvent->viewport()->installEventFilter(new QToolTipper(ui->listView_knownEvent));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_up_clicked()
{
    const QModelIndexList& indexKnownDir = ui->listView_known->selectionModel()->selectedIndexes();
    //    Q_ASSERT(indexKnownDir.size() == 1);
    if (indexKnownDir.size() == 0) {
        qDebug() << "no knownDir selected";
        return;
    }
    Q_ASSERT(indexKnownDir.size() == 1);
    QString knownEventDir = _model->data(indexKnownDir.first()).toString() + "/";

    QStringList knownSelected;
    for (const auto& index : ui->listView_knownEvent->selectionModel()->selectedIndexes()) {
        QString filename = _model->data(index).toString();
        QFile known(str_knownDir + knownEventDir + filename);
        Q_ASSERT(known.exists());
        //        qDebug() << known;
        if (!known.rename(str_newEventDir + filename)) {
            qDebug() << "file already exist, can't move " << filename;
        }
        //        knownSelected.append(_model->data(index).toString());
        qDebug() << "move " << filename;
    }
    //    qDebug() << knownSelected;
}

void MainWindow::on_pushButton_down_clicked()
{
    QModelIndexList newEventSelected = ui->listView_newEvent->selectionModel()->selectedIndexes();
    if (newEventSelected.size() == 0) {
        qDebug() << "no selected event";
        return;
    }

    const QModelIndexList& indexKnownDir = ui->listView_known->selectionModel()->selectedIndexes();
    //    Q_ASSERT(indexKnownDir.size() == 1);
    QString knownEventDir;
    if (indexKnownDir.size() == 0) {
        bool ok;
        knownEventDir = QInputDialog::getText(this, "new recon", "New object name :", QLineEdit::Normal, "", &ok) + "/";

        if (ok && !knownEventDir.isEmpty()) {
            QDir dir(str_knownDir + knownEventDir);
            if (!dir.exists()) {
                dir.mkpath(".");
            }
            ui->listView_knownEvent->setRootIndex(_model->index(str_knownDir + knownEventDir));
//            ui->listView_knownEvent->update();
        } else {
            return;
        }
    } else {
        knownEventDir = _model->data(indexKnownDir.first()).toString() + "/";
    }

    QPixmap best;
    QString path_best = str_knownDir + knownEventDir + "best.jpeg";
    if (QFile::exists(path_best)) {
        best.load(path_best);
    } else {
        best.load(str_newEventDir + _model->data(newEventSelected.first()).toString());
    }
    for (const auto& index : newEventSelected) {
        QString filename = _model->data(index).toString();
        QFile newEvent(str_newEventDir + filename);
        Q_ASSERT(newEvent.exists());
        QPixmap cur(str_newEventDir + filename);
        if (cur.width() + cur.height() > best.width() + best.height()) {
            best = cur;
        }
//        qDebug() << newEvent;
        //        Q_ASSERT(newEvent.rename(str_knownDir + knownEventDir + filename));
        //        QString newName = str_knownDir + knownEventDir + filename;
        //        newEvent.open(QIODevice::NewOnly);
        //        newEvent.open(Q)
        newEvent.rename(str_knownDir + knownEventDir + filename);
        //        newEvent.close();
        //        newEvent.copy(newName);
        //        newEvent.deleteLater();

        //        _model->fileRenamed(str_learningRootDir, str_newEventDir + filename, str_knownDir + knownEventDir + filename);
        //        newEvent.remove();
        //        newEventSelected.append(_model->data(index).toString());
    }

    best.save(str_knownDir + knownEventDir + "best.jpeg");
}
