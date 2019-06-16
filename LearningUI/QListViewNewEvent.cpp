#include "QListViewNewEvent.h"

#include <QDebug>
#include <QEvent>
#include <QHelpEvent>
#include <QKeyEvent>
#include <QToolTip>

#include "mainwindow.h"
#include "global.h"

QListViewNewEvent::QListViewNewEvent(QWidget* parent)
    : QListView(parent)
{
    setViewMode(QListView::IconMode);
    setMovement(QListView::Static);
    setSelectionMode(QAbstractItemView::ExtendedSelection);

    installEventFilter(this);
}

bool QListViewNewEvent::eventFilter(QObject* watched, QEvent* event)
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

        QString path = str_newEventDir + itemText + "/";
//        QPixmap pix(path + "hist.jpg");
//        QFile file(path + "primary.txt");
//        file.open(QFile::ReadOnly | QFile::Text);
//        QTextStream in (&file);

//        QString data;
//        QRegExp reg("[ ]");
//        while (! in.atEnd()) {
//            QString line = in.readLine();
//            QStringList list = line.split(reg);
//            for (int i =0; i <3; ++i) {
//                data += list[i] + "\n";
//            }
//            data += "\n";
//        }
//        file.close();
//        QString data = in.readAll();
//        text->setText(data);
//        label->setPixmap(pix);
        return true;
    } else if (event->type() == QEvent::KeyPress) {
//        qDebug() << "key pressed";
        QKeyEvent* key = static_cast<QKeyEvent*>(event);
//        qDebug() << key;
        if (key->key() == Qt::Key_Space) {
            //            qDebug() << "[ListViewNewEvent] enter";
            emit enterPressed();
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

//void QListViewNewEvent::setText(QTextEdit *value)
//{
//    text = value;
//}

//void QListViewNewEvent::setLabel(QLabel *value)
//{
//    label = value;
//}

//bool QListViewNewEvent::event(QEvent *event)
//{
//    qDebug() << event;
////    return false;
//    event->accept();
//    return false;

//}
