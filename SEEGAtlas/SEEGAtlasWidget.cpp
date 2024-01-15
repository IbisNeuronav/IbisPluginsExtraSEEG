// Plugin includes
#include "SEEGAtlasWidget.h"
#include "ui_SEEGAtlasWidget.h"
#include "Point3DInputDialog.h"
#include "ContactsListTableWidget.h"
#include "NameInputDialog.h"
#include <SEEGPointRepresentation.h>

// IBIS include
#include "application.h"
#include "polydataobject.h"
#include "scenemanager.h"
#include "view.h"
#include "imageobject.h"
#include "opendatafiledialog.h"
#include "filereader.h"
#include "ibisapi.h"
#include "contoursurfaceplugininterface.h"

// VTK includes
#include <vtkRenderWindowInteractor.h>
#include <vtkProperty.h>
#include "vtkLinearTransform.h"
#include "vtkTransform.h"
#include <vtkCommand.h>
#include <vtkIntersectionPolyDataFilter.h>
#include <vtkDistancePolyDataFilter.h>
#include <vtkThreshold.h>

//#include <vtkClipPolyData.h>

// Qt includes
#include <QDebug>
#include <QMessageBox>
#include <QTableWidgetItem>
#include <QFileDialog>
#include <QDateTime>
#include <QDir>
#include <QXmlStreamReader>
#include <QDirIterator>
#include <QProgressDialog>

// itk includes
#include "itkImageRegionIterator.h"

// seeg namespace includes
//#include "PlanningUI.h"
//#include "SEEGPathPlanner.h"
#include "MathUtils.h"
#include "VolumeTypes.h"
#include "SEEGTrajVisWidget.h"
#include "SEEGElectrodesCohort.h"
#include "ElectrodeInfo.h"
#include "SEEGFileHelper.h"
#include "SEEGContactsROIPipeline.h"
#include "ChannelInfo.h"

using namespace seeg;
using namespace itk;

//typedef ImageRegionIteratorWithIndex<IbisItk3DImageType> RegionIteratorType;


/**********************************************************************
 *  CONSTRUCTOR/DESTRUCTOR
 **********************************************************************/

SEEGAtlasWidget::SEEGAtlasWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SEEGAtlasWidget)
{
    ui->setupUi(this);

    m_ConfigurationDir = "";

    // electrode colors - Create the color map
//    vector <QColor> posColors;
    m_PosColors.push_back(QColor("green"));m_PosColors.push_back(QColor("purple")); m_PosColors.push_back(QColor("red"));m_PosColors.push_back( QColor("blue"));
    m_PosColors.push_back(QColor("yellow"));m_PosColors.push_back(QColor("pink")); m_PosColors.push_back(QColor("orange")); m_PosColors.push_back(QColor("brown"));
    m_PosColors.push_back(QColor("grey"));m_PosColors.push_back(QColor("olive")); m_PosColors.push_back(QColor("magenta"));m_PosColors.push_back(QColor("cyan"));
    m_PosColors.push_back(QColor("slateblue"));m_PosColors.push_back(QColor("lime"));m_PosColors.push_back(QColor("darkgoldenrod"));m_PosColors.push_back(QColor("lavender"));
    m_PosColors.push_back(QColor("springgreen"));m_PosColors.push_back(QColor("teal"));m_PosColors.push_back(QColor("thistle"));m_PosColors.push_back(QColor("tan"));

    // Init the scene!
    SceneManager *scene = Application::GetInstance().GetSceneManager();
    m_TrajPlanMainObject = SceneObject::New();
    m_TrajPlanMainObject->SetName("Electrodes (Ref)");
    m_TrajPlanMainObject->SetListable(true); //true is the default, so it is actually not necessary
    m_SavedPlansObject = SceneObject::New();
    m_SavedPlansObject->SetName("Saved Elect");
    m_SavedPlansObject->SetListable(true);

    m_PlanningRootObject = SceneObject::New();
    m_PlanningRootObject->SetName("ELECTRODES");
    scene->AddObject(m_PlanningRootObject);
    scene->AddObject(m_TrajPlanMainObject, m_PlanningRootObject );
    scene->AddObject(m_SavedPlansObject, m_PlanningRootObject );

    Application::GetInstance().GetSceneManager()->RemoveAllChildrenObjects(this->m_SavedPlansObject);

    m_ElectrodeDisplayWidth = 1;

  // Init SEEGTrajVisWidget instance
    m_TrajVisWidget = new SEEGTrajVisWidget();

    m_SpacingResolution = 0.5; //RIZ: HARDCODED FOR NOW!!!! see how to define!

    // Assign default model (the one in the combobox default)
    m_ElectrodeModel = nullptr;

    //Create Active Plan
    CreateActivePlan();

    //React when an object or image is added or removed from IBIS
    QObject::connect(Application::GetInstance().GetSceneManager(), SIGNAL(ObjectAdded(int)), this, SLOT(OnObjectAddedSlot(int)));
    QObject::connect(Application::GetInstance().GetSceneManager(), SIGNAL(ObjectRemoved(int)), this, SLOT(OnObjectRemovedSlot(int)));

   // this->onChangeTrajectoryCylinderRadius(ui->horizontalSliderCylRadius->value());
   // this->onChangeTrajectoryCylinderLength(ui->horizontalSliderCylinderLength->value());
   // RefreshAllPlanCoords();
}

SEEGAtlasWidget::~SEEGAtlasWidget()
{
    // TODO: more cleanup needed. It leaks badly!!
    ResetElectrodes();
    delete ui;
    m_ActivePlanData.m_CylObj->Delete();
    delete m_TrajVisWidget;

    Q_ASSERT(m_pluginInterface);
    IbisAPI * ibisApi = m_pluginInterface->GetIbisAPI();
    ibisApi->RemoveObject(m_PlanningRootObject);

    disconnect(Application::GetInstance().GetSceneManager(), SIGNAL(ObjectAdded(int)), this, SLOT(OnObjectAddedSlot(int)));
    disconnect(Application::GetInstance().GetSceneManager(), SIGNAL(ObjectRemoved(int)), this, SLOT(OnObjectRemovedSlot(int)));
}

void SEEGAtlasWidget::SetPluginInterface(SEEGAtlasPluginInterface * interf)
{
    m_pluginInterface = interf;
    this->InitUI();
}

void SEEGAtlasWidget::InitUI()
{
    // Initialize ui components

    // Create Cohort object
    m_SEEGElectrodesCohort = SEEGElectrodesCohort::New();
    m_SEEGElectrodesCohort->SetElectrodeModel(m_ElectrodeModel);
    m_SEEGElectrodesCohort->SetSpacingResolution(m_SpacingResolution);

    if( m_pluginInterface )
    {
        IbisAPI * ibisApi = m_pluginInterface->GetIbisAPI();

        // Read config file of the pointer in ./ibis/SEEGAtlasData/<Electrode>
        QString foldername(QDir(ibisApi->GetConfigDirectory()).filePath("SEEGAtlasData"));

        // create config directory if it does not exist
        if( !QFile::exists(foldername) )
        {
            QDir().mkdir(foldername);
        }

        m_ConfigurationDir = foldername;
        QStringList folders = QDir(foldername).entryList(QStringList() << "*.xml", QDir::Files | QDir::NoDot | QDir::NoDotDot, QDir::Name);

        // loop through all config files
        for(int i=0; i < folders.size(); ++i)
        {
            SerializerReader reader;
            reader.SetFilename(QDir(foldername).filePath(folders[i]).toUtf8().data());
            reader.Start();
            SEEGElectrodeModel::Pointer elec = SEEGElectrodeModel::New();
            elec->Serialize(&reader);
            reader.Finish();

            m_ElectrodeModelList.push_back(elec);

            QString elname(elec->GetElectrodeId().c_str());
            ui->comboBoxElectrodeType->addItem(elname, QVariant(elname));
        }

        // if no electrode config file was found
        if( m_ElectrodeModelList.size() == 0 )
        {
            // create default electrode model (MNI) and add config file to config dir
            SEEGElectrodeModel::Pointer elec = SEEGElectrodeModel::New();
            QString tempFilename = QDir(foldername).filePath(elec->GetElectrodeId().c_str()) + ".xml";
            SerializerWriter writer;
            writer.SetFilename(tempFilename.toUtf8().data());
            writer.Start();

            elec->Serialize(&writer);
            writer.Finish();
            
            m_ElectrodeModelList.push_back(elec);

            QString elname(elec->GetElectrodeId().c_str());
            ui->comboBoxElectrodeType->addItem(elname, QVariant(elname));
        }

        m_ElectrodeModel = m_ElectrodeModelList[ui->comboBoxElectrodeType->currentIndex()];
    }


    // Some components are hidden to the user to simplify the UI
    // but they are kept for compatibility
    ui->pushLoadDatasetsFromDir->setVisible(true);
    ui->comboBoxWhatToOpen->setVisible(true);
    ui->horizontalSliderCylRadius->setVisible(false);
    ui->horizontalSliderCylinderLength->setVisible(false);
    ui->pushButtonFindAnatLocChannel->setVisible(true);
    ui->comboBoxBrainSegmentation->setVisible(false);
    ui->pushGenerateSurface->setVisible(false);
    ui->horizontalSliderCylRadius->setEnabled(false);
    ui->horizontalSliderCylinderLength->setEnabled(false);
    ui->pushButtonBatch->setEnabled(false);
    ui->pushButtonBatch->setVisible(false);
    ui->pushButtonBatch->setEnabled(false);
    ui->pushButtonShowOnlyContacts->setEnabled(false);
    ui->pushButtonShowOnlyChannels->setEnabled(false);

    //Init Combo Box with Plans
    for( int iElec = 0; iElec < MAX_VISIBLE_PLANS; iElec++ ) { // Only 20 visible to start - MAX_SEEG_PLANS is now 1000!
        ui->comboBoxPlanSelect->addItem(QString("Elect ") + QString::number(iElec + 1));
    }

    //Set the electrode diameter and length as read-only, as they are specified by the manufacturer
    ui->lineEditCylRadius->setReadOnly(true);
    ui->lineEditCylinderLength->setReadOnly(true);

    Application::GetInstance().GetSceneManager()->ViewPlane(0, true);
    Application::GetInstance().GetSceneManager()->ViewPlane(1, true);
    Application::GetInstance().GetSceneManager()->ViewPlane(2, true);
    ui->checkBoxImagePlanes->setChecked(true);

    //Fill the combobox with the images whose contour surface will be generated
    FillComboBoxBrainSegmentation();

    this->UpdateConfigurationFromUi();
    this->UpdateUi();
}

void SEEGAtlasWidget::UpdateUi()
{
    // update UI according to the current electrode model
    if( m_ElectrodeModel )
    {
        //m_ElectrodeModel is not assigned anywhere, this may lead to error
        double sliderPos = m_ElectrodeModel->GetContactDiameter() / 10.0 * 100.0;
        ui->horizontalSliderCylRadius->setValue(sliderPos);
        ui->horizontalSliderCylinderLength->setValue(m_ElectrodeModel->GetElectrodeHeight());
        ui->lineEditCylRadius->setText(QString::number(m_ElectrodeModel->GetContactDiameter()));
        ui->lineEditCylinderLength->setText(QString::number(m_ElectrodeModel->GetElectrodeHeight()));
    }
}

void SEEGAtlasWidget::UpdateConfigurationFromUi()
{
    m_ElectrodeModel = m_ElectrodeModelList.at(ui->comboBoxElectrodeType->currentIndex());

    //TODO add cohort update
}

void SEEGAtlasWidget::UpdateUiFromConfiguration()
{
    for( int i = 0; i < m_ElectrodeModelList.size(); i++ )
    {
        if( m_ElectrodeModel->GetElectrodeId().compare(m_ElectrodeModelList[i]->GetElectrodeId()) == 0 )
        {
            ui->comboBoxElectrodeType->setCurrentIndex(i);
        }
    }
}

void SEEGAtlasWidget::FillComboBoxBrainSegmentation() {
    ui->comboBoxBrainSegmentation->clear();
    QList<ImageObject*> images;
    Application::GetInstance().GetSceneManager()->GetAllImageObjects(images);
    for (int i = 0; i < images.size(); ++i)
    {
        ImageObject* current = images[i];
        ui->comboBoxBrainSegmentation->addItem(current->GetName(), QVariant(current->GetObjectID()));
    }

    if (ui->comboBoxBrainSegmentation->count() == 0)
    {
        ui->comboBoxBrainSegmentation->addItem("None", QVariant(IbisAPI::InvalidId));
    }
}

void SEEGAtlasWidget::OnObjectAddedSlot(int imageObjectId)
{
    SceneObject* sceneObject = Application::GetInstance().GetSceneManager()->GetObjectByID(imageObjectId);
    if (sceneObject->IsA("ImageObject"))
    {
        if (ui->comboBoxBrainSegmentation->count() == 1) //Remove "None" when there is at least one image in the combo
        {
            int currentItemId = ui->comboBoxBrainSegmentation->itemData(ui->comboBoxBrainSegmentation->currentIndex()).toInt();
            if (currentItemId == IbisAPI::InvalidId)
            {
                ui->comboBoxBrainSegmentation->clear();
            }
        }
        ui->comboBoxBrainSegmentation->addItem(sceneObject->GetName(), QVariant(sceneObject->GetObjectID()));
        ui->pushGenerateSurface->setEnabled(true);
    }
}

void SEEGAtlasWidget::OnObjectRemovedSlot(int imageObjectId)
{
    SceneObject* sceneObject = Application::GetInstance().GetSceneManager()->GetObjectByID(imageObjectId);
    if (sceneObject->IsA("ImageObject"))
    {
        for (int i = 0; i < ui->comboBoxBrainSegmentation->count(); ++i)
        {
            int currentItemId = ui->comboBoxBrainSegmentation->itemData(i).toInt();
            if (currentItemId == imageObjectId)
            {
                ui->comboBoxBrainSegmentation->removeItem(i);
            }
        }

        if (ui->comboBoxBrainSegmentation->count() == 0) //Add "None" if the combo is empty
        {
            ui->comboBoxBrainSegmentation->addItem("None", QVariant(IbisAPI::InvalidId));
            ui->pushGenerateSurface->setEnabled(false);
        }
    }
}

void SEEGAtlasWidget::CreateAllElectrodes(bool showProgress) {
    
    // This progress bar does not reflect the real progress
    // it assumes (arbitrarily) MAX_VISIBLE_PLANS electrodes and loops until all electrodes are loaded
    Q_ASSERT(m_pluginInterface);
    IbisAPI * ibisApi = m_pluginInterface->GetIbisAPI();
    int progressMax = MAX_VISIBLE_PLANS;
    QProgressDialog * progress;
    if( showProgress ) progress = ibisApi->StartProgress(progressMax, "Creating electrodes");

    qDebug() << "Entering CreateAllElectrodes";
    // Init saved plan cylinders
    this->UpdateConfigurationFromUi(); //TODO check if needed?

    // Delete ALL electrodes
    ibisApi->RemoveAllChildrenObjects(this->m_SavedPlansObject);
    
    // Create each electrode
    for (int iElec=0; iElec<MAX_SEEG_PLANS; iElec++) {
        if (m_AllPlans[iElec].isEntrySet && m_AllPlans[iElec].isTargetSet) {
            this->CreateElectrode(iElec);
        }
        if( showProgress ) ibisApi->UpdateProgress(progress, iElec % progressMax);
    }
    // Init active electrode plan
    //m_ActivePlanData.m_CylObj->Delete();
    CreateActivePlan();

    if( showProgress ) ibisApi->StopProgress(progress);
	qDebug() << "Leaving CreateAllElectrodes";
}

void SEEGAtlasWidget::CreateElectrode(const int iElec) {
    Point3D pTargetPoint = m_AllPlans[iElec].targetPoint;
    Point3D pEntryPointLong;

    string electrodeName = m_AllPlans[iElec].name;
    ElectrodeInfo::Pointer electrode = m_SEEGElectrodesCohort->GetTrajectoryInBestCohort(electrodeName);
    if(!electrode) return;

    double electrodeLength = electrode->GetElectrodeModel()->GetElectrodeHeight();
    Resize3DLineSegmentSingleSide(m_AllPlans[iElec].targetPoint, m_AllPlans[iElec].entryPoint, electrodeLength, pEntryPointLong);
//    m_AllPlans[iElec].entryPoint = pEntryPointLong; // RIZ2022: update entry point to long point
    CreateElectrode(iElec, pTargetPoint, pEntryPointLong);
}

void SEEGAtlasWidget::CreateElectrode(const int iElec, Point3D pDeep, Point3D pSurface) {
	qDebug() << "Entering CreateElectrode"<< iElec;

    SceneManager *scene = Application::GetInstance().GetSceneManager();

    string electrodeName = m_AllPlans[iElec].name;
    ElectrodeInfo::Pointer electrode = m_SEEGElectrodesCohort->GetTrajectoryInBestCohort(electrodeName);
    if(!electrode) return;

    double electrodeDiameter = electrode->GetElectrodeModel()->GetContactDiameter();

    m_SavedPlansData[iElec].m_ElectrodeDisplay = CreateCylinderObj(m_AllPlans[iElec].name.c_str(), pDeep, pSurface);
    m_SavedPlansData[iElec].m_ElectrodeDisplay.m_Cylinder->SetRadius((electrodeDiameter / 2.0));
    //m_SavedPlansData[iLoc].m_ElectrodeDisplay.m_CylObj->SetColor(colors[iLoc]);
    double dcolor[3];
    dcolor[0] = double( m_PosColors[iElec].red() ) / 255.0;
    dcolor[1] = double( m_PosColors[iElec].green() ) / 255.0;
    dcolor[2] = double( m_PosColors[iElec].blue() ) / 255.0;
    m_SavedPlansData[iElec].m_ElectrodeDisplay.m_CylObj->SetColor(dcolor);
    //UpdatePlan(iElec);
    scene->AddObject(m_SavedPlansData[iElec].m_ElectrodeDisplay.m_CylObj, m_SavedPlansObject);

    m_SavedPlansData[iElec].m_PointRepresentation = seeg::SEEGPointRepresentation::New();
    m_SavedPlansData[iElec].m_PointRepresentation->SetPointsRadius(electrode->GetElectrodeModel()->GetRecordingRadius());
    m_SavedPlansData[iElec].m_PointRepresentation->SetColor(dcolor);
    m_SavedPlansData[iElec].m_PointRepresentation->SetName(m_AllPlans[iElec].name);
    scene->AddObject(m_SavedPlansData[iElec].m_PointRepresentation->GetPointsObject(), m_SavedPlansData[iElec].m_ElectrodeDisplay.m_CylObj);

    // Add also contacts all contacts of default electrode type (MNI)
    m_SavedPlansData[iElec].m_ContactsDisplay = CreateContactCylinderObj("contact", pDeep, pSurface, electrode->GetElectrodeModel());
    std::vector<seeg::Point3D> allContactsCentralPt;
    electrode->GetElectrodeModel()->CalcAllContactPositions(pDeep, pSurface, allContactsCentralPt, false);

    vector<ContactInfo::Pointer> contactList;
    if(electrode) contactList = electrode->GetAllContacts();

    for (int iContact=0; iContact<m_SavedPlansData[iElec].m_ContactsDisplay.size(); iContact++){
        scene->AddObject(m_SavedPlansData[iElec].m_ContactsDisplay[iContact].m_CylObj, m_SavedPlansData[iElec].m_ElectrodeDisplay.m_CylObj);
        m_SavedPlansData[iElec].m_ContactsDisplay[iContact].m_Cylinder->SetRadius((electrodeDiameter / 2.0) + 0.3);
        m_SavedPlansData[iElec].m_ContactsDisplay[iContact].m_CylObj->SetCrossSectionVisible(true); //RIZ20151130 moved here (after contacts are assigned to the scene, IBIS was crashing otherwise
        if(iContact < contactList.size())
        {
            seeg::Point3D savedCentralPoint = contactList[iContact]->GetCentralPoint();
            m_SavedPlansData[iElec].m_PointRepresentation->InsertNextPoint(savedCentralPoint[0], savedCentralPoint[1], savedCentralPoint[2]);
        }
        else
        {
            m_SavedPlansData[iElec].m_PointRepresentation->InsertNextPoint(allContactsCentralPt[iContact][0], allContactsCentralPt[iContact][1], allContactsCentralPt[iContact][2]);
        }
    }
    m_SavedPlansData[iElec].m_ElectrodeDisplay.m_CylObj->SetCrossSectionVisible(true); //RIZ20151130: moved here (after contacts are assigned to the scene, IBIS was crashing otherwise
    m_SavedPlansData[iElec].m_PointRepresentation->ShowPoints();
    UpdatePlan(iElec);


    //add also type of electrode
    m_SavedPlansData[iElec].m_ElectrodeModel = electrode->GetElectrodeModel();
	qDebug() << "Leaving CreateElectrode"<< iElec;
}


void SEEGAtlasWidget::DeleteElectrode(const int iElec) {
    if (m_SavedPlansData[iElec].m_ContactsDisplay.size()>0) {
		qDebug() << "Entering DeleteElectrode " <<iElec;
        m_SavedPlansData[iElec].m_PointRepresentation->Delete();
        m_SavedPlansObject->RemoveChild(m_SavedPlansData[iElec].m_ElectrodeDisplay.m_CylObj);
        for (int iContact=0; iContact<m_SavedPlansData[iElec].m_ContactsDisplay.size(); iContact++){
            m_SavedPlansData[iElec].m_ElectrodeDisplay.m_CylObj->RemoveChild(m_SavedPlansData[iElec].m_ContactsDisplay[iContact].m_CylObj);
            m_SavedPlansData[iElec].m_ContactsDisplay[iContact].m_CylObj->Delete();
       //     m_SavedPlansData[iElec].m_ContactsDisplay[iContact].m_Cylinder->Delete();
        }
     //   m_SavedPlansData[iElec].m_ElectrodeDisplay.m_Cylinder->Delete();
        m_SavedPlansData[iElec].m_ElectrodeDisplay.m_CylObj->SetHidden(true);
        m_SavedPlansData[iElec].m_ElectrodeDisplay.m_CylObj->Delete();

		qDebug() << "Leaving DeleteElectrode";
    }
}

void SEEGAtlasWidget::CreateActivePlan(){
    int iElec = ui->comboBoxPlanSelect->currentIndex();
	qDebug() << "Entering CreateActivePlan - Elec" <<iElec;
    // Remove and then create again
    Point3D pDeep;
    Point3D pSurface;
    if (m_SavedPlansObject->GetNumberOfChildren() > 0) {
        m_SavedPlansObject->RemoveChild(m_ActivePlanData.m_CylObj);
        m_ActivePlanData.m_CylObj->SetHidden(true);
        m_ActivePlanData.m_CylObj->Delete();
        if(m_SavedPlansData[iElec].m_ElectrodeDisplay.m_Line) {
            pDeep = m_SavedPlansData[iElec].m_ElectrodeDisplay.m_Line->GetPoint1(); //m_AllPlans[iElec].targetPoint;
            pSurface =  m_SavedPlansData[iElec].m_ElectrodeDisplay.m_Line->GetPoint2();//m_AllPlans[iElec].entryPoint;
        }
    } else {
        pDeep = m_AllPlans[iElec].targetPoint;
        pSurface =  m_AllPlans[iElec].entryPoint;
    }
    m_ActivePlanData = CreateCylinderObj("active trajectory", pDeep, pSurface); //Active Electrode does not include contacts -> only the electrode
    m_ActivePlanData.m_CylObj->SetOpacity(0.5); //make active semi -transparent
    QString strDiameter =ui->lineEditCylRadius->text();
    double electrodeDiameter = strDiameter.toFloat();
    m_ActivePlanData.m_Cylinder->SetRadius((electrodeDiameter / 2.0) + 0.2);
    //Add to Scene
    SceneManager *scene = Application::GetInstance().GetSceneManager();
    scene->AddObject(m_ActivePlanData.m_CylObj, m_SavedPlansObject);
    m_ActivePlanData.m_CylObj->SetCrossSectionVisible(true); //RIZ20151130: moved here (after contacts are assigned to the scene, IBIS was crashing otherwise
    UpdateActivePlan();
	qDebug() << "Leaving CreateActivePlan";
}


void SEEGAtlasWidget::onCreateAllElectrodesWithSelectedContacts() {
    createAllElectrodesWithSelectedContactsChannels("contacts");
}

void SEEGAtlasWidget::createAllElectrodesWithSelectedContactsChannels(const string whichSelected){
	qDebug() << "Entering createAllElectrodesWithSelectedContactsChannels - "<< whichSelected.c_str();
    // Init saved plan cylinders
    int electTypeIndex = ui->comboBoxElectrodeType->currentIndex();
    this->UpdateConfigurationFromUi();
    double electrodeLength = ui->lineEditCylinderLength->text().toFloat();

    // Delete ALL electrodes
    Application::GetInstance().GetSceneManager()->RemoveAllChildrenObjects(this->m_SavedPlansObject);
    // Create each electrode with ONLY the selected contacts
    for (int iElec=0; iElec<MAX_SEEG_PLANS; iElec++) {
        if (m_AllPlans[iElec].isEntrySet && m_AllPlans[iElec].isTargetSet) {
            // Show channels or contacts depending which was selected
            if (whichSelected.compare("channels") == 0) {
                CreateElectrodeWithSelectedChannels(iElec);
            } else if (whichSelected.compare("contacts") == 0) {
                CreateElectrodeWithSelectedContacts(iElec);
            }
        }
    }
	qDebug() << "Leaving createAllElectrodesWithSelectedContactsChannels";
}

void SEEGAtlasWidget::CreateElectrodeWithSelectedContacts(const int iElec) {

    // electrode points
    Point3D pDeep = m_AllPlans[iElec].targetPoint;
    //Resize3DLineSegmentSingleSide(m_AllPlans[iElec].targetPoint, m_AllPlans[iElec].entryPoint, electrodeLength, pSurface);
    Point3D pSurface = m_AllPlans[iElec].entryPoint;

    // Check if contact is selected
    vector<bool> areContactsSelected;
    std::vector<int > colorsPerContacts;
    vector<string> electrodeNames = m_SEEGElectrodesCohort->GetElectrodeNames();
    ElectrodeInfo::Pointer electrode = m_SEEGElectrodesCohort->GetTrajectoryInBestCohort(electrodeNames[iElec]);
    vector<ContactInfo::Pointer> contacts = electrode->GetAllContacts();
    for (int iCont=0; iCont<contacts.size(); iCont++) {
        ContactInfo::Pointer contact = contacts[iCont];
        // Find out if selected (use variable "isLocationSet")
        bool isContactSel = contact->IsLocationSet();
        areContactsSelected.push_back(isContactSel); // RIZ: OJO!!! CHECK if ORDER IS CORRECT!!!
		qDebug() << "contact: " << iCont <<" - bool: " << isContactSel;

       // double colorsPerContact[3] = {0.75,0.75,0.75}; // default is grey - RIZ: CHANGE to ELECTRODES default??
        int locationValue = 0; // default is 0 - correponds to 0 0 0 = black
        if (isContactSel == true) {
        // Find Color corresponding to Anatomy
             //locationValue = getLocationValue(contact->GetCentralPoint()); // based on pixel value of anatomical image
             locationValue = contact->GetContactLocationValue(); // Based on saved location
        }
        colorsPerContacts.push_back(locationValue);
		qDebug() << "contact: " << iCont << " - color: " << colorsPerContacts[iCont];
    }
    // Create ONLY selected contacts using specified color
    this->CreateElectrodeWithSelectedContacts(iElec, pDeep, pSurface, areContactsSelected, colorsPerContacts);
	qDebug() << "colorsPerContacts[1]: "  << " - color: " << colorsPerContacts[1];

}


void SEEGAtlasWidget::CreateElectrodeWithSelectedContacts(const int iElec, Point3D pDeep, Point3D pSurface, vector<bool> isContactSelected, std::vector<int> colorsPerContacts) {
	qDebug() << "Entering CreateElectrodeWithSelectedContacts - electrode: "<< iElec;

    SceneManager *scene = Application::GetInstance().GetSceneManager();

    QString strDiameter =ui->lineEditCylRadius->text();
    double electrodeDiameter = strDiameter.toFloat();

    m_SavedPlansData[iElec].m_ElectrodeDisplay = CreateCylinderObj(m_AllPlans[iElec].name.c_str(), pDeep, pSurface);
    m_SavedPlansData[iElec].m_ElectrodeDisplay.m_Cylinder->SetRadius((electrodeDiameter / 2.0));
    m_SavedPlansData[iElec].m_ElectrodeDisplay.m_CylObj->SetHidden(true); // DO not SHOW eletrode, only contacts!
    double dcolor[3];
    dcolor[0] = double( m_PosColors[iElec].red() ) / 255.0;
    dcolor[1] = double( m_PosColors[iElec].green() ) / 255.0;
    dcolor[2] = double( m_PosColors[iElec].blue() ) / 255.0;
    m_SavedPlansData[iElec].m_ElectrodeDisplay.m_CylObj->SetColor(dcolor);
    //UpdatePlan(iElec);
    scene->AddObject(m_SavedPlansData[iElec].m_ElectrodeDisplay.m_CylObj, m_SavedPlansObject);
    m_SavedPlansData[iElec].m_ContactsDisplay = CreateContactCylinderObj("contact", pDeep, pSurface, m_ElectrodeModel);
    for (int iContact=0; iContact<m_SavedPlansData[iElec].m_ContactsDisplay.size(); iContact++){
        scene->AddObject(m_SavedPlansData[iElec].m_ContactsDisplay[iContact].m_CylObj, m_SavedPlansData[iElec].m_ElectrodeDisplay.m_CylObj);
		qDebug() << "contact: " << iContact << " is selected? " << isContactSelected[iContact];
        dcolor[0] = labelColors[colorsPerContacts[iContact]][0]; // same as in labelvolumetosurface plugin object
        dcolor[1] = labelColors[colorsPerContacts[iContact]][1]; // same as in labelvolumetosurface plugin object
        dcolor[2] = labelColors[colorsPerContacts[iContact]][2]; // same as in labelvolumetosurface plugin object

        m_SavedPlansData[iElec].m_ContactsDisplay[iContact].m_CylObj->SetColor(dcolor);
		qDebug() << "contact: " << iContact << " - color: " << dcolor[0] << " " << dcolor[1] << " " << dcolor[2];

        if (isContactSelected[iContact] == true) {
            m_SavedPlansData[iElec].m_ContactsDisplay[iContact].m_CylObj->SetHidden(false);
        } else {
            m_SavedPlansData[iElec].m_ContactsDisplay[iContact].m_CylObj->SetHidden(true);
        }

        m_SavedPlansData[iElec].m_ContactsDisplay[iContact].m_Cylinder->SetRadius((electrodeDiameter / 2.0) + 0.3);
        m_SavedPlansData[iElec].m_ContactsDisplay[iContact].m_CylObj->SetCrossSectionVisible(true); //RIZ20151130 moved here (after contacts are assigned to the scene, IBIS was crashing otherwise
    }
    m_SavedPlansData[iElec].m_ElectrodeDisplay.m_CylObj->SetCrossSectionVisible(false); //RIZ20151130: moved here (after contacts are assigned to the scene, IBIS was crashing otherwise
    UpdatePlan(iElec);
    //add also type of electrode
    m_SavedPlansData[iElec].m_ElectrodeModel = m_ElectrodeModel;
	qDebug() << "Leaving CreateElectrodeWithSelectedContacts"<< iElec;
}

void SEEGAtlasWidget::onCreateAllElectrodesWithSelectedChannels(){
    createAllElectrodesWithSelectedContactsChannels("channels");
}

void SEEGAtlasWidget::CreateElectrodeWithSelectedChannels(const int iElec){
    // electrode points
    Point3D pDeep = m_AllPlans[iElec].targetPoint;
    //Resize3DLineSegmentSingleSide(m_AllPlans[iElec].targetPoint, m_AllPlans[iElec].entryPoint, electrodeLength, pSurface);
    Point3D pSurface = m_AllPlans[iElec].entryPoint;

    // Check if channel is selected
    vector<bool> areChannelsSelected;
    std::vector<int > colorsPerChannels;
    vector<string> electrodeNames = m_SEEGElectrodesCohort->GetElectrodeNames();
    ElectrodeInfo::Pointer electrode = m_SEEGElectrodesCohort->GetTrajectoryInBestCohort(electrodeNames[iElec]);
    vector<ChannelInfo::Pointer> channels = electrode->GetAllChannels();
    for (int iChannel=0; iChannel<channels.size(); iChannel++) {
        ChannelInfo::Pointer channel = channels[iChannel];
        // Find out if selected (use variable "isLocationSet")
        bool isChannelSel = channel->IsLocationSet();
        areChannelsSelected.push_back(isChannelSel); // RIZ: OJO!!! CHECK if ORDER IS CORRECT!!!
		qDebug() << "channel: " << iChannel << " - bool: " << isChannelSel;

        int locationValue = 0; // default is 0 - correponds to 0 0 0 = black
        if (isChannelSel == true) {
        // Find Color corresponding to Anatomy
             locationValue = channel->GetChannelLocationValue(); // Based on saved location
        }
        colorsPerChannels.push_back(locationValue);
		qDebug() << "channel: " << channel->GetChannelName().c_str() << " - color: " << colorsPerChannels[iChannel];
    }
    // Create ONLY selected channels using specified color
    this->CreateElectrodeWithSelectedChannels(iElec, pDeep, pSurface, areChannelsSelected, colorsPerChannels);
	qDebug() << "colorsPerChannels[1]: "  << " - color: " << colorsPerChannels[1];

}

void SEEGAtlasWidget::CreateElectrodeWithSelectedChannels(const int iElec, seeg::Point3D pDeep, seeg::Point3D pSurface, vector<bool> isChannelSelected, std::vector<int> colorsPerChannels){
	qDebug() << "Entering CreateElectrodeWithSelectedChannels - electrode: "<< iElec;

    SceneManager *scene = Application::GetInstance().GetSceneManager();
    QString strDiameter =ui->lineEditCylRadius->text();
    double electrodeDiameter = strDiameter.toFloat();
    //Create electrode
    m_SavedPlansData[iElec].m_ElectrodeDisplay = CreateCylinderObj(m_AllPlans[iElec].name.c_str(), pDeep, pSurface);
    m_SavedPlansData[iElec].m_ElectrodeDisplay.m_Cylinder->SetRadius((electrodeDiameter / 2.0));
    m_SavedPlansData[iElec].m_ElectrodeDisplay.m_CylObj->SetHidden(true); // DO not SHOW eletrode, only contacts!
    double dcolor[3];
    dcolor[0] = double( m_PosColors[iElec].red() ) / 255.0;
    dcolor[1] = double( m_PosColors[iElec].green() ) / 255.0;
    dcolor[2] = double( m_PosColors[iElec].blue() ) / 255.0;
    m_SavedPlansData[iElec].m_ElectrodeDisplay.m_CylObj->SetColor(dcolor);
    //UpdatePlan(iElec);
    scene->AddObject(m_SavedPlansData[iElec].m_ElectrodeDisplay.m_CylObj, m_SavedPlansObject);
    // Create channels
    m_SavedPlansData[iElec].m_ContactsDisplay = CreateChannelCylinderObj("channel", pDeep, pSurface, m_ElectrodeModel);
    for (int iChannel=0; iChannel<m_SavedPlansData[iElec].m_ContactsDisplay.size(); iChannel++){
        scene->AddObject(m_SavedPlansData[iElec].m_ContactsDisplay[iChannel].m_CylObj, m_SavedPlansData[iElec].m_ElectrodeDisplay.m_CylObj);
		qDebug() << "channel: " << iChannel << " is selected? " << isChannelSelected[iChannel];
        dcolor[0] = labelColors[colorsPerChannels[iChannel]][0]; // same as in labelvolumetosurface plugin object
        dcolor[1] = labelColors[colorsPerChannels[iChannel]][1]; // same as in labelvolumetosurface plugin object
        dcolor[2] = labelColors[colorsPerChannels[iChannel]][2]; // same as in labelvolumetosurface plugin object

        m_SavedPlansData[iElec].m_ContactsDisplay[iChannel].m_CylObj->SetColor(dcolor);
		qDebug() << "channel: " << iChannel << "-" << iChannel + 1 << " - color: " << dcolor[0] << " " << dcolor[1] << " " << dcolor[2];

        if (isChannelSelected[iChannel] == true) {
            m_SavedPlansData[iElec].m_ContactsDisplay[iChannel].m_CylObj->SetHidden(false);
        } else {
            m_SavedPlansData[iElec].m_ContactsDisplay[iChannel].m_CylObj->SetHidden(true);
        }

        m_SavedPlansData[iElec].m_ContactsDisplay[iChannel].m_Cylinder->SetRadius((electrodeDiameter / 2.0) + 0.3);
        m_SavedPlansData[iElec].m_ContactsDisplay[iChannel].m_CylObj->SetCrossSectionVisible(true); //RIZ20151130 moved here (after contacts are assigned to the scene, IBIS was crashing otherwise
    }
    m_SavedPlansData[iElec].m_ElectrodeDisplay.m_CylObj->SetCrossSectionVisible(false); //RIZ20151130: moved here (after contacts are assigned to the scene, IBIS was crashing otherwise
    UpdatePlan(iElec);
    //add also type of electrode
    m_SavedPlansData[iElec].m_ElectrodeModel = m_ElectrodeModel;
	qDebug() << "Leaving CreateElectrodeWithSelectedChannels"<< iElec;
}


CylinderDisplay SEEGAtlasWidget::CreateCylinderObj(QString name, seeg::Point3D p1, seeg::Point3D p2){
    CylinderDisplay cylObj;
    cylObj.m_Line = vtkSmartPointer<vtkLineSource>::New();
    cylObj.m_Line->SetResolution(30);
    cylObj.m_Line->SetPoint1(p1[0], p1[1], p1[2]);
    cylObj.m_Line->SetPoint2(p2[0], p2[1], p2[2]);
    cylObj.m_Cylinder = vtkSmartPointer<vtkTubeFilter>::New();
    cylObj.m_Cylinder->SetInputConnection(cylObj.m_Line->GetOutputPort());
    cylObj.m_Cylinder->SetNumberOfSides(20);
    cylObj.m_Cylinder->CappingOn();
    cylObj.m_CylObj = PolyDataObject::New();
    cylObj.m_CylObj->SetName(name);
    cylObj.m_Cylinder->Update();
    cylObj.m_CylObj->SetPolyData( cylObj.m_Cylinder->GetOutput() );
    cylObj.m_CylObj->SetCanEditTransformManually(false);
    cylObj.m_CylObj->SetCanAppendChildren(false);
    cylObj.m_CylObj->SetOpacity(1); //before it was 0.3 semi-transparent
    cylObj.m_CylObj->SetHidden(false);
    cylObj.m_CylObj->SetListable(true); //RIZ changed to true if we want them to appear from the beggining - changed for now until problem iwht IBIS is solved!
    cylObj.m_CylObj->SetLineWidth(m_ElectrodeDisplayWidth);
    // cylObj.m_CylObj->SetCrossSectionVisible(true); //RIZ: for now false since IBIS does not take it if default is true!
    //cylObj.m_CylObj->SetCrossSectionVisible(true);
   // cylObj.m_CylObj->MarkModified();

    return cylObj;
}

vector<CylinderDisplay> SEEGAtlasWidget::CreateContactCylinderObj(QString gralName, Point3D electrodeTip, Point3D entryPoint, SEEGElectrodeModel::Pointer electrodeModel){
    vector<CylinderDisplay> contacts;
    double grey[] = {0.75,0.75,0.75};

   Point3D p1,p2;
   for (int iContact=0; iContact<electrodeModel->GetNumContacts(); iContact++){
       electrodeModel->CalcContactStartEnds(iContact, electrodeTip, entryPoint, p1, p2);
       float defaultRadius = electrodeModel->GetContactDiameter()/2 + 0.3;
       CylinderDisplay contact = CreateCylinderObj(gralName + QString::number(iContact+1), p1, p2); //Contacts name is electrode name + 1,2,3 (not 0)
       contact.m_CylObj->SetColor(grey);
       contact.m_Cylinder->SetRadius(defaultRadius);
       contact.m_CylObj->SetHidden(false);
       contact.m_CylObj->SetListable(false);
//       contact.m_CylObj->SetCrossSectionVisible(false);  //RIZ20151130: Move after object is added to the scene - IBIS CRASHES if it is before
       //contact.m_CylObj->SetCrossSectionVisible(true);
       contacts.push_back(contact);
   }
   return contacts;
}

vector<CylinderDisplay> SEEGAtlasWidget::CreateChannelCylinderObj(QString gralName, Point3D electrodeTip, Point3D entryPoint, SEEGElectrodeModel::Pointer electrodeModel){
    vector<CylinderDisplay> channels;
    double magenta[] = {1, 0, 1};

   Point3D pCentral, p1, p2;
   for (int iChannel=0; iChannel<(electrodeModel->GetNumContacts() - 1); iChannel++){ // There are (NumContacts - 1) Channels
       pCentral = electrodeModel->CalcChannelCenterPosition(iChannel, electrodeTip, entryPoint, 0);
       float defaultRadius = electrodeModel->GetContactDiameter()/2 ;
       p1[0] = pCentral[0] - defaultRadius;
       p1[1] = pCentral[1] - defaultRadius;
       p1[2] = pCentral[2] - defaultRadius;
       p2[0] = pCentral[0] + defaultRadius;
       p2[1] = pCentral[1] + defaultRadius;
       p2[2] = pCentral[2] + defaultRadius;
       CylinderDisplay channel = CreateCylinderObj(gralName + QString::number(iChannel+1)+"-"+ QString::number(iChannel+2), p1, p2); //Channels name is electrode name + 1-2,2-3,3-4 (not 0)
       channel.m_CylObj->SetColor(magenta);
       channel.m_Cylinder->SetRadius(defaultRadius + 0.3);
       channel.m_CylObj->SetHidden(false);
       channel.m_CylObj->SetListable(false);
//       channel.m_CylObj->SetCrossSectionVisible(false);  //RIZ20151130: Move after object is added to the scene - IBIS CRASHES if it is before
       //channel.m_CylObj->SetCrossSectionVisible(true);
       channels.push_back(channel);
   }
   return channels;
}

/**********************************************************************
 *  QT SLOTS
 **********************************************************************/

// INPUT DATA
void SEEGAtlasWidget::onLoadSEEGDatasetsFromDir()  {
    onLoadSEEGDatasetsFromDir(QString(""));
}


void SEEGAtlasWidget::onLoadSEEGDatasetsFromDir(QString dirName)  {

    if (dirName == QString("")) {
        dirName = QFileDialog::getExistingDirectory( this, tr("Open Base Directory"), "/", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks | QFileDialog::DontUseNativeDialog);
        if (dirName == QString("")) {
            return;
        }
    }
    //  cleanup
    //Application::GetInstance().GetSceneManager()->RemoveObjectById(m_T1objectID);
    Application::GetInstance().GetSceneManager()->RemoveAllChildrenObjects(this->m_TrajPlanMainObject);
    Application::GetInstance().GetSceneManager()->RemoveAllChildrenObjects(this->m_SavedPlansObject);
    ResetElectrodes();
    RefreshAllPlanCoords();

    // Load dataset on the SEEGFileHelper and IBIS
    ui->labelDirBase->setText(dirName);
    AutoLoadSEEGFromBaseDir(dirName.toStdString(), string(""));
    RefreshVisualization(); // Loads anatomical and vascular data
}

/***
    INPUT POINTS
***/
void SEEGAtlasWidget::onSetTargetPointFromCursor() {

    Application &app = Application::GetInstance();
    SceneManager *scene = app.GetSceneManager();
    double pos_t1[3];
    scene->GetCursorPosition(pos_t1);
    int iElec = ui->comboBoxPlanSelect->currentIndex();

    m_AllPlans[iElec].targetPoint[0] = pos_t1[0];
    m_AllPlans[iElec].targetPoint[1] = pos_t1[1];
    m_AllPlans[iElec].targetPoint[2] = pos_t1[2];
    m_AllPlans[iElec].isTargetSet = true;

    onUpdateElectrode(iElec);
   // this->RefreshPlanCoords(iElec);
}

void SEEGAtlasWidget::onSetTargetPointFrom3DCoords() {

    Point3DInputDialog dlg;
    Point3D initialTarget;
    initialTarget.Fill(0);

    int iElec = ui->comboBoxPlanSelect->currentIndex();
    if (m_AllPlans[iElec].isTargetSet) {
        initialTarget =  m_AllPlans[iElec].targetPoint;
    }
    dlg.setPoint3D(initialTarget);

    if (dlg.exec() == QDialog::Accepted) {
        Point3D p = dlg.getPoint3D();
        m_AllPlans[iElec].targetPoint = p;
        onUpdateElectrode(iElec);
    }
}

void SEEGAtlasWidget::onSetEntryPointFrom3DCoords() {

    Point3DInputDialog dlg;

    Point3D initialEntry;
    initialEntry.Fill(0);

    int iElec = ui->comboBoxPlanSelect->currentIndex();
    if (m_AllPlans[iElec].isEntrySet) {
        initialEntry =  m_AllPlans[iElec].entryPoint;
    }

    dlg.setPoint3D(initialEntry);
    if (dlg.exec() == QDialog::Accepted) {
        Point3D p = dlg.getPoint3D();
        m_AllPlans[iElec].entryPoint = p;
        m_AllPlans[iElec].isEntrySet = true;
        onUpdateElectrode(iElec);
    }
}

void SEEGAtlasWidget::onSetEntryPointFrom3DSelection() {
    double *ep = m_ActivePlanData.m_Line->GetPoint2();
    int iElec = ui->comboBoxPlanSelect->currentIndex();
    m_AllPlans[iElec].entryPoint[0] = ep[0];
    m_AllPlans[iElec].entryPoint[1] = ep[1];
    m_AllPlans[iElec].entryPoint[2] = ep[2];
    m_AllPlans[iElec].isEntrySet = true;
    onUpdateElectrode(iElec);
}

void SEEGAtlasWidget::onSetEntryPointFromCursor() {
    Application &app = Application::GetInstance();
    SceneManager *scene = app.GetSceneManager();
    double pos_t1[3];
    scene->GetCursorPosition(pos_t1);
    int iElec = ui->comboBoxPlanSelect->currentIndex();
    m_AllPlans[iElec].entryPoint[0] = pos_t1[0];
    m_AllPlans[iElec].entryPoint[1] = pos_t1[1];
    m_AllPlans[iElec].entryPoint[2] = pos_t1[2];
    m_AllPlans[iElec].isEntrySet = true;
    onUpdateElectrode(iElec);
}

void SEEGAtlasWidget::onUpdateElectrode(int iElec) {
    if( iElec < 0 ) return;

    if (m_AllPlans[iElec].isTargetSet && m_AllPlans[iElec].isEntrySet) {
        // Add electrode to cohort
        addElectrodeToCohort(iElec);

        // Add to table
        if(iElec>=m_VectorContactsTables.size()) { //if not in table is NEW electrode -> ADD
            int indTab = iElec+1;
            addTrajectoryTableTab(indTab, m_AllPlans[iElec].name); //Add TAB for table information (Name is assign in changeElectrodeNames(string) and it will be populated when RefreshTrajectories is called)
        } else { //if index already exist - update name if necessary
            onChangeElectrodeName();
        }
        //Add info also to Table
        if (ui->pushButtonShowContactsTable->isChecked()){
            RefreshContactsTable(iElec);
        } else {
            RefreshChannelsTable(iElec);
        }

    }

    //Create All Electrodes (including active) again
    CreateAllElectrodes();

    // Refresh combo and textboxes info
    this->RefreshPlanCoords(iElec, m_AllPlans[iElec].name);
   // this->RefreshPlanCoords(iElec);
}

void SEEGAtlasWidget::onUpdateElectrode(int iElec, seeg::ElectrodeInfo::Pointer electrode) {
    if (m_AllPlans[iElec].isTargetSet && m_AllPlans[iElec].isEntrySet) {
        // Add electrode to cohort
        addElectrodeToCohort(iElec, electrode);

        // Add to table
        if(iElec>=m_VectorContactsTables.size()) { //if not in table is NEW electrode -> ADD
            int indTab = iElec+1;
            addTrajectoryTableTab(indTab, m_AllPlans[iElec].name); //Add TAB for table information (Name is assign in changeElectrodeNames(string) and it will be populated when RefreshTrajectories is called)
        } else { //if index already exist - update name if necessary
            onChangeElectrodeName();
        }
        //Add info also to Table
        if (ui->pushButtonShowContactsTable->isChecked()){
            RefreshContactsTable(iElec);
        } else {
            RefreshChannelsTable(iElec);
        }
    }
    //Create All Electrodes (including active) again
    CreateAllElectrodes();
    //Refresh display of electrodes
    RefreshPlanCoords(iElec, m_AllPlans[iElec].name);
}

void SEEGAtlasWidget::addElectrodeToCohort(int iElec) {
    bool onlyInsdeBrain = false; // RIZ20151227 since entry point could be specified inside the brain, compute contact position also "outside"
    //if no name ask for it
    if (m_AllPlans[iElec].name.compare("") == 0) {
        NameInputDialog dlg;
        if (dlg.exec() == QDialog::Accepted) {
            m_AllPlans[iElec].name = dlg.getName();
        }
    }
    //Create electrode
    ElectrodeInfo::Pointer electrode = ElectrodeInfo::New(m_AllPlans[iElec].entryPoint, m_AllPlans[iElec].targetPoint, m_ElectrodeModel, m_AllPlans[iElec].name);
    //Add contacts to electrode
    vector<Point3D> allContactsCentralPt;
    m_ElectrodeModel->CalcAllContactPositions(electrode->m_TargetPointWorld, electrode->m_EntryPointWorld, allContactsCentralPt, onlyInsdeBrain); //RIZ20151227 corrected - target and entry swaped
    electrode->AddAllContactsAndChannelsToElectrode(allContactsCentralPt);
    //electrode->AddAllContactsToElectrode (allContactsCentralPt);

    //Add to cohort
    addElectrodeToCohort(iElec, electrode);
    ReplaceElectrodeName(iElec, m_AllPlans[iElec].name);
}

void SEEGAtlasWidget::addElectrodeToCohort(int iElec, ElectrodeInfo::Pointer electrode) {
    //Add electrode to cohort
    //SEEGElectrodesCohort::Pointer electrodesCohort = GetSEEGElectrodesCohort();
    m_SEEGElectrodesCohort->AddTrajectoryToBestCohort(m_AllPlans[iElec].name, electrode, iElec); //RIZ20151124 - added

	qDebug() << " Electrode Type: " << m_ElectrodeModel->GetElectrodeId().c_str();
    m_SEEGElectrodesCohort->SetElectrodeModel(electrode->GetElectrodeModel());
}

void SEEGAtlasWidget::onFindAnatLocation(){
    //Find anatomical location for all electrodes
    //SEEGElectrodesCohort::Pointer electrodesCohort = GetSEEGElectrodesCohort();
    int nElec = m_SEEGElectrodesCohort->GetNumberOfElectrodesInCohort();
    vector<string> electrodeNames = m_SEEGElectrodesCohort->GetElectrodeNames();
    for (int iElec=0; iElec<nElec; iElec++) {
        ElectrodeInfo::Pointer electrode = m_SEEGElectrodesCohort->GetTrajectoryInBestCohort(electrodeNames[iElec]);
        int indexElectrode = m_SEEGElectrodesCohort->GetTrajectoryIndexInBestCohort(electrodeNames[iElec]);
        cout<<"Fidning Anat location per contact -  "<< electrodeNames[iElec] <<" = el: "<<indexElectrode<<std::endl;
        onFindAnatLocation(electrode);
        //Refresh table
        RefreshContactsTable(indexElectrode);
    }
    ui->pushButtonShowContactsTable->setChecked(true);
    ui->pushButtonShowContactsTable->setText(QString("Showing CONTACTS in table"));
}


void SEEGAtlasWidget::onFindAnatLocation(seeg::ElectrodeInfo::Pointer electrode){
    // Find anatomical location for all contacts of the eletrode
    int nContacts = electrode->GetNumberOfContacts();
    FloatVolume::Pointer anatLabelsVol = openAtlasVolume();
    if( anatLabelsVol.IsNotNull() ) {
        map<int,string> labelsMap = ReadAtlasLabels();
		qDebug() << " Electrode Type: " <<  m_ElectrodeModel->GetElectrodeId().c_str();
        SEEGContactsROIPipeline::Pointer pipelineContacts = SEEGContactsROIPipeline::New( anatLabelsVol, electrode->GetElectrodeModel() ); //volume is only to have size,

        bool useCylinder = ui->checkBoxUseCylinder->isChecked();

        for( int iContact = 0; iContact < nContacts; iContact++ ){
            // Find anatomical location for each contact of the eletrode
            vector<int> labelsCount;
            int voxelsInContactVol;
            labelsCount = pipelineContacts->GetLabelsInContact(iContact, electrode, anatLabelsVol, voxelsInContactVol, useCylinder);
            ContactInfo::Pointer contact = electrode->GetOneContact(iContact);
            //Compute Most common Label (highest count)
            //int probaMaxLabel = std::max_element(labelsCount, labelsCount.size());
            //int indexMaxLabel = std::distance(probaMaxLabel, labelsCount);
            int indexMaxLabel = -1;
            int sumMaxLabel = -1;
            int totalVoxelsLabels = 0;
            for( int iLabel = 0; iLabel < labelsCount.size(); iLabel++ ){
                totalVoxelsLabels += labelsCount[iLabel];
                if( ( labelsCount[iLabel] > 0 ) && ( labelsCount[iLabel] >= sumMaxLabel ) ){  // using > or >= changes the results when 0.5 proba
                    sumMaxLabel = labelsCount[iLabel];
                    indexMaxLabel = iLabel;
                    //float proba = float(sumMaxLabel)/float(totalVoxelsLabels);
                    qDebug() << " Most Common Label so far: " << indexMaxLabel  << " - name:" << labelsMap[indexMaxLabel].c_str() << " with Number of voxels of Occupacy: " << sumMaxLabel << " out of: " << voxelsInContactVol;
                }
            }

            //Assign most common label to contact
            float proba = float( sumMaxLabel ) / float( voxelsInContactVol );
			qDebug() << "Contact index: " << iContact << " Most Common Label number: " << indexMaxLabel << " - name:" << labelsMap[indexMaxLabel].c_str() << " with Percentage Occupacy: " << proba << " (voxels with labels: " << totalVoxelsLabels << " - # labels: " << labelsCount.size() << " )";
            if( !isnan( proba ) ) {
                if( labelsMap.size() > 0 ){
                    contact->SetContactLocation( labelsMap[indexMaxLabel], indexMaxLabel, proba );
                }else{
                    stringstream ssLabel;
                    ssLabel << indexMaxLabel;
                    contact->SetContactLocation( ssLabel.str(), indexMaxLabel, proba );
                }
            }
        }
    }
}


void SEEGAtlasWidget::onFindChannelsAnatLocation(){
    //Find anatomical location for all electrodes
    qDebug() <<"onFindChannelsAnatLocation ";
    //SEEGElectrodesCohort::Pointer pathCohortObj = GetSEEGElectrodesCohort();
    int nElec = m_SEEGElectrodesCohort->GetNumberOfElectrodesInCohort();
    vector<string> electrodeNames = m_SEEGElectrodesCohort->GetElectrodeNames();
    bool saveChannelInVol = ui->checkBoxSaveChannelVolume->isChecked();

    QString dirname = QString("");
    if (saveChannelInVol == true) { // Select directory where to save channel model volumes.
        QString baseDir = ui->labelDirBase->text();
        dirname = QFileDialog::getExistingDirectory(this, tr("Select Directory where Channel Model volumes go"), baseDir, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks | QFileDialog::DontUseNativeDialog);
    }
    for (int iEl=0; iEl<nElec; iEl++) {
        ElectrodeInfo::Pointer electrode = m_SEEGElectrodesCohort->GetTrajectoryInBestCohort(electrodeNames[iEl]);
        int indexElectrode = m_SEEGElectrodesCohort->GetTrajectoryIndexInBestCohort(electrodeNames[iEl]);
		qDebug() <<"Finding Anat location per channel -  " << electrodeNames[iEl].c_str() <<" = el: "<<indexElectrode;
        onFindChannelsAnatLocation(electrode, dirname);
        //Refresh table
        RefreshChannelsTable(indexElectrode);
    }
    ui->pushButtonShowContactsTable->setChecked(false);
    ui->pushButtonShowContactsTable->setText(QString("Showing CHANNELS in table"));
}

void SEEGAtlasWidget::onFindChannelsAnatLocation(seeg::ElectrodeInfo::Pointer electrode, const QString dirname){
    // Find anatomical location for all bipolar channel of the eletrode
    // Bipolar channel is defined as contact i to contact i+1
    int nContacts = electrode->GetNumberOfContacts();
	qDebug() << "onFindChannelsAnatLocation - number of contacts: "<<nContacts;
    FloatVolume::Pointer anatLabelsVol = openAtlasVolume();
   if (anatLabelsVol.IsNotNull()) {
	   qDebug() << " Electrode Type: " <<  m_ElectrodeModel->GetElectrodeId().c_str();
        SEEGContactsROIPipeline::Pointer pipelineContacts = SEEGContactsROIPipeline::New(anatLabelsVol, electrode->GetElectrodeModel()); //volume is only to have size,
        // Find anatomical location for each bipolar channel of the eletrode (nChannels = nContacts-1)
        bool useCylinder = ui->checkBoxUseCylinder->isChecked();
        map <int,string> labelsMap = ReadAtlasLabels();
        for (int iContact=0; iContact<(nContacts-1); iContact++) {
			qDebug() <<"onFindChannelsAnatLocation - contact: " << iContact;
            vector<int> labelsCount;
            labelsCount = pipelineContacts->GetLabelsInChannel(iContact, iContact +1, electrode, anatLabelsVol, useCylinder);
            ChannelInfo::Pointer channel = electrode->GetOneChannel(iContact); // channel's index is the forst of its 2 contacts.
            //Compute Most common Label (highest count)
            //int probaMaxLabel = std::max_element(labelsCount, labelsCount.size());
            //int indexMaxLabel = std::distance(probaMaxLabel, labelsCount);
            int indexMaxLabel=-1;
            int sumMaxLabel=-1;
            int totalVoxelsLabels =0;
            for (int iLabel=0; iLabel<labelsCount.size(); iLabel++) {
                totalVoxelsLabels+= labelsCount[iLabel];
                if (labelsCount[iLabel] >sumMaxLabel) {
                    sumMaxLabel = labelsCount[iLabel];
                    indexMaxLabel = iLabel;
                }
            }

            //Assign most common label to channel
            int voxelsInChannel = pipelineContacts->GetNumberVoxelsInChannelModel();

            float proba = float(sumMaxLabel)/float(voxelsInChannel);
			qDebug() <<"Contact 1 index: "<< iContact<<" - Contact 2 index: "<< iContact +1<< " - Channel Most Common Label: " << indexMaxLabel <<" with Percentage Occupacy: "<< proba<< " (total: "<<voxelsInChannel<<" voxels)";
            if (!isnan(proba)) {
                if ( labelsMap.size() >0) {
                    channel->SetChannelLocation(labelsMap[indexMaxLabel], indexMaxLabel, proba);
                } else {
                    stringstream ssLabel;
                    ssLabel << indexMaxLabel;
                    channel->SetChannelLocation(ssLabel.str(), indexMaxLabel, proba);
                }
            }

            // Save channel as volume in MINC file
            if (dirname != QString("")) {
                FloatVolume::Pointer channelModelVol = pipelineContacts->GetChannelModelVol();

                // Save minc file
                string filename = dirname.toStdString() + "/"+ string(FILE_CHANNELS_GRAL) + "_" + channel->GetChannelName()  + ".mnc";
                WriteFloatVolume(filename, channelModelVol);
            }
        }
    }
}

/***
    PLAN RELATED
***/
// PLAN selection and Manual generation
void SEEGAtlasWidget::onPlanSelect(int iElec) {
    if (iElec < m_VectorContactsTables.size()){
        string electrodeName = m_AllPlans[iElec].name;
        if (electrodeName.compare("") != 0) {
            RefreshPlanCoords(iElec, electrodeName);
        } else {
            ui->lineEditElectrodeName->setText(QString(""));
            RefreshPlanCoords(iElec);
        }
    } else {
        ui->lineEditElectrodeName->setText(QString(""));
        RefreshPlanCoords(iElec);
    }
    CreateActivePlan(); // Update the active plan to the selected electrode
}

void SEEGAtlasWidget::onChangeElectrodeName() { //RIZ20151212 CHECK!
    QString qsElectrodeName = ui->lineEditElectrodeName->text();
    //int iElec = ui->comboBoxPlanSelect->currentIndex();
    //addElectrodeToCohort(iElec);
    onChangeElectrodeName(qsElectrodeName.toStdString());
//    RefreshTrajectories();
}

void SEEGAtlasWidget::onChangeElectrodeName(const string newElectrodeName) {
    // change electrode name that corresponds to selected plan
    int iElec = ui->comboBoxPlanSelect->currentIndex();
    //Update Name in Cohort!
    string oldName =  m_AllPlans[iElec].name;
    if (!oldName.compare(newElectrodeName)) {
        //nothing new
        return;
    }
    //SEEGElectrodesCohort::Pointer cohort = GetSEEGElectrodesCohort();
    m_SEEGElectrodesCohort->ChangeTrajectoryNameInBestCohort(oldName, newElectrodeName);

    // Replace in GUI
    ReplaceElectrodeName(iElec, newElectrodeName);
    m_AllPlans[iElec].name = newElectrodeName;

    // update name in table
    //Tab 0 contains the list of electrodes
    // RIZ20151124 - FALTA actualizar nombre en All Electrodes tab

    //-> Tab iElec+1 corresponds to the electrode's tab(the one with contactcs information)
    int indTab = iElec+1;
    ui->tabWidgetScores->setTabText(indTab, QString(newElectrodeName.c_str()));
//    RefreshTrajectories();
    // update name in list
    RefreshPlanCoords(iElec, newElectrodeName);
}


// RESULTS INTERACTION (Tables in Tabs)
void SEEGAtlasWidget::onTabSelect(int indTab) {

    // clear all selected contact in tables
    for (int iTab = 0; iTab < m_VectorContactsTables.size(); ++iTab)
    {
        m_VectorContactsTables[iTab]->clearSelection();
    }
    ui->tabWidgetScores->setCurrentIndex(indTab);
    int iElec = indTab-1; //RIZ: first TAB of table is BEST trajectories, subsequent are contacts information for each electrode
    if (iElec>=0 && iElec != ui->comboBoxPlanSelect->currentIndex()) {
        onPlanSelect(iElec);
        //RefreshPlanCoords(iElec); //RIZ20151210 test if this is sufficient
    }
    ui->pushButtonUpdateContactPosition->setEnabled(false);
    for (int iElec = 0; iElec < GetNumberElectrodes(); ++iElec)
    {
        if(m_SavedPlansData[iElec].m_PointRepresentation)
            m_SavedPlansData[iElec].m_PointRepresentation->SelectPoint(-1);
    }
}

void SEEGAtlasWidget::onShowContactsChannelsTable(bool isChecked){
    //SEEGElectrodesCohort::Pointer pathCohortObj = GetSEEGElectrodesCohort();
    int nElec = m_SEEGElectrodesCohort->GetNumberOfElectrodesInCohort();
    for (int iElec=0; iElec<nElec; iElec++) {
        if (isChecked == true) {
            //    if (ui->pushButtonShowContactsTable->isChecked()){
            RefreshContactsTable(iElec);
            ui->pushButtonShowContactsTable->setText(QString("Showing CONTACTS in table"));
        } else {
            RefreshChannelsTable(iElec);
            ui->pushButtonShowContactsTable->setText(QString("Showing CHANNELS in table"));
        }
    }
}


void SEEGAtlasWidget::onTrajectoryTableCellChange(int newRow, int newCol, int oldRow, int oldCol) {
    int indTab = ui->tabWidgetScores->currentIndex();
    if (indTab == 0) {
    //First TAB of table is Table with electrode information, select electrode from here
        onAllElectrodesTableCellChange(newRow, newCol, oldRow, oldCol);
    } else {
    //First TAB of table is Table with electrode information, subsequent are contact lists per electrode
        int iLoc = indTab-1;
        onTrajectoryTableCellChange(newRow, newCol, oldRow, oldCol, iLoc);
    }
}

void SEEGAtlasWidget::onAllElectrodesTableCellChange(int newRow, int newCol, int oldRow, int oldCol) {
    //Generalization to be able to update N tabs
    // - uses vector<QTableWidget> to have which tab corresponds to which table -> see where to assign!
    if (newRow == oldRow) {return;}
    if (newRow < 0) {return;}

    int iElec = newRow; //Row represent an electrode

	qDebug() << "onTrajectoryTableCellChange Electrode "<< iElec;

    RefreshPlanCoords(iElec);
}

void SEEGAtlasWidget::onTrajectoryTableCellChange(int newRow, int newCol, int oldRow, int oldCol, int iElec) {
    //Generalization to be able to update N tabs
    // Tabs other than the first contain information regarding every contact of each electrode (1 electrode per tab)
    // - uses vector<QTableWidget> to have which tab corresponds to which table -> see where to assign!
    if (newRow == oldRow) {return;}
    if (newRow < 0) {return;}
    QTableWidget* table = m_VectorContactsTables[iElec];
    if (iElec > m_VectorContactsTables.size() || iElec<0) {return;}

    int iContact = newRow; //Row represent the contact

    // Each row is one contact -> go to the xyz position of the selected contact

    //First column is electrode name
    string contactName = table->item(newRow, 0)->text().toStdString();

    // 2-4 columns are xyz
    double contactPosition[3];
    for (int i=0; i<3; i++){ //column 0 is name / 1-3 columns are target / 4-6 columns are entry
        contactPosition[i] = table->item(newRow, i+1)->text().toFloat();
    }

    //Set cursor's position to contact's point
    Application &app = Application::GetInstance();
    SceneManager *scene = app.GetSceneManager();
    scene->SetCursorWorldPosition(contactPosition);

    for (int i = 0; i < GetNumberElectrodes(); ++i)
    {
        if(ui->pushButtonShowContactsTable->isChecked())
        {
            if( i == iElec )
                m_SavedPlansData[iElec].m_PointRepresentation->SelectPoint(iContact);
            else
                m_SavedPlansData[i].m_PointRepresentation->SelectPoint(-1);
        }
        else
        {
            m_SavedPlansData[i].m_PointRepresentation->SelectPoint(-1);
        }
    }

    if(ui->pushButtonShowContactsTable->isChecked())
    {
        ui->pushButtonUpdateContactPosition->setEnabled(true);
    }


	qDebug() <<"onTrajectoryTableCellChange Contact " << contactName.c_str() << " - "<<iContact<<" - Position:"<<contactPosition[0]<<" "<< contactPosition[1]<<" "<<contactPosition[2];

   // RefreshPlanCoords(iElec); // RIZ20151217 Not sure if this should be here
}

 void SEEGAtlasWidget::addTrajectoryTableTab(const int indTab, const string electrodeName) {
     // Create dynamically a new table of trajectories for this electrode and add it as a new TAB in tabWidgetScores

     // string electrodeName = *itLoc;
     ContactsListTableWidget *trajListWidgetPerTarget = new (ContactsListTableWidget);
     //ui->tabWidgetScores->insertTab(iLoc, trajListWidgetPerTarget, QString("Score ")+QString(electrodeName.c_str()));
     //ui->tabWidgetScores->insertTab(indTab, trajListWidgetPerTarget, QString(""));
     QTableWidget* table = trajListWidgetPerTarget->findChild<QTableWidget *>();
     trajListWidgetPerTarget->setParent(ui->tabWidgetScores);
     table->setEditTriggers(QAbstractItemView::NoEditTriggers);
     table->setSelectionBehavior(QAbstractItemView::SelectRows);
     table->setSelectionMode(QAbstractItemView::SingleSelection);

      QObject::connect(table, SIGNAL(currentCellChanged(int,int,int,int)),
                      this, SLOT(onTrajectoryTableCellChange(int, int, int, int)));
     const bool isBlocked = ui->tabWidgetScores->blockSignals(true);
     ui->tabWidgetScores->insertTab(indTab, trajListWidgetPerTarget, QString(electrodeName.c_str()));
     ui->tabWidgetScores->blockSignals(isBlocked);
     ui->tabWidgetScores->setTabEnabled(indTab, true);
     m_VectorContactsTables.push_back(table);
 }

 void SEEGAtlasWidget::removeContactsTableTab(const int indTab) {
     const bool isBlocked = ui->tabWidgetScores->blockSignals(true);
     ui->tabWidgetScores->removeTab(indTab);
     ContactsListTableWidget * tabContent = ui->tabWidgetScores->findChild<ContactsListTableWidget *>();
     //QWidget *tabContent = ui->tabWidgetScores->widget(indTab);
     if( tabContent ) {
         delete tabContent;
     }
     ui->tabWidgetScores->setCurrentIndex(0);
     ui->tabWidgetScores->blockSignals(isBlocked);
 }

 void SEEGAtlasWidget::onLoadPlanningFromDirectory() {
    onLoadPlanningFromDirectory(QString(""));
 }

 void SEEGAtlasWidget::onLoadPlanningFromDirectory(QString dirName) {
     
     Q_ASSERT(m_pluginInterface);
     IbisAPI * ibisApi = m_pluginInterface->GetIbisAPI();

     //ask for the directory where all the files with each planning are (one per target)
    if (dirName == QString("")) {
        QString baseDir = ui->labelDirBase->text();
        dirName = QFileDialog::getExistingDirectory(this, tr("Select Directory where Planning Files are"), baseDir, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks | QFileDialog::DontUseNativeDialog);
         if (dirName == QString("")) {
             return;
         }
    }

     // cleanup electrodes
     ResetElectrodes();

     m_SEEGElectrodesCohort = SEEGElectrodesCohort::New(m_ElectrodeModel, m_SpacingResolution); // create again to remove any previous electrode
     //SEEGElectrodesCohort::Pointer pathCohortObj = GetSEEGElectrodesCohort();
     vector<string> electrodeNames;
     int iElec;
     char delimiter(DELIMITER_TRAJFILE);
     // 1. Load Cohort
     string filename = dirName.toStdString() + "/"+ string(FILE_RESULTS_TRAJ_BEST) + string(FILE_POS_FIX);
     bool status = m_SEEGElectrodesCohort->LoadSEEGBestCohortDataFromFile(filename, delimiter, m_ElectrodeModelList);
     if (status==false) { // No trajectories found - likely because wrong delimiter (RIZ2016)
         status = m_SEEGElectrodesCohort->LoadSEEGBestCohortDataFromFile (filename, DELIMITER_TRAJFILE_2OPTION, m_ElectrodeModelList);
     }
     if (status==true){

         electrodeNames = m_SEEGElectrodesCohort->GetElectrodeNames();
         // Get electrode type from m_SEEGElectrodesCohort
         m_ElectrodeModel  = m_SEEGElectrodesCohort->GetElectrodeModel();
         m_SpacingResolution = m_SEEGElectrodesCohort->GetSpacingResolution();
         this->UpdateUiFromConfiguration();

         this->UpdateUiFromConfiguration();
         //order in SEEGplanningwidget list MUST be the same!
         // assign info for each trajectory in best cohort

         SetElectrodeNames(electrodeNames);

         // Load also contacts information for each electrode
         map<string,ElectrodeInfo::Pointer>::iterator elecIt;
         map<string,ElectrodeInfo::Pointer> bestCohort;
         bestCohort = m_SEEGElectrodesCohort->GetBestCohort();
         
         QProgressDialog * progress = ibisApi->StartProgress(bestCohort.size(), tr("Loading planning from directory"));

         for (elecIt = bestCohort.begin(), iElec=0; elecIt != bestCohort.end(); elecIt++, iElec++) {
             string elName = elecIt->first;
             ElectrodeInfo::Pointer electrode = elecIt->second;
             filename = dirName.toStdString() + "/"+ string(FILE_RESULTS_TRAJ_GRAL)+ elName  + string(FILE_POS_FIX); 
             status = electrode->LoadElectrodeDataFromFile (filename, delimiter, m_ElectrodeModelList);
             m_AllPlans[iElec].isTargetSet = true;
             m_AllPlans[iElec].isEntrySet = true;
             m_AllPlans[iElec].targetPoint = bestCohort[elName]->GetTargetPoint();
             m_AllPlans[iElec].entryPoint = bestCohort[elName]->GetEntryPoint();
             m_AllPlans[iElec].name = elName;
             onUpdateElectrode(iElec, electrode);
             //this->DisplaySavedPlan(iElec); //it was not here before
             // this->RefreshPlanCoords(iElec, elName);
             ibisApi->UpdateProgress(progress, iElec + 1);
         }



         //Update type of electrode on list
         //int indexType = m_ElectrodeModel->ElectrodeTypeEnumToIndex(m_SEEGElectrodesCohort->GetElectrodeType()); //TODO: replace by get electrode
         //ui->comboBoxElectrodeType->setCurrentIndex(indexType);
         for( int i = 0; i < m_ElectrodeModelList.size(); i++ )
         {
             if(m_ElectrodeModel->GetElectrodeId().compare(m_ElectrodeModelList[i]->GetElectrodeId()) == 0)
             {
                 ui->comboBoxElectrodeType->setCurrentIndex(i);

             }
         }
         
         // this->UpdatePlan(iElec);
         // CreateActivePlan();
         // CreateAllElectrodes();

         ibisApi->StopProgress(progress);
     }

     if( status == false )
     {
         QString message = tr("It seems that an error occurred when attempting to load:\n") +
             dirName + tr("\n") +
             tr("Please make sure that the electrode model specified in the planning exist.");
         ibisApi->Warning(tr("Loading error"), message);
     }

 }

void SEEGAtlasWidget::onLoad1Plan() {

    //Load only 1 plan to selected plan number
    QString baseDir = ui->labelDirBase->text();

    QString qFilename = QFileDialog::getOpenFileName(this,
            tr("Load One Plan (e.g. pos_trajectories_AG.csv"), baseDir,
            tr("IBIS Planning Data (*.csv)"), nullptr, QFileDialog::DontUseNativeDialog);
    if (qFilename == "") {
       return;
   }
    //get current plan from combobox
    int iElec = ui->comboBoxPlanSelect->currentIndex();

    //Load electrode data from file
    string filename = qFilename.toStdString();
    onLoadOnePlanFromCSVFile(iElec, filename);
	qDebug() << "onLoad1Plan Done!";
}

void SEEGAtlasWidget::onLoadOnePlanFromCSVFile(const int iElec, const string filename) {
    ElectrodeInfo::Pointer electrode = ElectrodeInfo::New();
    bool status = electrode->LoadElectrodeDataFromFile (filename, DELIMITER_TRAJFILE, m_ElectrodeModelList);
    if (status==false) {
        status = electrode->LoadElectrodeDataFromFile (filename, DELIMITER_TRAJFILE_2OPTION, m_ElectrodeModelList);
    }
    if (status==true){
        //order in SEEGplanningwidget list MUST be the same!
        // assign info for each trajectory in best cohort
        string electrodeName = electrode->GetElectrodeName();
        ReplaceElectrodeName(iElec, electrodeName);

        // Load also contacts information for each electrode
        m_AllPlans[iElec].isTargetSet = true;
        m_AllPlans[iElec].isEntrySet = true;
        m_AllPlans[iElec].targetPoint = electrode->GetTargetPoint();
        m_AllPlans[iElec].entryPoint = electrode->GetEntryPoint();
        m_AllPlans[iElec].name = electrodeName;
        onUpdateElectrode(iElec, electrode);
        //this->DisplaySavedPlan(iElec); //it was not here before
        // this->RefreshPlanCoords(iElec, elName);
        m_ElectrodeModel  = electrode->GetElectrodeModel();
        //Update type of electrode on list
        this->UpdateUiFromConfiguration();

		qDebug() << electrodeName.c_str() << " Electrode Type: " << electrode->GetElectrodeName().c_str() << " - " << m_ElectrodeModel->GetElectrodeName().c_str() << " index: " << ui->comboBoxElectrodeType->currentIndex();
        

    // this->UpdatePlan(iElec);
   // CreateActivePlan();
    }
	qDebug() << "onLoadOnePlanFromCSVFile " << iElec << " Done!";

 }

void SEEGAtlasWidget::onSavePlanningToDirectory() {
    onSavePlanningToDirectory(QString(""));
}

void SEEGAtlasWidget::onSavePlanningToDirectory(QString dirName) {
    // Save one file with all electrodes and one file per electrode with contacts' information
    Q_ASSERT(m_pluginInterface);

    IbisAPI * api = m_pluginInterface->GetIbisAPI();
    if (dirName == QString("")) {
        dirName = api->GetExistingDirectory(tr("Select Directory to Save Electrode Files"), api->GetWorkingDirectory());
    }
    if (dirName != QString("")) {
        //dirname.append("/AllTrajectories");
        QDir dir(dirName); //        QDir().mkdir(dirname); // In case it does not exist
        if (!dir.exists()) {
            bool status = dir.mkdir(dirName);
        }
		qDebug() << "... Saving electrodes in dir:" << dirName.toStdString().c_str();

        // 1. Save also Best Cohort
        //SEEGElectrodesCohort::Pointer pathCohortObj = GetSEEGElectrodesCohort();
        vector<string> electrodeNames= m_SEEGElectrodesCohort->GetElectrodeNames();
        if (electrodeNames.size()>0) { //unless there are NO trajectories whatsover
            string filename = dirName.toStdString() + "/"+ string(FILE_RESULTS_TRAJ_BEST) + string(FILE_POS_FIX);
            m_SEEGElectrodesCohort->SaveSEEGBestCohortDataToFile(filename, DELIMITER_TRAJFILE);
        } else {
			qDebug() <<"No Electrodes to Save...";
            //electrodeNames = this->GetElectrodeNames();
        }

        // 2. Save also information of all contacts for each electrode
        map<string,ElectrodeInfo::Pointer>::iterator elecIt;
        map<string,ElectrodeInfo::Pointer> bestCohort;
        bestCohort = m_SEEGElectrodesCohort->GetBestCohort();
        QProgressDialog *progress = api->StartProgress(bestCohort.size(), tr("Saving electrodes in dir: ") + dirName);
        int i;
        for (elecIt = bestCohort.begin(), i = 0; elecIt != bestCohort.end(); elecIt++, i++) {
            string elName = elecIt->first;
            ElectrodeInfo::Pointer el = elecIt->second;
            string filename = dirName.toStdString() + "/"+ string(FILE_RESULTS_TRAJ_GRAL)+ elName  + string(FILE_POS_FIX);
			qDebug() <<"Saving electrode: " << elName.c_str() <<" in file: "<< filename.c_str();

            el->SaveElectrodeDataToFile (filename, DELIMITER_TRAJFILE);
            api->UpdateProgress(progress, i);
        }
        api->StopProgress(progress);
    }
}


// PROBE EYE
void SEEGAtlasWidget::onProbeEyeView() {
// RIZ:TODO: ADD also each contact's position
    int sel = ui->comboBoxPlanSelect->currentIndex();
    int mode = ui->comboBoxProbeEyeSelect->currentIndex();
    m_TrajVisWidget->Configure((TRAJ_VIS_MODE)mode);
    m_TrajVisWidget->ShowTrajectory(m_ActivePlanData.m_Line->GetPoint1(),
                                    m_ActivePlanData.m_Line->GetPoint2());
}

// VISUALIZATION OPTIONS
void SEEGAtlasWidget::onChangeTrajectoryCylinderRadius(int sliderPos) {
    float newDiameter;
    newDiameter = sliderPos / 100.0 * 10.0;
    ui->lineEditCylRadius->setText(QString::number(newDiameter));
    CreateAllElectrodes();
}

void SEEGAtlasWidget::onChangeTrajectoryCylinderRadius(QString strDiameter) {
    double newDiameter = strDiameter.toFloat();
    double sliderPos = newDiameter / 10.0 * 100.0;
    if (sliderPos != ui->horizontalSliderCylRadius->value()){
        ui->horizontalSliderCylRadius->setValue(sliderPos);
        CreateAllElectrodes();
    }
}

//Created by RIZ2151215
//Since VTK6 we need to re-connect the data object to the vtk filters pipeline to update the whole pipeline
void SEEGAtlasWidget::UpdateActivePlan() {
    m_ActivePlanData.m_Cylinder->Update();
    m_ActivePlanData.m_CylObj->SetPolyData( m_ActivePlanData.m_Cylinder->GetOutput() );
    m_ActivePlanData.m_CylObj->MarkModified();
}


void SEEGAtlasWidget::UpdatePlan(int iElec) {
    for (int iCont=0; iCont<m_SavedPlansData[iElec].m_ContactsDisplay.size(); iCont++){
        m_SavedPlansData[iElec].m_ContactsDisplay[iCont].m_Cylinder->Update();
        m_SavedPlansData[iElec].m_ContactsDisplay[iCont].m_CylObj->SetPolyData( m_SavedPlansData[iElec].m_ContactsDisplay[iCont].m_Cylinder->GetOutput() );
        m_SavedPlansData[iElec].m_ContactsDisplay[iCont].m_CylObj->MarkModified();
    }
    m_SavedPlansData[iElec].m_ElectrodeDisplay.m_Cylinder->Update();
    m_SavedPlansData[iElec].m_ElectrodeDisplay.m_CylObj->SetPolyData( m_SavedPlansData[iElec].m_ElectrodeDisplay.m_Cylinder->GetOutput() );
    m_SavedPlansData[iElec].m_ElectrodeDisplay.m_CylObj->MarkModified();
}


void SEEGAtlasWidget::onChangeTrajectoryCylinderLength(int sliderPos) {
    int newlen = sliderPos;
    ui->lineEditCylinderLength->setText(QString::number(newlen));
    CreateAllElectrodes();
}

void SEEGAtlasWidget::onChangeTrajectoryCylinderLength(QString strPos) {
    int newlen = strPos.toFloat();
    if (newlen != ui->horizontalSliderCylinderLength->value()){
        ui->horizontalSliderCylinderLength->setValue(newlen);
    }
}

void SEEGAtlasWidget::onGetLocationValue(){

        // Get coordinates from cursor
        Application &app = Application::GetInstance();
        SceneManager *scene = app.GetSceneManager();
        double pos[3];
        scene->GetCursorPosition(pos);
        Point3D posPt;
        for (int i=0;i<3;i++) {
            posPt[i] = pos[i];
        }

        // Find value of anatomical Atlas at given coordinate
        int locationValue = getLocationValue(posPt);
        if (locationValue >= 0){
            // Indicate location in text box:
            map <int,string> labelsMap = ReadAtlasLabels();
            if (labelsMap.size() > 0) {
                string labelName = labelsMap[int(locationValue)];
                ui->lineEditLocationValue->setText(QString(labelName.c_str()) + " ("+ QString::number(locationValue)+")");
            } else {
                ui->lineEditLocationValue->setText(QString::number(locationValue));
            }
        }
}

int SEEGAtlasWidget::getLocationValue(seeg::Point3D point){
    // Finds value of Atlas volume at the given point and returns it
    float pixelValue;
    seeg::FloatVolume::Pointer anatLabelsVol = openAtlasVolume();
    if (anatLabelsVol.IsNotNull()) {
        // Convert position in coordinates to index
        FloatVolume::IndexType indexPos;
        anatLabelsVol->TransformPhysicalPointToIndex(point, indexPos);

        //Get Value at that location of the Atlas
        pixelValue = anatLabelsVol->GetPixel(indexPos);
    } else {
        pixelValue = -1;
    }
    return int(pixelValue);
}

FloatVolume::Pointer SEEGAtlasWidget::openAtlasVolume(){
    FloatVolume::Pointer anatLabelsVol;
    string whichSpace =ui->comboBoxWhatToOpen->currentText().toStdString();
    string strGroup;
    if (whichSpace.compare(0,8,"Template") ==0) {
        strGroup = "Template";
    } else if (whichSpace.compare(0,12,"Segmentation") ==0) {
        strGroup = "T1"; //if not template assumes that it is pre-space
    } else {
        QMessageBox::information(this, "No Atlas for this space", "No Atlas for this space - change space");
        return anatLabelsVol;
    }
    //anatLabelsVol = OpenFloatVolume(strGroup, "CSF_WM_GM");
	qDebug() <<"onFindAnatLocation "<< " - CSF_WM_GM - " << whichSpace.c_str() <<" - " <<strGroup.c_str();
    anatLabelsVol = OpenFloatVolume(strGroup, "AnatLabels");
	qDebug() << "openAtlasVolume "<< " - AnatLabels - " << whichSpace.c_str() <<" - " <<strGroup.c_str();

    return anatLabelsVol;
}


// ELECTRODE TYPE
void SEEGAtlasWidget::on_comboBoxElectrodeType_currentIndexChanged(int index){
    
    m_ElectrodeModel = m_ElectrodeModelList.at(index);
    this->UpdateUi();
    
    //SEEGElectrodesCohort::Pointer pathCohortObj = GetSEEGElectrodesCohort();
    m_SEEGElectrodesCohort->SetElectrodeModel(m_ElectrodeModel);

    int iElec = ui->comboBoxPlanSelect->currentIndex();
    onUpdateElectrode(iElec);

//    m_SEEGElectrodesCohort->GetBestCohort();
//    string electrodeName = m_AllPlans[iElec].name;
//    ElectrodeInfo::Pointer electrode = m_SEEGElectrodesCohort->GetTrajectoryInBestCohort(electrodeName);
//    if(!electrode) return;

    // Create visual electrodes again
    CreateAllElectrodes(true);
    
}


void SEEGAtlasWidget::on_checkBoxImagePlanes_stateChanged(int state)
{
    // Show/hide image planes
    Application::GetInstance().GetSceneManager()->ViewPlane(0, state);
    Application::GetInstance().GetSceneManager()->ViewPlane(1, state);
    Application::GetInstance().GetSceneManager()->ViewPlane(2, state);

    // Show/hide additional information
    // ...
}

void SEEGAtlasWidget::on_pushGenerateSurface_clicked()
{
    if( m_pluginInterface )
    {
        IbisAPI * ibisApi = m_pluginInterface->GetIbisAPI();
        Q_ASSERT(ibisApi);
        int currentObjectId = ui->comboBoxBrainSegmentation->currentData().toInt();
        SceneObject * currentObject = ibisApi->GetObjectByID(currentObjectId);
        if( currentObject )
        {
            ibisApi->SetCurrentObject(currentObject);
            ContourSurfacePluginInterface * surfaceReconstructor = static_cast<ContourSurfacePluginInterface *>(ibisApi->GetObjectPluginByName(tr("GeneratedSurface")));
            if( surfaceReconstructor )
            {
                PolyDataObject * surfaceObject;
                surfaceObject = PolyDataObject::SafeDownCast(surfaceReconstructor->CreateObject());
                if( surfaceObject )
                {
                    // set polydata properties
                    surfaceObject->SetOpacity(0.3);
                    double color[3] = {255 / 255.0 , 160 / 255.0 , 240 / 255.0}; // r, g, b
                    surfaceObject->SetColor(color);
                }
            }
        }
    }
}

/***********************************************************************
 * PRIVATE FUNCTIONS
 ***********************************************************************/

/*void SEEGAtlasWidget::SelectTrajectoryInList(Point3D targetPoint, Point3D entryPoint) {
    // After optimizing --> select the corresonding trajectory in the list
    // compute distance between trajectories
    float minDistance;
    int indTab = ui->tabWidgetScores->currentIndex();
    int iLoc = indTab-1; //RIZ: first TAB of table is BEST trajectories, subsequent are trajectory lists per location

    SEEGPathPlanner::Pointer planner = GetSEEGPathPlanners(iLoc);
    ElectrodeInfo::Pointer info = planner->FindActiveElectrodeInfo(targetPoint, entryPoint, minDistance);

    if (!info) {
        return;
    }

    Vector3D_f pe(info->m_EntryPointWorld[0], info->m_EntryPointWorld[1], info->m_EntryPointWorld[2]);
    Vector3D_f pt(info->m_TargetPointWorld[0], info->m_TargetPointWorld[1], info->m_TargetPointWorld[2]);

    if (minDistance < 100) { //check which number makes sense!
        // find entry point in list.
        QTableWidget* table = m_VectorContactsTables[iLoc];
        for (int i=0; i < table->rowCount(); i++) {
            Vector3D_f pe2;
            pe2.x = table->item(i, 0)->text().toFloat();
            pe2.y = table->item(i, 1)->text().toFloat();
            pe2.z = table->item(i, 2)->text().toFloat();
            Vector3D_f pt2;
            pt2.x = table->item(i, 3)->text().toFloat();
            pt2.y = table->item(i, 4)->text().toFloat();
            pt2.z = table->item(i, 5)->text().toFloat();

            if ((norm(pe2-pe) < 0.01) && (norm(pt2-pt) < 0.01)) {
                table->setCurrentCell(i, 0);
                onTrajectoryTableCellChange(i,0,-1,-1, iLoc); //RIZ: force update! ...it was not working without - check if better way...
            }
        }
    }
}
*/

/* void SEEGAtlasWidget::DisplayAllSavedPlan() { //display all
    int nLocations =GetNumberElectrodes();
    for (int iElec=0;iElec<nLocations;iElec++){
        DisplaySavedPlan(iElec);
    }
}
*/

/*void SEEGAtlasWidget::DisplaySavedPlan(int iElec) {

    double len = ui->lineEditCylinderLength->text().toFloat();

    Point3D entryPointLong, p1,p2;
    Resize3DLineSegmentSingleSide(m_AllPlans[iElec].targetPoint, m_AllPlans[iElec].entryPoint, len, entryPointLong);
    Point3D targetPoint = m_AllPlans[iElec].targetPoint;

    CreateElectrode(iElec);
    m_SavedPlansData[iElec].m_ElectrodeDisplay.m_Line->SetPoint1(targetPoint[0], targetPoint[1], targetPoint[2]);
    m_SavedPlansData[iElec].m_ElectrodeDisplay.m_Line->SetPoint2(entryPointLong[0], entryPointLong[1], entryPointLong[2]);
    for (int iCont=0; iCont<m_SavedPlansData[iElec].m_ContactsDisplay.size(); iCont++){
        m_SavedPlansData[iElec].m_ElectrodeModel->CalcContactStartEnds(iCont, targetPoint, entryPointLong, p1, p2);
        m_SavedPlansData[iElec].m_ContactsDisplay[iCont].m_Line->SetPoint1(p1[0], p1[1], p1[2]);
        m_SavedPlansData[iElec].m_ContactsDisplay[iCont].m_Line->SetPoint2(p2[0], p2[1], p2[2]);
        m_SavedPlansData[iElec].m_ContactsDisplay[iCont].m_CylObj->SetHidden(false);

       // m_SavedPlansData[planSel].m_ContactsDisplay[iCont].m_CylObj->MarkModified();
    }
    m_SavedPlansData[iElec].m_ElectrodeDisplay.m_CylObj->SetListable(true);
    m_SavedPlansData[iElec].m_ElectrodeDisplay.m_CylObj->SetHidden(false);
    UpdatePlan(iElec);//   m_SavedPlansData[planSel].m_ElectrodeDisplay.m_CylObj->MarkModified();
}

*/

/*void SEEGAtlasWidget::SetTrajectoryCursor(Point3D targetPoint, Point3D entryPoint) {

   // int sel = ui->comboBoxPlanSelect->currentIndex();
    Point3D p2_out;

    //m_AllPlans[sel].targetPoint = targetPoint; //RIZ
   // m_AllPlans[sel].entryPoint = entryPoint;   //RIZ
    float len = ui->lineEditCylinderLength->text().toFloat();
    Resize3DLineSegmentSingleSide(targetPoint, entryPoint, len, p2_out);
    m_ActivePlanData.m_Line->SetPoint1(targetPoint[0], targetPoint[1], targetPoint[2]);
    m_ActivePlanData.m_Line->SetPoint2(p2_out[0], p2_out[1], p2_out[2]);

 //   double p[3];//, p_conv[3]; //RIZ20151207 Gives a segmentation fault!
//   for (int i=0; i<3; i++) p[i] = p2_out[i];  //RIZ20151207 Gives a segmentation fault!
    // Application::GetInstance().GetSceneManager()->ReferenceToWorld(p, p_conv);   //RIZ20151207 Commented before
     //this->m_EntryPointHandleRepresentation->SetWorldPosition(p_conv);            //RIZ20151207 Commented before
  //this->m_EntryPointHandleRepresentation->SetWorldPosition(p); //RIZ20151207 Gives a segmentation fault!

    if (!m_TrajVisWidget->isHidden() && m_TrajVisWidget->HasDataToDisplay()) {
          m_TrajVisWidget->ShowTrajectory(    m_ActivePlanData.m_Line->GetPoint1(),
                                            m_ActivePlanData.m_Line->GetPoint2());
    }
    UpdateActivePlan();//Replaces:    m_ActivePlanData.m_CylObj->MarkModified();
 //   this->RenderIBISViews();
}
*/

/***
    Load Image Data
***/

void SEEGAtlasWidget::LoadAnatDataPosSpace() {
    Application &app = Application::GetInstance();

    OpenFileParams parentFileParams;
    DataSetInfo *dsInfo;

    if (!VolumeExists(VOL_GROUP_POS, VOL_T1_POS_ORIGINALSPACE) && !VolumeExists(VOL_GROUP_POS, VOL_CT_POS_ORIGINALSPACE)) {
        QMessageBox::warning(   this,
                                "Volume (CT or MRI) with electrodes not found",
                                "Warning: could not load pos implantation data because T1w/CT volume was not loaded");
        return;
    }

    // Load MRI with electrodes if exist
    if (VolumeExists(VOL_GROUP_POS, VOL_T1_POS_ORIGINALSPACE)) {
        dsInfo = seeg::GetDatasetInfo(VOL_GROUP_POS, VOL_T1_POS_ORIGINALSPACE);
        parentFileParams.AddInputFile(QString::fromStdString(dsInfo->filename), VOL_T1_POS_ORIGINALSPACE);
        parentFileParams.filesParams.last().isLabel = false;
        parentFileParams.filesParams.last().isReference = true;
        parentFileParams.filesParams.last().parent = this->m_TrajPlanMainObject;
    }

    // Load CT with electrodes if exist
    if (VolumeExists(VOL_GROUP_POS, VOL_CT_POS_ORIGINALSPACE)) {
        dsInfo = seeg::GetDatasetInfo(VOL_GROUP_POS, VOL_CT_POS_ORIGINALSPACE);
        parentFileParams.AddInputFile(QString::fromStdString(dsInfo->filename), VOL_CT_POS_ORIGINALSPACE);
        parentFileParams.filesParams.last().isLabel = false;
        parentFileParams.filesParams.last().isReference = true;
        parentFileParams.filesParams.last().parent = this->m_TrajPlanMainObject;
    }

    // Load MRI pre implantation if exist
    if (VolumeExists(VOL_GROUP_POS, VOL_T1PRE_REGTO_MRPOS)) {
        dsInfo = seeg::GetDatasetInfo(VOL_GROUP_POS, VOL_T1PRE_REGTO_MRPOS);
        parentFileParams.AddInputFile(QString::fromStdString(dsInfo->filename), VOL_T1PRE_REGTO_MRPOS);
        parentFileParams.filesParams.last().isLabel = false;
        parentFileParams.filesParams.last().isReference = false;
        parentFileParams.filesParams.last().parent = this->m_TrajPlanMainObject;
    }
    // Load MRI pre implantation if exist
    if (VolumeExists(VOL_GROUP_POS, VOL_T1PRE_REGTO_CTPOS)) {
        dsInfo = seeg::GetDatasetInfo(VOL_GROUP_POS, VOL_T1PRE_REGTO_CTPOS);
        parentFileParams.AddInputFile(QString::fromStdString(dsInfo->filename), VOL_T1PRE_REGTO_CTPOS);
        parentFileParams.filesParams.last().isLabel = false;
        parentFileParams.filesParams.last().isReference = false;
        parentFileParams.filesParams.last().parent = this->m_TrajPlanMainObject;
    }

    app.OpenFiles(&parentFileParams);

    for (int i=0; i<parentFileParams.filesParams.size(); i++) {
        SceneObject *o = parentFileParams.filesParams.at(i).loadedObject;
        o->SetCanChangeParent(false);
        o->SetNameChangeable(false);
        //     o->SetHidden(true);
    }

    //app.GetSceneManager()->AddObject(m_ActivePlanData.m_CylObj, m_TrajPlanMainObject);

}

void SEEGAtlasWidget::LoadAnatTemplateSpace() {
    Application &app = Application::GetInstance();

    OpenFileParams parentFileParams;
    DataSetInfo *dsInfo;

    if (!VolumeExists(VOL_GROUP_TAL, VOL_TAL_T1)) {
        QMessageBox::warning(   this,
                                "Volume in TAL space not found",
                                "Warning: could not load pos implantation data because T1w volume was not loaded");
        return;
    }

    // Load MRI pre implantation i TAL space if exist
    if (VolumeExists(VOL_GROUP_TAL, VOL_TAL_T1)) {
        dsInfo = seeg::GetDatasetInfo(VOL_GROUP_TAL, VOL_TAL_T1);
        parentFileParams.AddInputFile(QString::fromStdString(dsInfo->filename), VOL_TAL_T1);
        parentFileParams.filesParams.last().isLabel = false;
        parentFileParams.filesParams.last().isReference = true;
        parentFileParams.filesParams.last().parent = this->m_TrajPlanMainObject;
    }

    // Load Segmented CSF/WM/GM for this patient in TAL space if exist
    if (VolumeExists(VOL_GROUP_TAL, VOL_TAL_CSF_WM_GM)) {
        dsInfo = seeg::GetDatasetInfo(VOL_GROUP_TAL, VOL_TAL_CSF_WM_GM);
        parentFileParams.AddInputFile(QString::fromStdString(dsInfo->filename), VOL_TAL_CSF_WM_GM);
        parentFileParams.filesParams.last().isLabel = true;
        parentFileParams.filesParams.last().isReference = false;
        parentFileParams.filesParams.last().parent = this->m_TrajPlanMainObject;
    }

    // Load Segmented map for this patient in TAL space if exist
    if (VolumeExists(VOL_GROUP_TAL, VOL_TAL_ANATOMICAL_REGIONS)) {
        dsInfo = seeg::GetDatasetInfo(VOL_GROUP_TAL, VOL_TAL_ANATOMICAL_REGIONS);
        parentFileParams.AddInputFile(QString::fromStdString(dsInfo->filename), VOL_TAL_ANATOMICAL_REGIONS);
        parentFileParams.filesParams.last().isLabel = true;
        parentFileParams.filesParams.last().isReference = false;
        parentFileParams.filesParams.last().parent = this->m_TrajPlanMainObject;
    }

    app.OpenFiles(&parentFileParams);

    for (int i=0; i<parentFileParams.filesParams.size(); i++) {
        SceneObject *o = parentFileParams.filesParams.at(i).loadedObject;
        o->SetCanChangeParent(false);
        o->SetNameChangeable(false);
        //     o->SetHidden(true);
    }

   // app.GetSceneManager()->AddObject(m_ActivePlanData.m_CylObj, m_TrajPlanMainObject);
}

void SEEGAtlasWidget::LoadAnatData() {
    Application &app = Application::GetInstance();

   // list<string> groupList;
   // list<string> datasetList;
  //  list<string>::iterator it1, it2;
    OpenFileParams parents, childs;
    DataSetInfo *dsInfo;

   bool lightLoad = ui->checkBoxLightDatasetLoading->isChecked();

    int indT1 = 0;
    if (!VolumeExists(VOL_GROUP_T1, VOL_T1_PRE)) {
#if 0
        QMessageBox::warning(   this,
                                "T1w volume pre implantation not found",
                                "Warning: could not load Anat Preset because T1w volume was not loaded");
#endif
        return;
    }

    dsInfo = seeg::GetDatasetInfo(VOL_GROUP_T1, VOL_T1_PRE);
    parents.AddInputFile(QString::fromStdString(dsInfo->filename), VOL_GROUP_T1);
    parents.filesParams.last().isReference = true;
    parents.filesParams.last().parent = this->m_TrajPlanMainObject;
    app.OpenFiles(&parents);

    SceneObject *t1vol = parents.filesParams.last().loadedObject;

    if (t1vol) {
        t1vol->SetCanChangeParent(false);
        t1vol->SetNameChangeable(false);

        // Load an cortex surface object in a similar way as a volume
        if (VolumeExists(VOL_GROUP_T1, OBJ_T1_BRAIN_SURFACE_OBJ)) {
            dsInfo = seeg::GetDatasetInfo(VOL_GROUP_T1, OBJ_T1_BRAIN_SURFACE_OBJ);
            childs.AddInputFile(QString::fromStdString(dsInfo->filename), OBJ_T1_BRAIN_SURFACE_OBJ); //object obtained from FACE
            childs.filesParams.last().isLabel = false;
            childs.filesParams.last().parent = t1vol;
        }

        // Load Atlas
        if (VolumeExists(VOL_GROUP_T1, VOL_PRE_ANATOMICAL_REGIONS)) {
            dsInfo = seeg::GetDatasetInfo(VOL_GROUP_T1, VOL_PRE_ANATOMICAL_REGIONS);
            childs.AddInputFile(QString::fromStdString(dsInfo->filename), VOL_PRE_ANATOMICAL_REGIONS); // atlas obtained from MICCAI segmentation challenge
            childs.filesParams.last().isLabel = true;
            childs.filesParams.last().parent = t1vol;
        }

        // Load CSF_GM_WM
        if (VolumeExists(VOL_GROUP_T1, VOL_PRE_CSF_WM_GM)) {
            dsInfo = seeg::GetDatasetInfo(VOL_GROUP_T1, VOL_PRE_CSF_WM_GM);
            childs.AddInputFile(QString::fromStdString(dsInfo->filename), VOL_PRE_CSF_WM_GM); //CSF/WM/GM from longitudinal pipeline
            childs.filesParams.last().isLabel = true;
            childs.filesParams.last().parent = t1vol;
        }

        // Load All segmented volumes
        if (VolumeExists(VOL_GROUP_T1, VOL_T1_BRAIN) && !lightLoad) {
            dsInfo = seeg::GetDatasetInfo(VOL_GROUP_T1, VOL_T1_BRAIN);
            childs.AddInputFile(QString::fromStdString(dsInfo->filename), VOL_T1_BRAIN);
            childs.filesParams.last().parent = t1vol;
        }

        if (VolumeExists(VOL_GROUP_T1, VOL_T1_VENTRICLES) && !lightLoad) {
            dsInfo = seeg::GetDatasetInfo(VOL_GROUP_T1, VOL_T1_VENTRICLES);
            childs.AddInputFile(QString::fromStdString(dsInfo->filename), VOL_T1_VENTRICLES);
            childs.filesParams.last().isLabel = true;
            childs.filesParams.last().parent = t1vol;
        }

        if (VolumeExists(VOL_GROUP_T1, VOL_T1_SULCI) && !lightLoad) {
            dsInfo = seeg::GetDatasetInfo(VOL_GROUP_T1, VOL_T1_SULCI);
            childs.AddInputFile(QString::fromStdString(dsInfo->filename), VOL_T1_SULCI);
            childs.filesParams.last().isLabel = true;
            childs.filesParams.last().parent = t1vol;
        }

        if (VolumeExists(VOL_GROUP_T1, VOL_T1_CAUDATE) && !lightLoad) {
            dsInfo = seeg::GetDatasetInfo(VOL_GROUP_T1, VOL_T1_CAUDATE);
            childs.AddInputFile(QString::fromStdString(dsInfo->filename), VOL_T1_CAUDATE);
            childs.filesParams.last().isLabel = true;
            childs.filesParams.last().parent = t1vol;
        }

        if (VolumeExists(VOL_GROUP_T1, VOL_T1_MUSCLE) && !lightLoad) {
            dsInfo = seeg::GetDatasetInfo(VOL_GROUP_T1, VOL_T1_MUSCLE);
            childs.AddInputFile(QString::fromStdString(dsInfo->filename), VOL_T1_MUSCLE);
            childs.filesParams.last().isLabel = true;
            childs.filesParams.last().parent = t1vol;
        }

        if (VolumeExists(VOL_GROUP_T1, VOL_T1_CSF) && !lightLoad) {
            dsInfo = seeg::GetDatasetInfo(VOL_GROUP_T1, VOL_T1_CSF);
            childs.AddInputFile(QString::fromStdString(dsInfo->filename), VOL_T1_CSF);
            childs.filesParams.last().isLabel = true;
            childs.filesParams.last().parent = t1vol;
        }


        //load also skull data
        if (VolumeExists(VOL_GROUP_T1, VOL_SKULL)) {
            dsInfo = seeg::GetDatasetInfo(VOL_GROUP_T1, VOL_SKULL);
            childs.AddInputFile(QString::fromStdString(dsInfo->filename), VOL_SKULL);
            childs.filesParams.last().isLabel = true;
            childs.filesParams.last().parent = t1vol;
        }
        if (VolumeExists(VOL_GROUP_T1, VOL_SKULL_MASK) && !lightLoad) {
            dsInfo = seeg::GetDatasetInfo(VOL_GROUP_T1, VOL_SKULL_MASK);
            childs.AddInputFile(QString::fromStdString(dsInfo->filename), VOL_SKULL_MASK);
            childs.filesParams.last().isLabel = true;
            childs.filesParams.last().parent = t1vol;
        }

    }
    app.OpenFiles(&childs);

    for (int i=0; i<childs.filesParams.size(); i++) {
        SceneObject *o = childs.filesParams.at(i).loadedObject;
        o->SetCanChangeParent(false);
        o->SetNameChangeable(false);
        o->SetHidden(true);
        if (indT1==i){
            m_T1objectID = o->GetObjectID(); /// RIZ: do it properly later on!!!
        }
    }

   // app.GetSceneManager()->AddObject(m_ActivePlanData.m_CylObj, m_TrajPlanMainObject);

}

void SEEGAtlasWidget::LoadGadoData() {
    Application &app = Application::GetInstance();
    OpenFileParams parents, childs;
    DataSetInfo *dsInfo;
    GroupInfo *groupInfo;

    bool lightLoad = ui->checkBoxLightDatasetLoading->isChecked();

    if (!VolumeExists(VOL_GROUP_GADO, VOL_GADO_RAW)) {
#if 0
        QMessageBox::warning(   this,
                                "T1w-Gd (preop) volume not found",
                                "Warning: could not load PreOp Preset because T1w-Gd (preop) volume was not found");
#endif
        return;
    }

    dsInfo = seeg::GetDatasetInfo(VOL_GROUP_GADO, VOL_GADO_RAW);
    parents.AddInputFile(QString::fromStdString(dsInfo->filename), VOL_GADO_RAW);
    parents.filesParams.last().parent = this->m_TrajPlanMainObject;
    app.OpenFiles(&parents);

    SceneObject *gadovol = parents.filesParams.last().loadedObject;
    if (gadovol) {
        gadovol->SetCanChangeParent(false);
        gadovol->SetNameChangeable(false);
        gadovol->SetHidden(true);

        groupInfo = seeg::GetGroupInfo(VOL_GROUP_GADO);
        app.OpenTransformFile(groupInfo->transformFile.c_str(), gadovol);

        if (VolumeExists(VOL_GROUP_GADO, VOL_GADO_VESSELNESS) && !lightLoad) {
            dsInfo = seeg::GetDatasetInfo(VOL_GROUP_GADO, VOL_GADO_VESSELNESS);
            childs.AddInputFile(QString::fromStdString(dsInfo->filename), VOL_GADO_VESSELNESS);
            childs.filesParams.last().parent = gadovol;
        }
    }
    app.OpenFiles(&childs);

    for (int i=0; i<childs.filesParams.size(); i++) {
        SceneObject *o = childs.filesParams.at(i).loadedObject;
        o->SetCanChangeParent(false);
        o->SetNameChangeable(false);
        o->SetHidden(true);
    }

}

void SEEGAtlasWidget::LoadCTAData() {
    Application &app = Application::GetInstance();
    OpenFileParams parents, childs;
    DataSetInfo *dsInfo;
    GroupInfo *groupInfo;

    bool lightLoad = ui->checkBoxLightDatasetLoading->isChecked();

    if (!VolumeExists(VOL_GROUP_CTA, VOL_CTA_RAW_NATIVE)) {
#if 0
        QMessageBox::warning(   this,
                                "CTA (preop) volume not found",
                                "Warning: could not load PreOp Preset because CTA (preop) volume was not found");
#endif
        return;
    }

    dsInfo = seeg::GetDatasetInfo(VOL_GROUP_CTA, VOL_CTA_RAW_NATIVE);
    parents.AddInputFile(QString::fromStdString(dsInfo->filename), VOL_CTA_RAW_NATIVE);
    parents.filesParams.last().parent = this->m_TrajPlanMainObject;
    app.OpenFiles(&parents);

    SceneObject *ctavol = parents.filesParams.last().loadedObject;
    if (ctavol) {
        ctavol->SetCanChangeParent(false);
        ctavol->SetNameChangeable(false);
        ctavol->SetHidden(true);

        groupInfo = seeg::GetGroupInfo(VOL_GROUP_CTA);
        app.OpenTransformFile(groupInfo->transformFile.c_str(), ctavol);

        if (VolumeExists(VOL_GROUP_CTA, VOL_CTA_VESSELNESS) && !lightLoad) {
            dsInfo = seeg::GetDatasetInfo(VOL_GROUP_CTA, VOL_CTA_VESSELNESS);
            childs.AddInputFile(QString::fromStdString(dsInfo->filename), VOL_CTA_VESSELNESS);
            childs.filesParams.last().parent = ctavol;
        }

        if (VolumeExists(VOL_GROUP_CTA, VOL_CTA_VESSELNESS_BINARY) && !lightLoad) {
            dsInfo = seeg::GetDatasetInfo(VOL_GROUP_CTA, VOL_CTA_VESSELNESS_BINARY);
            childs.AddInputFile(QString::fromStdString(dsInfo->filename), VOL_CTA_VESSELNESS_BINARY);
            childs.filesParams.last().parent = ctavol;
        }

        if (VolumeExists(VOL_GROUP_CTA, VOL_CTA_VESSELNESS_3D)) {
            dsInfo = seeg::GetDatasetInfo(VOL_GROUP_CTA, VOL_CTA_VESSELNESS_3D);
            childs.AddInputFile(QString::fromStdString(dsInfo->filename), VOL_CTA_VESSELNESS_3D);
            childs.filesParams.last().parent = ctavol;
        }
    }
    app.OpenFiles(&childs);

    for (int i=0; i<childs.filesParams.size(); i++) {
        SceneObject *o = childs.filesParams.at(i).loadedObject;
        o->SetCanChangeParent(false);
        o->SetNameChangeable(false);
        o->SetHidden(true);
    }
}

void SEEGAtlasWidget::LoadCTData() {
    Application &app = Application::GetInstance();
    OpenFileParams parents, childs;
    DataSetInfo *dsInfo;
    GroupInfo *groupInfo;

    bool lightLoad = ui->checkBoxLightDatasetLoading->isChecked();

    if (!VolumeExists(VOL_GROUP_CT, VOL_CT_RAW_NATIVE)) {
#if 0
        QMessageBox::warning(   this,
                                "CT (preop) volume not found",
                                "Warning: could not load PreOp Preset because CT (preop) volume was not found");
#endif
        return;
    }

    dsInfo = seeg::GetDatasetInfo(VOL_GROUP_CT, VOL_CT_RAW_NATIVE);
    parents.AddInputFile(QString::fromStdString(dsInfo->filename), VOL_CT_RAW_NATIVE);
    parents.filesParams.last().parent = this->m_TrajPlanMainObject;
    app.OpenFiles(&parents);

    SceneObject *ctvol = parents.filesParams.last().loadedObject;
    if (ctvol) {
        ctvol->SetCanChangeParent(false);
        ctvol->SetNameChangeable(false);
        ctvol->SetHidden(true);

        groupInfo = seeg::GetGroupInfo(VOL_GROUP_CT);
        app.OpenTransformFile(groupInfo->transformFile.c_str(), ctvol);
    }
}


/***
    Visualization
***/
void SEEGAtlasWidget::RefreshVisualization() {
//    Application &app = Application::GetInstance();
//    SceneManager *scene = Application::GetInstance().GetSceneManager();

    QString whatToOpen = ui->comboBoxWhatToOpen->currentText();
    if (whatToOpen.contains("Electrode") != 0 ){ //To mark electrodes
        LoadAnatDataPosSpace();
    } else if(whatToOpen.contains("Template") != 0 ){ //To localize contacts in Template space
        LoadAnatTemplateSpace();
    } else {                    //To look at segmented data in patient's space
        LoadAnatData();
        LoadGadoData();
        LoadCTAData();
        LoadCTData();
    }
}

void SEEGAtlasWidget::RefreshContactsTable(const int iElec) {
    //Refreshes list of contacts in Table
    //vector<string> electrodeNames = GetElectrodeNames();

    vector<ContactInfo::Pointer> contactList;
    QTableWidgetItem *item;
    string electrodeName = m_AllPlans[iElec].name;

    // Get contacts of electrode
   // SEEGElectrodesCohort::Pointer cohort = GetSEEGElectrodesCohort();
    ElectrodeInfo::Pointer electrode = m_SEEGElectrodesCohort->GetTrajectoryInBestCohort(electrodeName);
    contactList = electrode->GetAllContacts();

    //populate table
    int indTab = iElec +1;
    QTableWidget* table = m_VectorContactsTables[iElec];
    table->setRowCount(0); //First empty table

    ui->tabWidgetScores->setTabText(indTab, QString(electrodeName.c_str()));
    ui->tabWidgetScores->setTabEnabled(indTab, false);
    table->setRowCount(contactList.size());
    table->setToolTip(QString(electrodeName.c_str()));
    for (int row=0; row<contactList.size(); row++) {
        ContactInfo::Pointer contact = contactList[row];
        //Name
        item = new QTableWidgetItem();
        item->setText(QString(contact->GetContactName().c_str()));
        table->setItem(row, 0, item);
        //Coordinates
        for (int iDim=0;iDim<3;iDim++) { //add ep & tp
            item = new QTableWidgetItem(); //RIZ: is this necessary??
            item->setText(QString::number(contact->m_CentralPointWorld[iDim]));
            table->setItem(row, iDim+1, item);
        }
        //Anatomical Location
        if (contact->m_IsLocationSet) {
            item = new QTableWidgetItem();
            item->setText(QString(contact->GetContactLocation().c_str()));
            table->setItem(row, 4, item);
            item = new QTableWidgetItem();
            item->setText(QString::number(contact->GetProbabilityOfLocation(),'g',3));
            table->setItem(row, 5, item);
        }
    }
    ui->tabWidgetScores->setCurrentIndex(indTab);
    ui->tabWidgetScores->setTabEnabled(indTab, true);
}

void SEEGAtlasWidget::RefreshChannelsTable(const int iElec) {
    //Refreshes list of contacts in Table
    //vector<string> electrodeNames = GetElectrodeNames();

    vector<ChannelInfo::Pointer> channelsList;
    QTableWidgetItem *item;
    string electrodeName = m_AllPlans[iElec].name;

    // Get contacts of electrode
    //SEEGElectrodesCohort::Pointer cohort = GetSEEGElectrodesCohort();
    ElectrodeInfo::Pointer electrode = m_SEEGElectrodesCohort->GetTrajectoryInBestCohort(electrodeName);
    channelsList = electrode->GetAllChannels();

    //populate table
    int indTab = iElec +1;
    QTableWidget* table = m_VectorContactsTables[iElec];
    table->setRowCount(0);  //delete row to avoid previous values
    ui->tabWidgetScores->setTabText(indTab, QString(electrodeName.c_str()));
    ui->tabWidgetScores->setTabEnabled(indTab, false);
    table->setRowCount(channelsList.size());
    table->setToolTip(QString(electrodeName.c_str()));
    for (int row=0; row<channelsList.size(); row++) {
        ChannelInfo::Pointer channel = channelsList[row];
        //Name
        item = new QTableWidgetItem();
        item->setText(QString(channel->GetChannelName().c_str()));
        table->setItem(row, 0, item);
        //Coordinates
       // Point3D Contact1Pt = channel->GetContact1()->GetCentralPoint(); // before we were assigning the coord of the first contact
        Point3D ChannelCentralPt = m_ElectrodeModel->CalcChannelCenterPosition(channel->GetChannelIndex(),electrode->GetTargetPoint(),electrode->GetEntryPoint(), 0);
        for (int iDim=0;iDim<3;iDim++) { //add ep & tp
            item = new QTableWidgetItem(); //RIZ: is this necessary??
            //item->setText(QString::number(Contact1Pt[iDim]));     // before we were assigning the coord of the first contact
            item->setText(QString::number(ChannelCentralPt[iDim])); // now is the central point of the channel
            table->setItem(row, iDim+1, item);
        }
        //Anatomical Location
        if (channel->IsLocationSet()) {
            item = new QTableWidgetItem();
            item->setText(QString(channel->GetChannelLocation().c_str()));
            table->setItem(row, 4, item);
            item = new QTableWidgetItem();
            item->setText(QString::number(channel->GetProbabilityOfLocation(),'g',3));
            table->setItem(row, 5, item);
        }
    }
    ui->tabWidgetScores->setCurrentIndex(indTab);
    ui->tabWidgetScores->setTabEnabled(indTab, true);
}



void SEEGAtlasWidget::RefreshAllPlanCoords() {
    //int sel = ui->comboBoxPlanSelect->currentIndex();
    int nLocations = GetNumberElectrodes();
    //vector<string> electrodeNames = GetElectrodeNames();
    for (int iElec=0;iElec<nLocations;iElec++){
        //RefreshPlanCoords(iLoc, electrodeNames[iLoc]);
        RefreshPlanCoords(iElec);
        m_SavedPlansData[iElec].m_PointRepresentation->SelectPoint(-1);
    }
}

void SEEGAtlasWidget::RefreshPlanCoords(int iElec) {

    if (iElec<0) { return;}

    if (iElec >= ui->comboBoxPlanSelect->count()) {
        ui->comboBoxPlanSelect->addItem(QString(m_AllPlans[iElec].name.c_str()));
    }

    ui->comboBoxPlanSelect->setCurrentIndex(iElec);
    if (m_AllPlans[iElec].isTargetSet) {
        ui->lineEditTargetX->setText(QString::number(m_AllPlans[iElec].targetPoint[0]));
        ui->lineEditTargetY->setText(QString::number(m_AllPlans[iElec].targetPoint[1]));
        ui->lineEditTargetZ->setText(QString::number(m_AllPlans[iElec].targetPoint[2]));
    } else {
        ui->lineEditTargetX->setText(QString("X"));
        ui->lineEditTargetY->setText(QString("Y"));
        ui->lineEditTargetZ->setText(QString("Z"));
    }

    if (m_AllPlans[iElec].isEntrySet) {
        ui->lineEditEntryX->setText(QString::number(m_AllPlans[iElec].entryPoint[0]));
        ui->lineEditEntryY->setText(QString::number(m_AllPlans[iElec].entryPoint[1]));
        ui->lineEditEntryZ->setText(QString::number(m_AllPlans[iElec].entryPoint[2]));
    } else {
        ui->lineEditEntryX->setText(QString("X"));
        ui->lineEditEntryY->setText(QString("Y"));
        ui->lineEditEntryZ->setText(QString("Z"));
    }

    if (!m_AllPlans[iElec].name.compare("")) {
        ui->lineEditElectrodeName->setText (QString(m_AllPlans[iElec].name.c_str()));
        ui->comboBoxPlanSelect->setCurrentText(QString(m_AllPlans[iElec].name.c_str()));
    }
}

void SEEGAtlasWidget::RefreshPlanCoords(int iElec, string electrodeName) {
    //assign name to Plan in List -> then update
    ui->lineEditElectrodeName->setText(QString(electrodeName.c_str()));

    RefreshPlanCoords(iElec);

    //update current TAB in Table with trajectories
    if (electrodeName != string("")) {
        int selTab=1000;
        int indexSelTab=1000;
        for (int iTab=iElec; iTab<ui->tabWidgetScores->count(); iTab++) {
            QString tabName = ui->tabWidgetScores->tabText(iTab);
            int index = tabName.indexOf(QString(electrodeName.c_str()),0, Qt::CaseInsensitive);
            if (index>=0 && index<indexSelTab){
                selTab = iTab;
                indexSelTab = index;
            }
        }
        if (selTab<1000) {ui->tabWidgetScores->setCurrentIndex(selTab);}
        ui->comboBoxPlanSelect->setItemText(iElec, QString(electrodeName.c_str()));
    }
    //ui->comboBoxPlanSelect->setItemText(sel, QString("Plan ") + QString::number(sel+1) + QString(" ")+ QString(electrodeName.c_str()));
}

// GETTERS & SETTERS
void SEEGAtlasWidget::AddElectrodeName(const string electrodeName){
    m_ElectrodesNames.push_back(electrodeName);
}

void SEEGAtlasWidget::ResetElectrodes(){
    // Delete ALL electrodes
    Application::GetInstance().GetSceneManager()->RemoveAllChildrenObjects(this->m_SavedPlansObject);
    for (int iElec=0; iElec<MAX_SEEG_PLANS; iElec++) {
        m_AllPlans[iElec] = TrajectoryDef();
      //  DeleteElectrode(iElec);
    }
    for (int iElec=0; iElec<ui->comboBoxPlanSelect->count(); iElec++) {
        ui->comboBoxPlanSelect->setItemText(iElec, QString("Elect ") + QString::number(iElec+1));
    }
    //m_ActivePlanData.m_CylObj->Delete();
    for (int iTab=1; iTab<=m_VectorContactsTables.size(); iTab++) {
        removeContactsTableTab(iTab);
    }
    RefreshAllPlanCoords();
    //Clean
    m_SEEGElectrodesCohort = SEEGElectrodesCohort::New(m_ElectrodeModel, m_SpacingResolution); // create again to remove any previous electrode
    m_ElectrodesNames.clear();
    m_VectorContactsTables.clear();
}

vector<string> SEEGAtlasWidget::GetElectrodeNames(){
    return m_ElectrodesNames;
}

void SEEGAtlasWidget::SetElectrodeNames(std::vector<std::string> electrodeNames){
    vector<string>::const_iterator itLoc;
    for (itLoc = electrodeNames.begin(); itLoc != electrodeNames.end(); itLoc++) { //for each location add electrode name
        string electrodeName = *itLoc;
        AddElectrodeName(electrodeName);
    }
}

void SEEGAtlasWidget::ReplaceElectrodeName(int indexElectrode, const string electrodeName){
    if (m_ElectrodesNames.size() > indexElectrode) {
        m_ElectrodesNames[indexElectrode] =electrodeName;
    } else {
        AddElectrodeName(electrodeName);
    }
}

int SEEGAtlasWidget::GetNumberElectrodes(){
    return m_ElectrodesNames.size();
}


SEEGElectrodesCohort::Pointer SEEGAtlasWidget::GetSEEGElectrodesCohort() {
    return m_SEEGElectrodesCohort;
}

// XML PArser
map <int,string> SEEGAtlasWidget::ReadAtlasLabels() {
    // Reads atlas from xml file into map
    map <int,string> atlasLabels;
    QString filenameAtlasLabelsXml(FILE_ATLAS_LABELS);
    QString baseDir = ui->labelDirBase->text();

    QString fullFileNameAtlasLabelsXml = baseDir + QString("/") + filenameAtlasLabelsXml;
    QFile xmlFileAtlasLabels(fullFileNameAtlasLabelsXml);
    if (!xmlFileAtlasLabels.open (QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this, "Error reading Atlas Labels file ", "Cannot read file " + fullFileNameAtlasLabelsXml);
        return atlasLabels;
    }

    QXmlStreamReader xmlReaderAtlasLabels(&xmlFileAtlasLabels);
    while(!xmlReaderAtlasLabels.atEnd() && !xmlReaderAtlasLabels.hasError()) {
        // QXmlStreamReader::TokenType tokenAtlasLabel = xmlReaderAtlasLabels.readNext();
        while (xmlReaderAtlasLabels.readNextStartElement()) {
            if (xmlReaderAtlasLabels.name() == "Label") { // now we are inside label
                QString qsLabelName, qsLabelNumber;
                while (xmlReaderAtlasLabels.readNextStartElement()) {
                    if (xmlReaderAtlasLabels.name() == "Name") {
                        qsLabelName = xmlReaderAtlasLabels.readElementText();
                    }
                    else if (xmlReaderAtlasLabels.name() == "Number"){
                        qsLabelNumber = xmlReaderAtlasLabels.readElementText();

                    }
                }
                // once we have the name and number we can input in the map var
                int labelNumber = qsLabelNumber.toInt();
                string labelName = qsLabelName.toStdString();
                atlasLabels[labelNumber] = labelName;
				qDebug() << "Name: "<< labelName.c_str() << "number: " << labelNumber;
            }
        }
    }
	qDebug() << "Finished reading "<< fullFileNameAtlasLabelsXml.toStdString().c_str();
    return atlasLabels;
}

// Batch Analysis
void SEEGAtlasWidget::onRunBatchAnalysis(){
    //1.1 Load anatomical Data
    QString baseDirectory = QFileDialog::getExistingDirectory(  this, tr("Open Base Directory - where subdirs are the patients' data"), "/",
                                                                QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks | QFileDialog::DontUseNativeDialog);

    if (baseDirectory == QString("")) {
        return;
    }
    //1.2. Read Patients names from file = directories from file
    string filenamePatients=(FILE_PATIENT_NAMES);
    string fullFileNamePatients = baseDirectory.toStdString() + string("/") + filenamePatients;
    ifstream filePatients;
    string linePatientInfo;
    filePatients.open(fullFileNamePatients.c_str());
    if (filePatients.is_open()){
        getline(filePatients, linePatientInfo);
        while(!filePatients.eof() && !linePatientInfo.empty()) {
            ResetElectrodes();
            Application::GetInstance().GetSceneManager()->RemoveAllChildrenObjects(this->m_TrajPlanMainObject);

    // 2. For each patient:
            QString patientName(linePatientInfo.c_str()); // each line contains the name of 1 patient subdirectory
            //construct patient's directory as baseDir/patientName
            QString patientDir = baseDirectory;
            patientDir.append(QString("/") + patientName);
			qDebug() << "Processing "<< patientName.toStdString().c_str();
    //2.1. Load dataset
            onLoadSEEGDatasetsFromDir(patientDir);
			qDebug() << "... Anatomical files loaded - "<< patientDir;

    //2.2. Load electrodes
            QString electrodesDirStr = patientDir;
            //electrodesDirStr.append(QString("/") + "electrodes/prespace");
            electrodesDirStr.append(QString("/") + "electrodes/TALspace");
            QDir electrodesDir(electrodesDirStr.toStdString().c_str() );
			qDebug() << "Electrodes dir: "<< electrodesDirStr;
            QStringList fileExtension;
            fileExtension << "*.csv";
            electrodesDir.setNameFilters(fileExtension);
            QFileInfoList filesList = electrodesDir.entryInfoList(QDir::Files, QDir::Type);
                  for (int iElec=0; iElec<filesList.size(); iElec++) {
                      QString fileFullPath = filesList[iElec].absoluteFilePath();
					  qDebug() << "... Loading Electrode "<< iElec << " - file "<< fileFullPath;
                      onLoadOnePlanFromCSVFile(iElec,  fileFullPath.toStdString());
                  }

            //onLoadPlanningFromDirectory(electrodesDir);

    //2.3. Find anatomical location of channels
            onFindChannelsAnatLocation();
			qDebug() << "... Anatomical Location per channel Found ";

    //2.4. Find anatomical location of contacts
            onFindAnatLocation();
			qDebug() << "... Anatomical Location per contact Found ";

    //2.5 Save All Electrodes' information in folder
            QString anatomicalInfoDir = electrodesDirStr;
          //  anatomicalInfoDir.append(QString("/") + "3x3x3/CSF_GM_WM");
           // anatomicalInfoDir.append(QString("/") + "3x3x3/AnatLoc_seg_lobes");
             anatomicalInfoDir.append(QString("/") + "3x3x3/AnatLoc_tal_seg_lobes_HCAG");
            onSavePlanningToDirectory(anatomicalInfoDir);
			qDebug() << "... Saving Electrodes with Anatomical Info" << " - Dir: "<< anatomicalInfoDir;

			qDebug() << "Patient "<< patientName << "Done!";
            // Read next patient
            getline(filePatients, linePatientInfo);
        }
    } else {
		qDebug() << "File "<< fullFileNamePatients.c_str() << "not found.";
    }
}

void SEEGAtlasWidget::on_pushButtonUpdateContactPosition_clicked()
{
    Q_ASSERT(m_pluginInterface);
    IbisAPI *api = m_pluginInterface->GetIbisAPI();

//    QString tabName = ui->tabWidgetScores->tabText(ui->tabWidgetScores->currentIndex());

    // get electrode index that matches the current tab name
//    int iElec = -1;
//    for (int i = 0; i < ui->comboBoxPlanSelect->count(); ++i)
//    {
//        if (tabName.compare(ui->comboBoxPlanSelect->itemText(i)) == 0)
//        {
//            iElec = i;
//        }
//    }
//    if (iElec == -1) return;

    int indTab = ui->tabWidgetScores->currentIndex();
    int iElec = indTab-1;

    if( iElec > m_VectorContactsTables.size() || iElec < 0 ) {return;}
    if( m_SavedPlansData[iElec].m_PointRepresentation->GetSelectedPoint() == -1 ) return;
    QTableWidget* table = m_VectorContactsTables[iElec];

    int iContact = table->currentRow();
    if(iContact < 0) return;

    //First column is electrode name
    string contactName = table->item(iContact, 0)->text().toStdString();

    // 2-4 columns are xyz
    double contactPosition[3];
    api->GetCursorPosition(contactPosition);

    // update table contact position
    for (int i=0; i<3; i++){ //column 0 is name / 1-3 columns are target / 4-6 columns are entry
        QTableWidgetItem * item = new QTableWidgetItem();
        item->setText( QString::number(contactPosition[i]) );
        table->setItem(iContact, i+1, item);
    }

    string electrodeName = m_AllPlans[iElec].name;
    ElectrodeInfo::Pointer electrode = m_SEEGElectrodesCohort->GetTrajectoryInBestCohort(electrodeName);
    ContactInfo::Pointer contact = electrode->GetOneContact(iContact);
    if (std::strcmp(contactName.c_str(), contact->GetContactName().c_str()) != 0) return;

    contact->SetCentralPoint(contactPosition);
    electrode->ReplaceContact(contact, iContact);

    // update table ui location
    if (contact->m_IsLocationSet)
    {
        {
            QTableWidgetItem * item = new QTableWidgetItem();
            item->setText( contact->GetContactLocation().c_str() );
            table->setItem(iContact, 4, item);
        }

        {
            QTableWidgetItem * item = new QTableWidgetItem();
            item->setText( QString::number(contact->GetProbabilityOfLocation(),'g',3) );
            table->setItem(iContact, 5, item);
        }
    }

    m_SavedPlansData[iElec].m_PointRepresentation->SetPointPosition(iContact, contactPosition);

    ///TODO: work in progress
    /// - update probability and location
    /// - write fonctions tocaltucate probaility in SEEGContactsROIPipeline based on 3d points (not electrode indices)
    /// - update cohort info (make sure UI does not reload electrode info)
    /// - change contact color in 3D representation




//    double pos_t1[3];
//    api->GetCursorPosition(pos_t1);
//    m_AllPlans[iElec].targetPoint[0] = pos_t1[0];
//    m_AllPlans[iElec].targetPoint[1] = pos_t1[1];
//    m_AllPlans[iElec].targetPoint[2] = pos_t1[2];
//    m_AllPlans[iElec].isTargetSet = true;

//    onUpdateElectrode(iElec);
}

void SEEGAtlasWidget::on_spinBoxElectrodeLineThickness_valueChanged(int value)
{
    m_ElectrodeDisplayWidth = value;
    Q_ASSERT(m_pluginInterface);

    IbisAPI * api = m_pluginInterface->GetIbisAPI();
    QProgressDialog * progress = api->StartProgress(MAX_VISIBLE_PLANS, tr("Üpdating electrode display..."));

    for( int iElec = 0; iElec < MAX_VISIBLE_PLANS; iElec++ )
    {
        if( m_AllPlans[iElec].isEntrySet && m_AllPlans[iElec].isTargetSet )
        {
            m_SavedPlansData[iElec].m_ElectrodeDisplay.m_CylObj->SetLineWidth(m_ElectrodeDisplayWidth);
            for( CylinderDisplay cylinder : m_SavedPlansData[iElec].m_ContactsDisplay )
            {
                cylinder.m_CylObj->SetLineWidth(m_ElectrodeDisplayWidth);
            }
        }
        api->UpdateProgress(progress, iElec);
    }

    api->StopProgress(progress);
}

void SEEGAtlasWidget::on_checkBoxShowContactRadius_stateChanged(int checked)
{
    for (int iElec = 0; iElec < GetNumberElectrodes(); ++iElec)
    {
        if(m_SavedPlansData[iElec].m_PointRepresentation)
        {
            if(checked == Qt::Unchecked)
            {
                m_SavedPlansData[iElec].m_PointRepresentation->HidePoints();
            }
            else
            {
                m_SavedPlansData[iElec].m_PointRepresentation->ShowPoints();
            }
        }
    }
}
