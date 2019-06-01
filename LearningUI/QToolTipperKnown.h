#ifndef QTOOLTIPPERKNOWN_H
#define QTOOLTIPPERKNOWN_H

#include "global.h"
#include <QAbstractItemView>
#include <QDebug>
#include <QEvent>
#include <QFileSystemModel>
#include <QHelpEvent>
#include <QListView>
#include <QObject>
#include <QToolTip>

class QToolTipperKnown : public QObject {
    Q_OBJECT
public:
    QToolTipperKnown(QListView* listView, QListView* knownEvent, QFileSystemModel* model, QObject* parent = nullptr)
        : QObject(parent)
        , _view { listView }
        , _knownEvent { knownEvent }
        , _model { model }
    {
    }

protected:
    bool eventFilter(QObject* watched, QEvent* event) override
    {
        switch (event->type()) {
        case QEvent::ToolTip:
        case QEvent::MouseButtonPress:
            break;
        }
        if (event->type() == QEvent::ToolTip) {
            //            QAbstractItemView* view = qobject_cast<QAbstractItemView*>(watched->parent());
            //            QAbstractItemView * view = static_cast<QAbstractItemView*>(parent());
            //            if (!view) {
            //                return false;
            //            }
            QHelpEvent* helpEvent = static_cast<QHelpEvent*>(event);
            QPoint pos = helpEvent->pos();
            //            qDebug() << "pos = " << pos;
            QModelIndex index = _view->indexAt(pos);
            //            qDebug() << "index = " << index;
            QString itemText = _view->model()->data(index, Qt::DisplayRole).toString();
            //            qDebug() << itemText;
            QToolTip::showText(helpEvent->globalPos(), itemText, _view);

        } else if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
            if (mouseEvent->button() == Qt::LeftButton) {
                qDebug() << "mouse button press";
                QPoint pos = mouseEvent->pos();
                QModelIndex index = _view->indexAt(pos);
                QString itemText = _view->model()->data(index, Qt::DisplayRole).toString();
                qDebug() << itemText;

//                QFileSystemModel* model = static_cast<QFileSystemModel*>(_view->model());
                _knownEvent->setRootIndex(_model->index(str_knownDir + "gauthier/"));
//                _knownEvent->reset();

            }
        } else {
            return false;
        }
        return true;
    }

private:
    QListView* _view;
    QListView* _knownEvent;
    QFileSystemModel* _model;
};

#endif // QTOOLTIPPERKNOWN_H
