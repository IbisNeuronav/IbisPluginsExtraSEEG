#ifndef NAMEINPUTDIALOG_H
#define NAMEINPUTDIALOG_H

#include <QDialog>


namespace Ui {
class NameInputDialog;
}

class NameInputDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NameInputDialog(QWidget *parent = 0);
    ~NameInputDialog();
    std::string getName();
    void setName(std::string name);



private:
    Ui::NameInputDialog *ui;
};


#endif // NAMEINPUTDIALOG_H
