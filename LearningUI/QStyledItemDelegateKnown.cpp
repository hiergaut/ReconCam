#include "QStyledItemDelegateKnown.h"
#include "global.h"
#include <QDebug>
#include <QEvent>
#include <QPainter>

QStyledItemDelegateKnown::QStyledItemDelegateKnown(QFileSystemModel* model, QListView* view, QObject* parent)
    : QStyledItemDelegate(parent)
    , _model(model)
    , _listView_knownEvent { view }
{
}

void QStyledItemDelegateKnown::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QRect rect = option.rect;
    //    qDebug() << _model->data(index).toString();
    //    QFileInfo image(_rootPath + _model->data(index).toString());
    QString dir = str_knownDir + _model->data(index).toString() + "/";
    //    return;
    if (option.state & QStyle::State_Selected) {
        //        qDebug() << "paint selected";
        //        painter->setBrush(QBrush(Qt::white));
        //        painter->drawRect(rect.x(), rect.y(), rect.width(), rect.height());
        painter->fillRect(rect, QBrush(Qt::green));
//        if (_listView_knownEvent->rootIndex() != _model->index(dir)) {
//            _listView_knownEvent->setRootIndex(_model->index(dir));
////            qDebug() << "set root knownEvent : " << _model->index(dir);
//            qDebug() << "new known path : " << _model->data(index).toString();
//        }
    }
    //    else {
    //        painter->setBrush(QBrush(Qt::black));
    //    }

    QPixmap pix(dir + "best.jpeg");
    //    qDebug() << _model->data(index, Qt::ToolTipRole);
    //    QRect rect = option.rect;
    if (option.showDecorationSelected) {
        qDebug() << "fuck";
    }
    //    QRect rect = pix.rect();
    //    painter->setBrush(QBrush(Qt::white));
    //    painter->setBackground(Qt::green);
    //    painter->drawRect(rect.x(), rect.y(), rect.width(), rect.height());
    painter->drawPixmap(rect.x() + 2, rect.y() + 2, rect.width() - 4, rect.height() - 4, pix);
}

QSize QStyledItemDelegateKnown::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    //    _model->setData(index, "fuck", Qt::ToolTip);
    if (option.state & QStyle::State_Selected) {
        qDebug() << "bitch";
    }

    QSize result = QStyledItemDelegate::sizeHint(option, index);
    int size = 100;
    result.setHeight(size);
    result.setWidth(size);
    return QSize(size, size);
    //    return result;
}
