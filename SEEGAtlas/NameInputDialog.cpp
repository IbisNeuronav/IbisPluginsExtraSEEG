#include "NameInputDialog.h"
#include "ui_NameInputDialog.h"

NameInputDialog::NameInputDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NameInputDialog)
{
    ui->setupUi(this);
}

NameInputDialog::~NameInputDialog()
{
    delete ui;
}

std::string NameInputDialog::getName() {
    QString qName = ui->lineEditName->text();
    return qName.toStdString();
}

void NameInputDialog::setName(std::string name) {
    ui->lineEditName->setText(QString(name.c_str()));
}



