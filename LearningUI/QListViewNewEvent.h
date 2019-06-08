#ifndef QLISTVIEWNEWEVENT_H
#define QLISTVIEWNEWEVENT_H

#include <QObject>
#include <QListView>
//#include <QObject>

#include <QLabel>
#include <QTextEdit>

class QListViewNewEvent : public QListView {
    Q_OBJECT
public:
    QListViewNewEvent(QWidget* parent);

    void setLabel(QLabel *value);

    void setText(QTextEdit *value);

signals:
    void enterPressed();
    void deletePressed();

protected:
    //    bool event(QEvent *event) override;
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    QLabel * label;
    QTextEdit * text;

};

#endif // QLISTVIEWNEWEVENT_H
