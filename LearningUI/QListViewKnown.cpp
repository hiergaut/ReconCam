#include "QListViewKnown.h"

#include <QDebug>
#include <QEvent>
#include <QHelpEvent>
#include <QToolTip>

QListViewKnown::QListViewKnown(QWidget *parent) : QListView(parent)
{
    setViewMode(QListView::IconMode);
    setMovement(QListView::Static);

    installEventFilter(this);
}

QString QListViewKnown::getSelected()
{
    return selectedIndexes().first().data().toString() + "/";
}

bool QListViewKnown::eventFilter(QObject *watched, QEvent *event)
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
        if (key->key() == Qt::Key_Delete) {
            emit deletePressed();
            return true;
        }
    }
    else if (event->type() == QEvent::MouseButtonPress) {
        qDebug() << "mouse event";
        QMouseEvent * mouse = static_cast<QMouseEvent*>(event);
        if (mouse->button() == Qt::LeftButton) {
            qDebug() << "mouse left";
        }
        return true;
    }
    return QListView::eventFilter(watched, event);
    //    else {
    //        return false;
    //    }

}
