#ifndef QLISTVIEWNEWEVENT_H
#define QLISTVIEWNEWEVENT_H

#include <QObject>
#include <QListView>
//#include <QObject>

class QListViewNewEvent : public QListView {
    Q_OBJECT
public:
    QListViewNewEvent(QWidget* parent);

signals:
    void enterPressed();
    void deletePressed();

protected:
    //    bool event(QEvent *event) override;
    bool eventFilter(QObject* watched, QEvent* event) override;
};

#endif // QLISTVIEWNEWEVENT_H
