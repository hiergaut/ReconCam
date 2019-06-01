#ifndef QTOOLTIPPER_H
#define QTOOLTIPPER_H

#include <QEvent>
#include <QObject>
#include <QDebug>
#include <QAbstractItemView>
#include <QHelpEvent>
#include <QToolTip>

class QToolTipper : public QObject {
    Q_OBJECT
public:
    QToolTipper(QObject* parent = nullptr)
        : QObject(parent)
    {
    }

protected:
    bool eventFilter(QObject* watched, QEvent* event) override
    {
        if (event->type() == QEvent::ToolTip) {
            QAbstractItemView * view = qobject_cast<QAbstractItemView*>(watched->parent());
//            QAbstractItemView * view = static_cast<QAbstractItemView*>(parent());
            if (! view) {
                return false;
            }
            QHelpEvent * helpEvent = static_cast<QHelpEvent*>(event);
            QPoint pos = helpEvent->pos();
//            qDebug() << "pos = " << pos;
            QModelIndex index = view->indexAt(pos);
//            qDebug() << "index = " << index;
            QString itemText = view->model()->data(index, Qt::DisplayRole).toString();
//            qDebug() << itemText;
            QToolTip::showText(helpEvent->globalPos(), itemText, view);
            return true;
        }
        return false;
    }

};

#endif // QTOOLTIPPER_H
