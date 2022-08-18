#ifndef POINT3DINPUTDIALOG_H
#define POINT3DINPUTDIALOG_H

#include <QtWidgets/QDialog>
#include "ui_Point3DInputDialog.h"
#include "VolumeTypes.h"


class Point3DInputDialog : public QDialog
{
    Q_OBJECT

public:
    Point3DInputDialog(QWidget *parent = 0);
    ~Point3DInputDialog();

    seeg::Point3D getPoint3D();
    void setPoint3D(seeg::Point3D point);

public slots:
    void onOk();
    void onCancel();

private:
    Ui::Point3DInputDialogClass ui;
};

#endif // POINT3DINPUTDIALOG_H
