#ifndef QLISTVIEWKNOWNEVENT_H
#define QLISTVIEWKNOWNEVENT_H

#include <QObject>
#include <QListView>
#include "QListViewKnown.h"
#include <QLabel>
#include <QTextEdit>

class QListViewKnownEvent : public QListView {
    Q_OBJECT
public:
    QListViewKnownEvent(QWidget* parent);




    void setKnown(QListViewKnown *value);

//    void setLabel(QLabel *value);

//    void setText(QTextEdit *value);

signals:
    void spacePressed();
    void deletePressed();

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
//    QLabel * label;
    QListViewKnown * known;
//    QTextEdit * text;
};

#endif // QLISTVIEWKNOWNEVENT_H
