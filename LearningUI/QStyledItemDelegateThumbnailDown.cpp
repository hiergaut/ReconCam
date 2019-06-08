#include "QStyledItemDelegateThumbnailDown.h"

#include "global.h"
#include <QDebug>
#include <QEvent>
#include <QPainter>

QStyledItemDelegateThumbnailDown::QStyledItemDelegateThumbnailDown(QAbstractItemModel* model, QListView* view, QObject* parent)
    : QStyledItemDelegate(parent)
    , _model(model)
    , _view { view }
{
}

void QStyledItemDelegateThumbnailDown::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QRect rect = option.rect;
    //    qDebug() << _model->data(index).toString();
    //    QFileInfo image(_rootPath + _model->data(index).toString());
    //    QPixmap pix;
    //    return;
    //    qDebug() << "fuck" << _model->data(index).toString();

    QModelIndex rootIndex = _view->rootIndex();
    QString path = str_knownDir + _model->data(rootIndex).toString() + "/" + _model->data(index).toString() + "/";
    //    QString imagePath = path + "image.jpg";
    //    QFileInfo img(imagePath);
    //    if (img.isDir()) {
    //        return;
    //    }
    //    if (!img.exists()) {
    ////        qDebug() << "[QStyledItemDelegateThumbnailDown] file not exist : " << img;
    //        return;
    //    }
    //    qDebug() << img;
    QPixmap color(path + "image.jpg");
    QPixmap hist(path + "hist.jpg");

    //    qDebug() << "color = " << color;
//    int w = rect.width();
//    int middle = w / 2 - 10;
    if (option.state & QStyle::State_Selected) {
        //                qDebug() << "paint selected";
        //        painter->setBrush(QBrush(Qt::white));
        //        painter->drawRect(rect.x(), rect.y(), rect.width(), rect.height());
        //        painter->fillRect(rect, QBrush(Qt::green));
        QImage img_gray = color.toImage().convertToFormat(QImage::Format_Grayscale8);
        QPixmap gray = QPixmap::fromImage(img_gray);
        painter->fillRect(rect, QBrush(Qt::green));
        painter->drawPixmap(rect.x() + 2, rect.y() + 2, rect.width() - 4, rect.height() - 4, gray);
    } else {
        painter->drawPixmap(rect.x() + 2, rect.y() + 2, rect.width() - 4, rect.height() - 4, color);
    }

    //    painter->drawPixmap(rect.x() + 2 + middle, rect.y() + 2, middle + middle, rect.height() - 4, hist);
    //    painter->drawPixmap(rect.x() + 2, rect.y() + 2, middle, rect.height() - 4, hist);
    //    else {
    //        painter->setBrush(QBrush(Qt::black));
    //    }

    //    qDebug() << _model->data(index, Qt::ToolTipRole);
    //    QRect rect = option.rect;
    //    if (option.showDecorationSelected) {
    //        qDebug() << "fuck";
    //    }
    //    QRect rect = pix.rect();
    //    painter->setBrush(QBrush(Qt::white));
    //    painter->setBackground(Qt::green);
    //    painter->drawRect(rect.x(), rect.y(), rect.width(), rect.height());
}

QSize QStyledItemDelegateThumbnailDown::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    //    _model->setData(index, "fuck", Qt::ToolTip);
    if (option.state & QStyle::State_Selected) {
        qDebug() << "bitch";
    }

    QSize result = QStyledItemDelegate::sizeHint(option, index);
    int size = 100;
    result.setHeight(size);
    result.setWidth(size);
    //    return result;
//    return QSize(size, size);
        return result;
}
