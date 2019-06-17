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
#include "Identity.hpp"

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
//QVector3D MainWindow::vec3ReadFromBig(QString path)
//{
//    QVector3D vec3;

//    QFile file(path);

//    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
//        qDebug() << "cannot open file : " << file;
//    }

//    QTextStream in(&file);
//    for (int i = 0; i < 3; ++i) {
//        QString line = in.readLine();

//        const QRegExp rx("[ ]");
//        QStringList list = line.split(rx);

//        Q_ASSERT(list.size() == 3);
//        float h = list[0].toFloat();
//        Q_ASSERT(-1.0f <= h && h <= 1.0f);

//        vec3[i] = h;
//    }
//    file.close();
//    return vec3;
//}

//QVector3D MainWindow::vec3Read(QString path)
//{
//    QVector3D vec3;

//    QFile file(path);

//    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
//        qDebug() << "cannot open file : " << file;
//    }

//    QTextStream in(&file);
//    QString line = in.readLine();

//    const QRegExp rx("[ ]");
//    QStringList list = line.split(rx);
//    Q_ASSERT(list.size() == 3);

//    for (int i = 0; i < 3; ++i) {
//        float h = list[i].toFloat();
//        Q_ASSERT(-1.0f <= h && h <= 1.0f);

//        vec3[i] = h;
//    }
//    file.close();
//    return vec3;
//}

//void vec3Save(QVector3D vec3, QString path)
//{
//    QFile file(path);
//    file.open(QIODevice::WriteOnly);
//    QTextStream out(&file);

//    out << vec3.x() << " " << vec3.y() << " " << vec3.z() << "\n";
//    file.close();
//}

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QColor c(255, 255, 255, 70);
    //    c.setHsvF(1.0, 0.0, 1.0, 1.0);
    m_colors[str_newEventDir] = c;
    //    std::vector<float>({ 0.4f, 0.4f, 0.4f, 0.5f });

    QDir dir(str_knownDir);
    for (QString known : dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
        QFile file(str_knownDir + known + "/");
        Q_ASSERT(file.exists());
        newHashKnownDir(known);
    }

    QMatrix4x4 object;
    object.setToIdentity();
    //    object.rotate(180, QVector3D(1.0, 0, 0));
    object.scale(1.0, -1.0, 1.0);
    //    object.translate(-1, -1, 0);
    ui->openGLWidget_pos->setObject(object);
    QMatrix4x4 normalize;
    normalize.setToIdentity();
    normalize.translate(-1, +1, 0);
    normalize.scale(1.0f / 320.0f, 1.0f / 240.0f, 1.0f);
    ui->openGLWidget_pos->setNormalize(normalize);
    ui->openGLWidget_pos->setZCamera(-1.5);
    ui->openGLWidget_pos->setArea(m_square, m_squareEbo);
    //    ui->openGLWidget_pos->render(m_triangle);
    //    ui->openGLWidget_pos->render(m_triangle);
    //    ui->openGLWidget_pos->render(m_triangle);

    //    ui->openGLWidget_density->setZCamera(-1.5);
    //    normalize.rotate(90, QVector3D(1.0, 0, 0));
    object.setToIdentity();
    object.scale(1.0, 1.0, -1.0);
    //    object.translate(-1, -1, +1);
    ui->openGLWidget_density->setObject(object);
    normalize.setToIdentity();
    normalize.translate(-1, -1, +1);
    normalize.scale(2.0f / 640.0f, 2.0f / 480.0f, 10.0f / (640 * 480));
    //    normalize.scale(1.0, 1.0, -1.0);
    ui->openGLWidget_density->setNormalize(normalize);
    ui->openGLWidget_density->setArea(m_box, m_boxEbo);

    //    object.setToIdentity();
    //    object.translate(-1, -1, -1);
    ui->openGLWidget_first->setObject(object);
    normalize.setToIdentity();
    normalize.translate(-1, -1, +1);
    normalize.scale(2.0f / 256.0f, 2.0f / 256.0f, 2.0f / 256.0f);
    ui->openGLWidget_first->setNormalize(normalize);
    ui->openGLWidget_first->setArea(m_box, m_boxEbo);

    ui->openGLWidget_second->setObject(object);
    ui->openGLWidget_second->setNormalize(normalize);
    ui->openGLWidget_second->setArea(m_box, m_boxEbo);

    ui->openGLWidget_third->setObject(object);
    ui->openGLWidget_third->setNormalize(normalize);
    ui->openGLWidget_third->setArea(m_box, m_boxEbo);

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

    //    ui->listView_newEvent->setText(ui->textEdit_hist);
    //    ui->listView_newEvent->setLabel(ui->label_hist);
    //        ui->listView_newEvent->setSelectionMode(QAbstractItemView::ExtendedSelection);
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
    ui->listView_knownEvent->setRootIndex(_model->index(str_learningRootDir + "empty/"));
    ui->listView_knownEvent->setItemDelegate(new QStyledItemDelegateThumbnailDown(_model, ui->listView_knownEvent, ui->listView_knownEvent));
    //    ui->listView_knownEvent->setLabel(ui->label_hist);
    ui->listView_knownEvent->setKnown(ui->listView_known);
    //    ui->listView_knownEvent->setText(ui->textEdit_hist);

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

//void MainWindow::on_pushButton_up_clicked()
//{
//    on_moveKnownEventSelectedToNewEvent();
//}

//void MainWindow::on_pushButton_down_clicked()
//{
//    on_moveNewEventSelectedToKnown();
//}

//void MainWindow::on_pushButton_deleteAllNewEvent_clicked()
//{
//    //    QDir newEventDir(str_newEventDir);
//    //    newEventDir.setFilter(QDir::Files | QDir::NoDotAndDotDot);
//    //    for (const QString& file : newEventDir.entryList()) {
//    //        newEventDir.remove(file);
//    //    }

//    QDir dir(str_newEventDir);
//    Q_ASSERT(dir.exists());
//    if (!dir.removeRecursively()) {
//        qDebug() << "unable to remove dir : " << str_newEventDir;
//    }
//    dir.mkpath(".");
//}

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
            newHashKnownDir(knownEventDir);
            knownEventDir += "/";
            QString path = str_knownDir + knownEventDir;

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
    QModelIndexList knownSelected = ui->listView_known->selectionModel()->selectedIndexes();
    if (knownSelected.empty()) {
        return;
    }
    //    else if (knownSelected.size() == 1) {
    ////        ui->listView_known->setRootIndex(_model->index(str_learningRootDir + "empty/"));
    //        ui->listView_knownEvent->setRootIndex(_model->index(str_learningRootDir + "empty/"));
    //    }
    //    Q_ASSERT(knownSelected.size() == 1);

    QModelIndex index = knownSelected.first();
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
        qDebug() << "[MainWindow::on_changeKnownSelected] no known dir selected";
        ui->listView_knownEvent->setRootIndex(_model->index(str_learningRootDir + "empty/"));
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
    //    return;
    //    using vec3 = std::vector<float>;
    //    std::vector<float> points;
    //    std::vector<float>
    std::vector<float> poses;
    std::vector<float> densities;
    //    std::vector<float> firsts;
    std::vector<float> rgbs[3];

    std::vector<float> boxPoses;
    std::vector<float> boxDensities;
    std::vector<float> boxRgbs[3];

    //    std::vector<float> boxes;
    //    std::vector<float> color(3);
    //    std::vector<float> color;
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

    //    int nbBoxes = 0;
    for (const auto& pair : m_colors) {
        //    for (QString pathDir : pathDirs) {
        QString pathDir = pair.first;
        QColor color = pair.second;
        std::vector<float> colors({ (float)color.redF(), (float)color.greenF(), (float)color.blueF(), (float)color.alphaF() });

        QDir dir(pathDir);
        Q_ASSERT(dir.exists());
        //        Q_ASSERT(color.size() == 4);
        //        if (color.size() != 3) {
        //            qDebug() << "[MainWindow::on_modelChanged] not good color : " << color << "from " << pathDir;
        //            continue;
        //        }
        dir.setFilter(QDir::AllDirs | QDir::NoDotAndDotDot);

        //        QModelIndex index = _model->index(pathDir);
        //        qDebug() << _model->data(index, Qt::DecorationRole);

        for (const QString& dirName : dir.entryList()) {
            QString filename = pathDir + dirName + "/";
            //            points.insert(points.end(), point.begin(), point.end());
            Identity id(filename.toStdString() + "identity.txt");

            //            QVector3D vec3 = vec3ReadFromBig(pathDir + dirName + "/primary.txt");
            //            std::vector<float> point({ vec3.x(), vec3.y(), vec3.z() });
            std::vector<float> pos = id.pos();
            poses.insert(poses.end(), pos.begin(), pos.end());
            poses.insert(poses.end(), colors.begin(), colors.end());

            std::vector<float> density = id.density();
            densities.insert(densities.end(), density.begin(), density.end());
            densities.insert(densities.end(), colors.begin(), colors.end());

            for (int i = 0; i < 3; ++i) {
                std::vector<float> rgb = id[i];
                rgbs[i].insert(rgbs[i].end(), rgb.begin(), rgb.end());
                rgbs[i].insert(rgbs[i].end(), colors.begin(), colors.end());
            }

            //            points.insert(points.end(), point.begin(), point.end());

            //            points.insert(points.end(), colors.begin(), colors.end());

            //            file.close();
        }

        //        int pointsSize = poses.size();

//    std::vector<float> boxPoses;
//    std::vector<float> boxDensities;
//    std::vector<float> boxRgbs[3];
        if (pathDir != str_newEventDir) {
            Box box(pathDir.toStdString());

            std::vector<float> boxPos = box.pos(colors);
            boxPoses.insert(boxPoses.end(), boxPos.begin(), boxPos.end());
//            boxPoses.insert(boxPoses.end(), colors.begin(), colors.end());

            std::vector<float> boxDensity = box.density(colors);
            boxDensities.insert(boxDensities.end(), boxDensity.begin(), boxDensity.end());
//            boxDensities.insert(boxDensities.end(), colors.begin(), colors.end());
            for (int i =0; i <3; ++i) {
                std::vector<float> boxRgb = box.rgb(colors, i);
                boxRgbs[i].insert(boxRgbs[i].end(), boxRgb.begin(), boxRgb.end());
            }
        }

        //        std::vector<float> boxPoses;

        //        if (pathDir != str_newEventDir) {
        //            QVector3D min = vec3Read(pathDir + "/min.txt");
        //            QVector3D max = vec3Read(pathDir + "/max.txt");

        //                    std::vector<float> a({ min.x(), min.y(), min.z() });
        //                    std::vector<float> b({ max.x(), min.y(), min.z() });
        //                    std::vector<float> c({ max.x(), max.y(), min.z() });
        //                    std::vector<float> d({ min.x(), max.y(), min.z() });

        //                    std::vector<float> e({ min.x(), min.y(), max.z() });
        //                    std::vector<float> f({ max.x(), min.y(), max.z() });
        //                    std::vector<float> g({ max.x(), max.y(), max.z() });
        //                    std::vector<float> h({ min.x(), max.y(), max.z() });

        //                    //            std::vector<float> box({ min.x(), min.y(), min.z() });
        //                    //            std::vector<float> boxes;
        //                    std::vector<std::pair<std::vector<float>&, std::vector<float>&>> indices = { { a, b }, { b, c }, { c, d }, { d, a }, { e, f }, { f, g }, { g, h }, { h, e }, { a, e }, { b, f }, { c, g }, { d, h } };
        //                    for (int i = 0; i < 12; ++i) {
        //                        std::vector<float>& first = indices[i].first;
        //                        std::vector<float>& second = indices[i].second;

        //                        //                qDebug() << first << second;
        //                        boxes.insert(boxes.end(), first.begin(), first.end());
        //                        boxes.insert(boxes.end(), colors.begin(), colors.end());
        //                        boxes.insert(boxes.end(), second.begin(), second.end());
        //                        boxes.insert(boxes.end(), colors.begin(), colors.end());
        //                    }

        //            boxes.insert(boxes.end(), box.begin(), box.end());
        //            boxes.insert(boxes.end(), colors.begin(), colors.end());

        //            box = { max.x(), max.y(), max.z() };
        //            boxes.insert(boxes.end(), box.begin(), box.end());
        //            //            boxes.insert(boxes.end(), colors.begin(), colors.end());
        //        }
    }
    Q_ASSERT(poses.size() % 7 == 0);
    Q_ASSERT(densities.size() % 7 == 0);
    //    qDebug() << "[MainWindow::on_modelChanged] load nb poses =" << poses.size() / 7;

    //    Q_ASSERT(boxes.size() % 7 == 0);
    //    Q_ASSERT(boxes.size() == nbBoxes * 7 * 2);

    //    int nbPoints = points.size() / 7;
    //    points.insert(points.end(), boxes.begin(), boxes.end());

    //    qDebug() << boxes;

    //    ui->openGLWidget->setPoints(points, nbPoints, boxes.size() / (7 * 2 * 12));
    ui->openGLWidget_pos->render(poses, boxPoses);
    ui->openGLWidget_density->render(densities, boxDensities);
    ui->openGLWidget_first->render(rgbs[0], boxRgbs[0]);
    ui->openGLWidget_second->render(rgbs[1], boxRgbs[1]);
    ui->openGLWidget_third->render(rgbs[2], boxRgbs[2]);

    //    ui->openGLWidget_pos->render(m_triangle);
    //    ui->openGLWidget_pos->render(m_triangle);
}

void MainWindow::updateKnownBestPicture()
{
    const QString knownSelected = _model->data(ui->listView_known->selectionModel()->selectedIndexes().first()).toString() + "/";
    //    qDebug() << "knownSelected " << knownSelected;
    //    QString knownEventDir = _model->data(knownEventSelected.first()).toString();
    //    QString path_best = str_knownDir + knownEventDir + "best.jpg";
    QPixmap best;
    //    QVector3D min(1.0, 1.0, 1.0), max(-1.0, -1.0, -1.0), sum(0.0, 0.0, 0.0);
    //    Identity min(641, 481, 641, 481, 640 * 480, NColors({ Color(256, 256, 256), Color(256, 256, 256), Color(256, 256, 256) }));
    Identity sum(0, 0, 0, 0, 0, NColors({ Color(0, 0, 0), Color(0, 0, 0), Color(0, 0, 0) }));
    //    Identity max(-1, -1, -1, -1, -1, NColors({ Color(-1, -1, -1), Color(-1, -1, -1), Color(-1, -1, -1) }));

    Box box;

    //    if (QFile::exists(path_best)) {
    //        best.load(path_best);
    //    } else {
    //        best.load(str_newEventDir + _model->data(newEventSelected.first()).toString() + "/image.jpg");
    //    }
    QDir knownDir(str_knownDir + knownSelected);
    knownDir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
    //    if (!knownDir.isEmpty()) {
    int cpt = 0;
    for (const QString& file : knownDir.entryList()) {
        //        qDebug() << "file " << file;
        //        QString filename = _model->data(index).toString();
        QString path = str_knownDir + knownSelected + file + "/";
        QString imagePath = path + "image.jpg";
        if (!QFile::exists(imagePath)) {
            qDebug() << "[MainWindow::updateKnownBestPicture] : image file not exist : " << imagePath;
        }
        QPixmap cur = QPixmap(imagePath);

        if (cur.width() * cur.height() > best.width() * best.height()) {
            best = cur;
        }

        //        QVector3D vec3 = vec3ReadFromBig(path + "primary.txt");
        //        for (int i = 0; i < 3; ++i) {
        //            min[i] = qMin(min[i], vec3[i]);
        //            max[i] = qMax(max[i], vec3[i]);
        //        }
        //        sum += vec3;
        Identity id(path.toStdString() + "identity.txt");

        sum += id;
        box += id;

        ++cpt;
    }
    if (cpt != 0) {
        box.set_mean(sum / cpt);
    }
    //    }
    box.write((str_knownDir + knownSelected).toStdString());
    //    vec3Save(min, str_knownDir + knownSelected + "min.txt");
    //    vec3Save(max, str_knownDir + knownSelected + "max.txt");
    //    vec3Save(sum / cpt, str_knownDir + knownSelected + "mean.txt");

    //    QString knownEventDir = _model->data(knownEventSelected.first()).toString() + "/";
    best.save(str_knownDir + knownSelected + "best.jpg");
    //    qDebug() << "update best picture done" << knownSelected;
}

void MainWindow::newHashKnownDir(QString filename)
{
    //    qDebug() << "[MainWindow::newHashKnownDir] filename : " << filename;
    auto hash = std::hash<std::string> {}(filename.toStdString());
    double h = (hash % 256) / 256.0;
    //    float g = ((hash / 1000) % 256) / 256.0f;
    //    float b = ((hash / 1000000) % 256) / 256.0f;

    QString path = str_knownDir + filename + "/";
    //    QFile dir (path);
    //    Q_ASSERT(dir.exists());
    //    std::vector<float> vec({r, g, b});
    QColor c;
    c.setHsvF(h, 1.0, 1.0, 1.0);
    m_colors[path] = c;
    qDebug() << "[MainWindow::newHashKnownDir] create new key map value :" << m_colors[path] << "key :" << path;
}
