#include "QStyledItemDelegateKnown.h"
#include "global.h"
#include <QDebug>
#include <QEvent>
#include <QPainter>

QStyledItemDelegateKnown::QStyledItemDelegateKnown(QFileSystemModel* model, QListView* view, std::map<QString, QColor>& colors, QObject* parent)
    : QStyledItemDelegate(parent)
    , _model(model)
    , _listView_knownEvent { view }
    , m_colors { colors }
{
}

void QStyledItemDelegateKnown::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QRect rect = option.rect;
    //    qDebug() << _model->data(index).toString();
    //    QFileInfo image(_rootPath + _model->data(index).toString());
    QString dir = str_knownDir + _model->data(index).toString() + "/";
    QPixmap pix(dir + "best.jpg");
    //    return;
    QColor c = m_colors[dir];
    if (!(option.state & QStyle::State_Selected)) {
        //        qDebug() << "paint selected";
        c.setAlphaF(0.5);
        //        painter->setBrush(QBrush(Qt::white));
        //        painter->drawRect(rect.x(), rect.y(), rect.width(), rect.height());
        //        painter->fillRect(rect, QBrush(Qt::green));
        //        if (_listView_knownEvent->rootIndex() != _model->index(dir)) {
        //            _listView_knownEvent->setRootIndex(_model->index(dir));
        //            qDebug() << "new known path : " << _model->data(index).toString();
        //        }
        QImage img_gray = pix.toImage().convertToFormat(QImage::Format_Grayscale8);
        pix = QPixmap::fromImage(img_gray);
    }

    //    Q_ASSERT(! m_colors[dir].empty());
    //    if (m_colors[dir]) {
    //        return;
    //    }
    //    QColor c;
    //    c.setRgbF(color[0], color[1], color[2]);
    painter->fillRect(rect, QBrush(c));

    //    else {
    //        painter->setBrush(QBrush(Qt::black));
    //    }

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
