#include "QTreeViewOpengl.h"

#include <QDebug>

QTreeViewOpengl::QTreeViewOpengl(QWidget *parent)
{

}

void QTreeViewOpengl::rowsInserted(const QModelIndex &parent, int start, int end)
{
//    qDebug() << "rowsInserted : " << model()->data(parent);

//    expandAll();
//    expand(parent);

}

void QTreeViewOpengl::dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles)
{
//    qDebug() << "dataChanged";

}
