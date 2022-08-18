#include "Point3DInputDialog.h"

using namespace seeg;

Point3DInputDialog::Point3DInputDialog(QWidget *parent)
    : QDialog(parent)
{
	ui.setupUi(this);
}

Point3DInputDialog::~Point3DInputDialog()
{

}

Point3D Point3DInputDialog::getPoint3D() {
    bool rc;
    Point3D point;
    point[0] = ui.lineEditPointX->text().toFloat(&rc);
    point[1] = ui.lineEditPointY->text().toFloat(&rc);
    point[2] = ui.lineEditPointZ->text().toFloat(&rc);
    return point;
}

void Point3DInputDialog::setPoint3D(Point3D point) {
    ui.lineEditPointX->setText(QString::number(point[0]));
    ui.lineEditPointY->setText(QString::number(point[1]));
    ui.lineEditPointZ->setText(QString::number(point[2]));
}


void Point3DInputDialog::onOk() {
    this->setResult(QDialog::Accepted);
    hide();
}

void Point3DInputDialog::onCancel() {
    close();
}
