#ifndef QSTYLEDITEMDELEGATETHUMBNAIL_H
#define QSTYLEDITEMDELEGATETHUMBNAIL_H

#include <QStyledItemDelegate>
//#include <QStandardItemModel>
#include <QFileSystemModel>
#include <QStyleOptionViewItem>
#include <QListView>

class QStyledItemDelegateThumbnail : public QStyledItemDelegate {
public:
    QStyledItemDelegateThumbnail(QAbstractItemModel* model, QObject* parent = nullptr);

    virtual void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    virtual QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
//    virtual bool eventFilter(QObject *object, QEvent *event) override;

private:
    QAbstractItemModel* _model;
//    QListView * _view;
//    QString _rootPath;
};

#endif // QSTYLEDITEMDELEGATETHUMBNAIL_H
