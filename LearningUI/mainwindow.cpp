#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "QListViewKnown.h"
#include "QListViewKnownEvent.h"
#include "QStyledItemDelegateKnown.h"
#include "QStyledItemDelegateThumbnail.h"
#include "QStyledItemDelegateThumbnailDown.h"
#include "QStyleditemDelegateOpengl.h"
#include "QToolTipper.h"
#include "QToolTipperKnown.h"
#include "global.h"
#include <QInputDialog>
#include <QListViewNewEvent.h>
#include <QModelIndex>
#include <QStandardItem>
#include <QtDebug>
//#include <map>

//void expandChildren(const QModelIndex &index, QTreeView *view)
//{
//    if (!index.isValid()) {
//        return;
//    }

//    int childCount = view->model()->rowCount(index);
//    qDebug() << "childCount = " << childCount;
//    for (int i = 0; i < childCount; i++) {
//        const QModelIndex &child = index.child(i, 0);
//        // Recursively call the function for each child node.
//        expandChildren(child, view);
//    }

//    if (!view->isExpanded(index)) {
////        view->expand(index);
//        view->setExpanded(index, true);
//        qDebug() << "expand " << view->model()->data(index);
//    }
//}

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_colors[str_newEventDir] = std::vector<float>({ 0.4, 0.4, 0.4 });

    QDir dir(str_knownDir);
    for (QString known : dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
        QFile file (str_knownDir + known + "/");
        Q_ASSERT(file.exists());
        newHashKnownDir(known);
    }

    _model = new QFileSystemModel(this);
    //    _model->setRootPath(str_learningRootDir);
    _model->setRootPath(str_learningRootDir);
    //    QStringList filter;
    //    filter << "*.jpg";
    //    _model->setNameFilterDisables(0);
    _model->setFilter(QDir::AllDirs | QDir::NoDotAndDotDot);
    //    _model->setNameFilters(filter);

    //    ui->treeView->setItemsExpandable(false);
    //    ui->treeView->expandToDepth(1);
    //    ui->treeView->setModel(_model);
    //    ui->treeView->setRootIndex(_model->index(str_learningRootDir));

    //    ui->treeView->expandToDepth(0);
    //    ui->treeView->setItemDelegate(new QStyledItemDelegateOpengl(_model, ui->treeView));
    //    expandChildren(_model->index(str_learningRootDir + "known/"), ui->treeView);
    //    ui->treeView->setExpanded(_model->index(str_learningRootDir + "known/"), true);
    //    ui->treeView->setExpanded(_model->index(str_learningRootDir + "newEvent/"), true);
    //    ui->treeView->setRootIsDecorated(true);
    //    ui->treeView->expandAll();
    //    ui->treeView->expandRecursively(_model->index(str_learningRootDir));
    //    ui->treeView->expandAll();

    connect(_model, &QFileSystemModel::layoutChanged, this, &MainWindow::on_modelChanged);
    //    connect(_model, &QFileSystemModel::directoryLoaded, this, &MainWindow::on_modelChanged);

    // --------------------------- NEW EVENT
    //    ui->listView_newEvent->setViewMode(QListView::IconMode);
    //    ui->listView_newEvent->setMovement(QListView::Static);
    ui->listView_newEvent->setModel(_model);
    ui->listView_newEvent->setRootIndex(_model->index(str_newEventDir));
    ui->listView_newEvent->setItemDelegate(new QStyledItemDelegateThumbnail(_model, ui->listView_newEvent));
    //    ui->listView_newEvent->setSelectionMode(QAbstractItemView::ExtendedSelection);
    connect(ui->listView_newEvent, &QListViewNewEvent::enterPressed, this, &MainWindow::on_moveNewEventSelectedToKnown);
    connect(ui->listView_newEvent, &QListViewNewEvent::deletePressed, this, &MainWindow::on_deleteNewEventSelected);

    //    ui->listView_newEvent->setDragDropMode()
    //    ui->listView_newEvent->selectAll();
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

    //    ui->listView_known->setViewMode(QListView::IconMode);
    //    ui->listView_known->setMovement(QListView::Static);
    ui->listView_known->setModel(_model);
    ui->listView_known->setRootIndex(_model->index(str_knownDir));
    ui->listView_known->setItemDelegate(new QStyledItemDelegateKnown(_model, ui->listView_knownEvent, m_colors, ui->listView_known));

    connect(ui->listView_known, &QListViewKnown::deletePressed, this, &MainWindow::on_deleteKnownSelected);
    connect(ui->listView_known->selectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)), this, SLOT(on_changeKnownSelected(QItemSelection)));

    //    ui->listView_known->viewport()->installEventFilter(new QToolTipperKnown(ui->listView_known, ui->listView_knownEvent, _model));

    //    ui->listView_known->viewport()->setBackgroundRole(QPalette::Dark);

    // --------------------------- KNOWN EVENT
    //    ui->listView_knownEvent->setViewMode(QListView::IconMode);
    //    ui->listView_knownEvent->setMovement(QListView::Static);
    ui->listView_knownEvent->setModel(_model);
    ui->listView_knownEvent->setRootIndex(_model->index(str_knownDir));
//    ui->listView_knownEvent->setItemDelegate(new QStyledItemDelegateThumbnailDown(_model, ui->listView_knownEvent, ui->listView_knownEvent));
    ui->listView_knownEvent->setLabel(ui->label_hist);
    ui->listView_knownEvent->setKnown(ui->listView_known);
    ui->listView_knownEvent->setText(ui->textEdit_hist);

    connect(ui->listView_knownEvent, &QListViewKnownEvent::spacePressed, this, &MainWindow::on_moveKnownEventSelectedToNewEvent);
    connect(ui->listView_knownEvent, &QListViewKnownEvent::deletePressed, this, &MainWindow::on_deleteKnownEventSelected);

    //    ui->listView_knownEvent->setRootIndex(_model->index(str_knownDir + "gauthier/"));

    //    ui->listView_knownEvent->setSelectionMode(QAbstractItemView::ExtendedSelection);
    //    ui->listView_knownEvent->setDragEnabled(true);
    //    ui->listView_knownEvent->setAcceptDrops(true);
    //    ui->listView_knownEvent->viewport()->setAcceptDrops(true);
    //    ui->listView_knownEvent->setDropIndicatorShown(true);
    ////    ui->listView_knownEvent->showDropIndicator()
    //    ui->listView_hist->setSelectionModel(ui->listView_knownEvent->selectionModel());
    //    ui->listView_knownEvent->setSelectionModel(ui->listView_hist->selectionModel());

    //    _model->directoryLoaded(str_knownDir + "gauthier/");
    //    _model->setRootPath(str_learningRootDir);
    //    ui->listView_knownEvent->viewport()->installEventFilter(new QToolTipper(ui->listView_knownEvent));
    //    ui->treeView->setSelectionModel(ui->listView_newEvent->selectionModel());
    //        ui->treeView->setSelectionModel(ui->listView_knownEvent->selectionModel());
    //    ui->listView_newEvent->setSelectionModel(ui->treeView->selectionModel());
    //    ui->listView_knownEvent->setSelectionModel(ui->treeView->selectionModel());
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_up_clicked()
{
    on_moveKnownEventSelectedToNewEvent();
}

void MainWindow::on_pushButton_down_clicked()
{
    on_moveNewEventSelectedToKnown();
}

void MainWindow::on_pushButton_deleteAllNewEvent_clicked()
{
    QDir newEventDir(str_newEventDir);
    newEventDir.setFilter(QDir::Files | QDir::NoDotAndDotDot);
    for (const QString& file : newEventDir.entryList()) {
        newEventDir.remove(file);
    }
}

void MainWindow::on_moveNewEventSelectedToKnown()
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
        knownEventDir = QInputDialog::getText(this, "new recon", "New object name :", QLineEdit::Normal, "", &ok);

        //        qDebug() << "knownEventDir = " << knownEventDir;

        if (ok && !knownEventDir.isEmpty()) {
            knownEventDir += "/";
            QString path = str_knownDir + knownEventDir;
            newHashKnownDir(knownEventDir);

            QDir dir(path);
            if (!dir.exists()) {
                dir.mkpath(".");
            }
            //            ui->listView_knownEvent->setRootIndex(_model->index(str_knownDir + knownEventDir));
            ui->listView_known->selectionModel()->select(_model->index(str_knownDir + knownEventDir), QItemSelectionModel::Select);
            //            ui->listView_knownEvent->update();
        } else {
            return;
        }
    } else {
        knownEventDir = _model->data(indexKnownDir.first()).toString() + "/";
    }

    for (const auto& index : newEventSelected) {
        QString filename = _model->data(index).toString();
        QFile newEvent(str_newEventDir + filename);
        newEvent.rename(str_knownDir + knownEventDir + filename);
    }

    updateKnownBestPicture();
    //        QPixmap best;
    //        QString path_best = str_knownDir + knownEventDir + "best.jpg";
    //        if (QFile::exists(path_best)) {
    //            best.load(path_best);
    //        } else {
    //            best.load(str_newEventDir + _model->data(newEventSelected.first()).toString() + "/image.jpg");
    //        }
    //        for (const auto& index : newEventSelected) {
    //            QString filename = _model->data(index).toString();
    //            QFile newEvent(str_newEventDir + filename);
    //            Q_ASSERT(newEvent.exists());
    //            QPixmap cur(str_newEventDir + filename + "/image.jpg");
    //            if (cur.width() + cur.height() > best.width() + best.height()) {
    //                best = cur;
    //            }
    //            //        qDebug() << newEvent;
    //            //        Q_ASSERT(newEvent.rename(str_knownDir + knownEventDir + filename));
    //            //        QString newName = str_knownDir + knownEventDir + filename;
    //            //        newEvent.open(QIODevice::NewOnly);
    //            //        newEvent.open(Q)
    //            newEvent.rename(str_knownDir + knownEventDir + filename);
    //            //        newEvent.close();
    //            //        newEvent.copy(newName);
    //            //        newEvent.deleteLater();

    //            //        _model->fileRenamed(str_learningRootDir, str_newEventDir + filename, str_knownDir + knownEventDir + filename);
    //            //        newEvent.remove();
    //            //        newEventSelected.append(_model->data(index).toString());
    //        }

    //        best.save(str_knownDir + knownEventDir + "best.jpg");
}

void MainWindow::on_deleteNewEventSelected()
{
    QModelIndexList newEventSelected = ui->listView_newEvent->selectionModel()->selectedIndexes();
    for (const auto& index : newEventSelected) {
        QString filename = _model->data(index).toString();
        QFile newEvent(str_newEventDir + filename);
        Q_ASSERT(newEvent.exists());
        if (newEvent.remove()) {
            qDebug() << "cannot remove new event selected file : " << filename;
        }
    }
}

void MainWindow::on_deleteKnownSelected()
{
    QModelIndexList newEventSelected = ui->listView_known->selectionModel()->selectedIndexes();
    if (newEventSelected.empty()) {
        return;
    }
//    Q_ASSERT(newEventSelected.size() == 1);

    QModelIndex index = newEventSelected.first();
    QString filename = _model->data(index).toString();
    QDir knownDir(str_knownDir + filename);
    Q_ASSERT(knownDir.exists());
    if (!knownDir.removeRecursively()) {
        qDebug() << "unable to remove dir : " << filename;
    }
}

void MainWindow::on_changeKnownSelected(QItemSelection item)
{
    QModelIndexList indexes = item.indexes();
    //    Q_ASSERT(indexes.size() == 1);
    if (indexes.size() == 0) {
        qDebug() << "no change selected known dir";
        ui->listView_knownEvent->setRootIndex(_model->index(str_learningRootDir));
        return;
    }
    QModelIndex index = indexes.first();
    QString dir = str_knownDir + _model->data(index).toString() + "/";
    ui->listView_knownEvent->setRootIndex(_model->index(dir));
}

void MainWindow::on_moveKnownEventSelectedToNewEvent()
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
        //        qDebug() << "move " << filename;
    }

    updateKnownBestPicture();
    //    qDebug() << knownSelected;
}

void MainWindow::on_deleteKnownEventSelected()
{
    const QModelIndexList& indexKnownDir = ui->listView_known->selectionModel()->selectedIndexes();
    if (indexKnownDir.size() == 0) {
        qDebug() << "no knownDir selected";
        return;
    }
    QString knownEventDir = _model->data(indexKnownDir.first()).toString() + "/";

    QModelIndexList knownEventSelected = ui->listView_knownEvent->selectionModel()->selectedIndexes();
    for (const auto& index : knownEventSelected) {
        QString filename = _model->data(index).toString();
        QFile newEvent(str_knownDir + knownEventDir + filename);
        Q_ASSERT(newEvent.exists());
        if (newEvent.remove()) {
            qDebug() << "cannot remove new event selected file : " << filename;
        }
    }
    updateKnownBestPicture();
}

void MainWindow::on_modelChanged()
{
    //    using vec3 = std::vector<float>;
    std::vector<float> points;
    //    std::vector<float> color(3);
    std::vector<float> color;
//    std::vector<float> point;
    //    std::vector<float> point(3);
    //    std::map<std::string, std::vector<float>> colors;
    //    colors[str_newEventDir.toStdString()] = std::vector<float>({0, 0, 0});
    //    std::map<std::string, std::vector<float>> colors;
    //    colors[str_newEventDir.toStdString()] = std::vector<float>({0, 0, 0});

//    qDebug() << "[MainWindow::on_modelChanged] model changed ";
    //    QStringList pathDirs = { str_newEventDir };

    //    QDir knowDir(str_knownDir);
    //    for (QString known : knowDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
    //        qDebug() << known;
    //        QString path = str_knownDir + known + "/";
    //        pathDirs += path;
    ////        colors[path.toStdString()] = std::vector<float>({1, 1, 1});
    //    }

    for (const auto& pair : m_colors) {
        //    for (QString pathDir : pathDirs) {
        QString pathDir = pair.first;
        color = pair.second;
        Q_ASSERT(color.size() == 3);

        QDir dir(pathDir);
        Q_ASSERT(dir.exists());
        dir.setFilter(QDir::AllDirs | QDir::NoDotAndDotDot);

        //        QModelIndex index = _model->index(pathDir);
        //        qDebug() << _model->data(index, Qt::DecorationRole);

        for (const QString& dirName : dir.entryList()) {
            QFile file(pathDir + dirName + "/primary.txt");
            if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                qDebug() << "cannot open file : " << file;
            }

            QTextStream in(&file);
            for (int i = 0; i < 3; ++i) {
                QString line = in.readLine();

                QRegExp rx("[ ]");
                QStringList list = line.split(rx);

                Q_ASSERT(list.size() == 3);
                float h = list[0].toFloat();
                Q_ASSERT(0.0f <= h && h <= 1.0f);
                //                Q_ASSERT(0.0f <= h && h <= 255.0f);
//                float s = list[1].toFloat();
//                Q_ASSERT(0.0f <= s && s <= 1.0f);
//                point[i] = h;
                points.push_back(h);
            }
//            points.insert(points.end(), point.begin(), point.end());
            points.insert(points.end(), color.begin(), color.end());

            file.close();
        }
    }
    Q_ASSERT(points.size() % 6 == 0);
    qDebug() << "[MainWindow::on_modelChanged] load nb points =" << points.size() / 6;

    ui->openGLWidget->setPoints(points);
}

void MainWindow::updateKnownBestPicture()
{
    const QString knownSelected = _model->data(ui->listView_known->selectionModel()->selectedIndexes().first()).toString() + "/";
    //    qDebug() << "knownSelected " << knownSelected;
    //    QString knownEventDir = _model->data(knownEventSelected.first()).toString();
    //    QString path_best = str_knownDir + knownEventDir + "best.jpg";
    QPixmap best;
    //    if (QFile::exists(path_best)) {
    //        best.load(path_best);
    //    } else {
    //        best.load(str_newEventDir + _model->data(newEventSelected.first()).toString() + "/image.jpg");
    //    }
    QDir knownDir(str_knownDir + knownSelected);
    knownDir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QString& file : knownDir.entryList()) {
        //        qDebug() << "file " << file;
        //        QString filename = _model->data(index).toString();
        QString path = str_knownDir + knownSelected + file + "/image.jpg";
        if (!QFile::exists(path)) {
            qDebug() << "[MainWindow::updateKnownBestPicture] : image file not exist : " << path;
        }
        QPixmap cur = QPixmap(path);

        if (cur.width() * cur.height() > best.width() * best.height()) {
            best = cur;
        }
    }

    //    QString knownEventDir = _model->data(knownEventSelected.first()).toString() + "/";
    best.save(str_knownDir + knownSelected + "best.jpg");
}

void MainWindow::newHashKnownDir(QString filename)
{
    qDebug() << "[MainWindow::newHashKnownDir] filename : " << filename;
    auto hash = std::hash<std::string> {}(filename.toStdString());
    float r = (hash % 256) / 256.0f;
    float g = ((hash / 1000) % 256) / 256.0f;
    float b = ((hash / 1000000) % 256) / 256.0f;

//    QString path = str_knownDir + filename + "/";
//    QFile dir (path);
//    Q_ASSERT(dir.exists());
    m_colors[str_knownDir + filename + "/"] = std::vector<float>({ r, g, b });
}
