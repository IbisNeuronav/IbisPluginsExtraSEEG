#ifndef CONTACTSLISTTABLEWIDGET_H
#define CONTACTSLISTTABLEWIDGET_H

#include <QWidget>

class ContactsListTableWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ContactsListTableWidget(QWidget *parent = 0);
    
signals:
    void currentCellChanged(int,int,int,int);
public slots:

    
};

#endif // CONTACTSLISTTABLEWIDGET_H
