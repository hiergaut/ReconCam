#include "QStyledItemDelegateThumbnail.h"

#include <QDebug>
#include <QPainter>
#include <QEvent>

QStyledItemDelegateThumbnail::QStyledItemDelegateThumbnail(QAbstractItemModel* model, QString rootPath, QObject* parent)
    : QStyledItemDelegate(parent)
    , _model(model)
    , _rootPath { rootPath }
{
}

void QStyledItemDelegateThumbnail::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
//    qDebug() << _model->data(index).toString();
//    QFileInfo image(_rootPath + _model->data(index).toString());

    QPixmap pix(_rootPath + _model->data(index).toString());
//    qDebug() << _model->data(index, Qt::ToolTipRole);
    QRect rect = option.rect;
//    QRect rect = pix.rect();
    painter->setBrush(QBrush(Qt::white));
//    painter->setBackground(Qt::green);
    painter->drawRect(rect.x(), rect.y(), rect.width(), rect.height());
    painter->drawPixmap(rect.x() +2, rect.y() + 2, rect.width() -4, rect.height() -4, pix);
}

QSize QStyledItemDelegateThumbnail::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
//    _model->setData(index, "fuck", Qt::ToolTip);

    QSize result = QStyledItemDelegate::sizeHint(option, index);
    int size = 100;
    result.setHeight(size);
    result.setWidth(size);
    return QSize(size, size);

}
