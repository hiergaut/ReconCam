#ifndef QLISTVIEWKNOWN_H
#define QLISTVIEWKNOWN_H

#include <QObject>
#include <QWidget>
#include <QListView>

class QListViewKnown : public QListView
{
    Q_OBJECT
public:
    explicit QListViewKnown(QWidget *parent = nullptr);

signals:
    void deletePressed();

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

signals:

public slots:
};

#endif // QLISTVIEWKNOWN_H
