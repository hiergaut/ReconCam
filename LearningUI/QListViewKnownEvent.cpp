#include "QListViewKnownEvent.h"

#include <QDebug>
#include <QEvent>
#include <QHelpEvent>
#include <QKeyEvent>
#include <QToolTip>
#include "global.h"

QListViewKnownEvent::QListViewKnownEvent(QWidget* parent)
    : QListView(parent)
{

    setViewMode(QListView::IconMode);
    setMovement(QListView::Static);
    setSelectionMode(QAbstractItemView::ExtendedSelection);

    installEventFilter(this);
//    viewport()->installEventFilter(this);
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
//        qDebug() << index;
        if (! index.isValid()) {
//            qDebug() << "[QListViewKnownEvent::eventFilter] index not valid";
            return false;
        }
        //            qDebug() << "index = " << index;
        QString itemText = model()->data(index, Qt::DisplayRole).toString();
//                    qDebug() << itemText;
        QToolTip::showText(helpEvent->globalPos(), itemText, this);

//        QString path = str_knownDir + known->getSelected() + itemText + "/";
//        QPixmap pix(path + "hist.jpg");
//        QFile file(path + "primary.txt");
//        file.open(QFile::ReadOnly | QFile::Text);
//        QTextStream in (&file);

//        QString data = in.readAll();
////        qDebug() << data;
//        text->setText(data);

//        label->setPixmap(pix);
//        label->setToolTip()

//        QPixmap pix(str_kn)
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
        //    else {
        //        return false;
        //    }
    } else if (event->type() == QEvent::MouseButtonPress) {
        qDebug() << "mouse";
    }
    return QListView::eventFilter(watched, event);
}

//void QListViewKnownEvent::setText(QTextEdit *value)
//{
//    text = value;
//}

//void QListViewKnownEvent::setLabel(QLabel *value)
//{
//    label = value;
//}

void QListViewKnownEvent::setKnown(QListViewKnown *value)
{
    known = value;
}


