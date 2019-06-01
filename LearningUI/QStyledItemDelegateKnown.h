#ifndef QSTYLEDITEMDELEGATEKNOWN_H
#define QSTYLEDITEMDELEGATEKNOWN_H

#include <QFileSystemModel>
#include <QListView>
#include <QStyledItemDelegate>

class QStyledItemDelegateKnown : public QStyledItemDelegate {
public:
    QStyledItemDelegateKnown(QFileSystemModel* model, QListView* view, QObject* parent = nullptr);

    virtual void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    virtual QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    //    virtual bool eventFilter(QObject* object, QEvent* event) override;

private:
    QFileSystemModel* _model;
    //    QString _rootPath;
    QListView* _listView_knownEvent;
};

#endif // QSTYLEDITEMDELEGATEKNOWN_H
