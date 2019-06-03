#include "QListViewKnownEvent.h"

#include <QDebug>
#include <QEvent>
#include <QHelpEvent>
#include <QKeyEvent>
#include <QToolTip>

QListViewKnownEvent::QListViewKnownEvent(QWidget* parent)
    : QListView(parent)
{

    setViewMode(QListView::IconMode);
    setMovement(QListView::Static);
    setSelectionMode(QAbstractItemView::ExtendedSelection);

    installEventFilter(this);
}

bool QListViewKnownEvent::eventFilter(QObject* watched, QEvent* event)
{
    if (event->type() == QEvent::ToolTip) {
//        qDebug() << "eventFilter";
        //        QAbstractItemView* view = qobject_cast<QAbstractItemView*>(watched->parent());
        //        qDebug() << view;
        //        //            QAbstractItemView * view = static_cast<QAbstractItemView*>(parent());
        //        if (!view) {
        //            return false;
        //        }
        QHelpEvent* helpEvent = static_cast<QHelpEvent*>(event);
        QPoint pos = helpEvent->pos();
        //            qDebug() << "pos = " << pos;
        QModelIndex index = indexAt(pos);
        //            qDebug() << "index = " << index;
        QString itemText = model()->data(index, Qt::DisplayRole).toString();
        //            qDebug() << itemText;
        QToolTip::showText(helpEvent->globalPos(), itemText, this);
        return true;
    } else if (event->type() == QEvent::KeyPress) {
//        qDebug() << "key pressed";
        QKeyEvent* key = static_cast<QKeyEvent*>(event);
//        qDebug() << key;
        if (key->key() == Qt::Key_Space) {
            //            qDebug() << "[ListViewNewEvent] enter";
            emit spacePressed();
            //            emit enterPressede();
            return true;
        } else if (key->key() == Qt::Key_Delete) {
            emit deletePressed();
            return true;
        }
    }
    return QListView::eventFilter(watched, event);
    //    else {
    //        return false;
    //    }
}
