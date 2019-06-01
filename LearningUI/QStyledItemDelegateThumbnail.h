#ifndef QSTYLEDITEMDELEGATETHUMBNAIL_H
#define QSTYLEDITEMDELEGATETHUMBNAIL_H

#include <QStyledItemDelegate>
//#include <QStandardItemModel>
#include <QFileSystemModel>
#include <QStyleOptionViewItem>

class QStyledItemDelegateThumbnail : public QStyledItemDelegate {
public:
    QStyledItemDelegateThumbnail(QAbstractItemModel* model, QString rootPath, QObject* parent = nullptr);

    virtual void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    virtual QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

private:
    QAbstractItemModel* _model;
    QString _rootPath;
};

#endif // QSTYLEDITEMDELEGATETHUMBNAIL_H
