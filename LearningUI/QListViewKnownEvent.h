#ifndef QLISTVIEWKNOWNEVENT_H
#define QLISTVIEWKNOWNEVENT_H

#include <QObject>
#include <QListView>

class QListViewKnownEvent : public QListView {
    Q_OBJECT
public:
    QListViewKnownEvent(QWidget* parent);

signals:
    void spacePressed();
    void deletePressed();

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;
};

#endif // QLISTVIEWKNOWNEVENT_H
