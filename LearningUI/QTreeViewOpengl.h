#ifndef QTREEVIEWOPENGL_H
#define QTREEVIEWOPENGL_H

#include <QTreeView>

class QTreeViewOpengl : public QTreeView
{
    Q_OBJECT
public:
    QTreeViewOpengl(QWidget * parent);


protected:
    void rowsInserted(const QModelIndex &parent, int start, int end) override;
    void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles = QVector<int>()) override;
};

#endif // QTREEVIEWOPENGL_H
