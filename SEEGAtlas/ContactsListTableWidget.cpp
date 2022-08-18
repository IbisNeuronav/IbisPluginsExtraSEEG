#include "ContactsListTableWidget.h"
#include <QGridLayout>
#include <QPushButton>
#include <QTableWidget>

ContactsListTableWidget::ContactsListTableWidget(QWidget *parent) :
    QWidget(parent)
{
    // On the Top: place the table
    int nCols = 6;
    QTableWidget *tableWidgetContacts = new QTableWidget(this);
    tableWidgetContacts->setAccessibleName("tableWidgetContacts");
    tableWidgetContacts->setColumnCount(nCols);
    tableWidgetContacts->setSortingEnabled(false);
    QStringList  headerLabels;
    headerLabels<< "Name"<< "X"<< "Y"<< "Z"<< "location"<<"Proba";
    tableWidgetContacts->setHorizontalHeaderLabels(headerLabels);
    for (int i=0;i<nCols; i++){
        tableWidgetContacts->setColumnWidth(i, 55);
    }
    tableWidgetContacts->setRowCount(0);

// in  SEEGAtlasWidget add:   QObject::connect(ContactsListTableWidget, SIGNAL(currentCellChanged(int,int,int,int)), SEEGAtlasWidget, onTrajectoryTableCellChange(int, int, int, int));
    QHBoxLayout *topLayout = new QHBoxLayout;
    topLayout->addWidget(tableWidgetContacts);


    // On the Bottom: place the buttons
  //  QPushButton *btnUpdateElect = new QPushButton("Update Electrodes", this);
  //  QPushButton *btnFindContactLocation = new QPushButton("Find Contacts location", this);
 //   QHBoxLayout *btnLayout = new QHBoxLayout;
 //   btnLayout->addWidget(btnUpdateElect);
 //   btnLayout->addWidget(btnFindContactLocation);

    // Put Everything together
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(topLayout);
   // mainLayout->addLayout(btnLayout);
    this->setLayout(mainLayout);
}

