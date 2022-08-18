#ifndef __SEEGAtlasWidget_h_
#define __SEEGAtlasWidget_h_

#include <QWidget>
#include <vtkLineSource.h>
#include <vtkHandleWidget.h>
#include <vtkSphereHandleRepresentation.h>
#include <vtkSmartPointer.h>
#include <vtkTubeFilter.h>
#include "imageobject.h"
#include "SEEGFileHelper.h"
#include "SEEGElectrodeModel.h"
#include "SEEGElectrodesCohort.h"
#include "seegatlasplugininterface.h"
#include <QTableWidget>

class SEEGAtlasPluginInterface;
class PolyDataObject;
class Application;
class SceneObject;
class SEEGTrajVisWidget;

namespace Ui
{
    class SEEGAtlasWidget;
}


// To display each surgical plan as a cylinder - it can be used also to create contacts (children of electrode)
struct CylinderDisplay {
    vtkSmartPointer<vtkLineSource> m_Line;
    vtkSmartPointer<vtkTubeFilter> m_Cylinder;
    PolyDataObject *m_CylObj;
};

struct ElectrodeDisplay{
    CylinderDisplay m_ElectrodeDisplay;
    std::vector<CylinderDisplay> m_ContactsDisplay;
    seeg::SEEGElectrodeModel::Pointer m_ElectrodeModel;
};



class SEEGAtlasWidget : public QWidget
{

    Q_OBJECT

public:

    explicit SEEGAtlasWidget(QWidget *parent = 0);
    ~SEEGAtlasWidget();
    void SetPluginInterface(SEEGAtlasPluginInterface * interf);

private:

    Ui::SEEGAtlasWidget * ui;

    vector<string> m_ElectrodesNames;

    seeg::SEEGElectrodeModel::Pointer m_ElectrodeModel;

    float m_SpacingResolution; //RIZ: this might be unnecessary...

    vector<QTableWidget *> m_VectorContactsTables; //vector of tables with trajectory lists (vector size is number of electrodes)

    vector <QColor> m_PosColors;

    seeg::SEEGElectrodesCohort::Pointer m_SEEGElectrodesCohort;


private slots:
    // Dataset management and visualization presets
    void onLoadSEEGDatasetsFromDir();
    void onLoadSEEGDatasetsFromDir(QString dirName);
    void onChangeElectrodeType(QString newType);
    void OnObjectAddedSlot(int imageObjectId);
    void OnObjectRemovedSlot(int imageObjectId);

    // Visualization
    void on_checkBoxImagePlanes_stateChanged(int);
    void on_pushGenerateSurface_clicked();

    // Trajectory planning
    void onSetTargetPointFrom3DCoords();
    void onSetTargetPointFromCursor();
    void onSetEntryPointFrom3DCoords();
    void onSetEntryPointFrom3DSelection();
    void onSetEntryPointFromCursor();
    void addElectrodeToCohort(int iElec);
    void addElectrodeToCohort(int iElec, seeg::ElectrodeInfo::Pointer electrode);
    void onPlanSelect(int iElec);

    void onChangeElectrodeName();
    void onChangeElectrodeName(const string newElectrodeName);
    void onUpdateElectrode(int iElec);
    void onUpdateElectrode(int iElec, seeg::ElectrodeInfo::Pointer electrode);
    void onFindAnatLocation();
    void onFindAnatLocation(seeg::ElectrodeInfo::Pointer electrode);
    void onFindChannelsAnatLocation();
    void onFindChannelsAnatLocation(seeg::ElectrodeInfo::Pointer electrode, const QString dirname);

    // Tables Interactions
    void onTabSelect(int indTab);
    void onTrajectoryTableCellChange(int newRow, int newCol, int oldRow, int oldCol); //Genereic -> should replace the specific for each table
    void onAllElectrodesTableCellChange(int newRow, int newCol, int oldRow, int oldCol); //First tab corresponds to electrodes
    void onTrajectoryTableCellChange(int newRow, int newCol, int oldRow, int oldCol, int iElec); //Genereic -> should replace the specific for each table
    void addTrajectoryTableTab(const int indTab, const string electrodeName);
    void removeContactsTableTab(const int indTab);
    void RefreshContactsTable(const int iElec);
    void RefreshChannelsTable(const int iElec);
    void onShowContactsChannelsTable(bool isChecked);

    //Saving & Loading functions
    void onLoadPlanningFromDirectory();
    void onLoadPlanningFromDirectory(QString dirName);
    void onLoadOnePlanFromCSVFile(const int iElec, const string filename);
    void onSavePlanningToDirectory();
    void onSavePlanningToDirectory(QString dirName);
    void onLoad1Plan();

    // trajectory visualization functions
    void onProbeEyeView();
    void onChangeTrajectoryCylinderRadius(int sliderPos);
    void onChangeTrajectoryCylinderRadius(QString strDiameter);
    void onChangeTrajectoryCylinderLength(int sliderPos);
    void onChangeTrajectoryCylinderLength(QString strPos);
    void onGetLocationValue();
    //contact and channel visualization
    void onCreateAllElectrodesWithSelectedContacts();
    void onCreateAllElectrodesWithSelectedChannels();

    //Batch Analysis
    void onRunBatchAnalysis();


private:

   void SelectTrajectoryInList(seeg::Point3D targetPoint, seeg::Point3D entryPoint);
   int getLocationValue(seeg::Point3D point);

   void InitUI();

    // Create and Display trajectory cylinders
   // void DisplaySavedPlan(int iElec);
   // void DisplayAllSavedPlan();
   // void SetTrajectoryCursor(seeg::Point3D targetPoint,seeg::Point3D entryPoint);
    void FillComboBoxBrainSegmentation();
    void CreateAllElectrodes();
    void CreateElectrode(const int iElec);
    void CreateElectrode(const int iElec, seeg::Point3D pDeep, seeg::Point3D pSurface);
    void DeleteElectrode(const int iElec);
    void CreateActivePlan();
    void createAllElectrodesWithSelectedContactsChannels(const string whichSelected);
    void CreateElectrodeWithSelectedContacts(const int iElec);
    void CreateElectrodeWithSelectedContacts(const int iElec, seeg::Point3D pDeep, seeg::Point3D pSurface, vector<bool> isContactSelected, std::vector<int> colorsPerContacts);
    void CreateElectrodeWithSelectedChannels(const int iElec);
    void CreateElectrodeWithSelectedChannels(const int iElec, seeg::Point3D pDeep, seeg::Point3D pSurface, vector<bool> isChannelSelected, std::vector<int> colorsPerChannels);
    


    // loading the datasets
//    void LoadAnatData();
    void LoadAnatData();
    void LoadCTData();
    //    void LoadSWIData();
//    void LoadTOFData();
    void LoadGadoData();
    void LoadCTAData();
    void LoadAnatDataPosSpace();
    void LoadAnatTemplateSpace();

    // for refreshing various part of the user interface
    void RefreshVisualization();
    void RefreshPlanningScores();
    void RefreshPlanCoords(int iElec);
    void RefreshPlanCoords(int iElec, string electrodeName);
    void RefreshAllPlanCoords();
   // void RefreshTrajectories();

    //
    //Since VTK6 we need to re-connect the data object to the vtk filters pipeline to update the whole pipeline
    void UpdateActivePlan();
    void UpdatePlan(int iElec);

    //clean up
    void ResetElectrodes();

    SEEGAtlasPluginInterface * m_pluginInterface;

    // Atlas
    seeg::FloatVolume::Pointer openAtlasVolume();
    map <int,string> ReadAtlasLabels();

    //various
    string timeStamp();

    // Data about the active trajectory (user can interact with that cylinder)
    CylinderDisplay m_ActivePlanData;
   // vtkSmartPointer<vtkHandleWidget> m_EntryPointHandle;
  //  vtkSmartPointer<vtkSphereHandleRepresentation> m_EntryPointHandleRepresentation;  //RIZ20151207 Gives a segmentation fault!


    // Data about saved plans (cannot interact with those cynlinders)
    ElectrodeDisplay m_SavedPlansData[MAX_SEEG_PLANS];

    // Pop-up for visualization of Probe's eye and trajectory views
    SEEGTrajVisWidget *m_TrajVisWidget;

    // Main scene objects created by the plugin
    SceneObject *m_PlanningRootObject;
    SceneObject *m_TrajPlanMainObject;
    SceneObject *m_SavedPlansObject;

    // Computer-assisted planning data (color-coded volume of the trajectory scores)
    // ImageObject *m_TrajectoryScoresObject;
    // IbisItk3DImageType::Pointer m_TrajectoryScores[MAX_SEEG_PLANS];

    // keep the ID of the T1 object
    int m_T1objectID;

    // Getters & Setters of electrode names - it is repeated in Planning Dialog
    void AddElectrodeName(const std::string electrodeName);
    std::vector<std::string> GetElectrodeNames();
    void SetElectrodeNames(std::vector<std::string> electrodeNames);
    void ReplaceElectrodeName(const int indexElectrode, const std::string electrodeName);
    int GetNumberElectrodes();

    // Getter/ Setter Cohort
    seeg::SEEGElectrodesCohort::Pointer GetSEEGElectrodesCohort();

    //Visualization functions
    CylinderDisplay CreateCylinderObj(QString name, seeg::Point3D p1, seeg::Point3D p2);
    std::vector<CylinderDisplay> CreateContactCylinderObj(QString gralName, seeg::Point3D electrodeTip, seeg::Point3D entryPoint, seeg::SEEGElectrodeModel::Pointer electrodeModel);
    std::vector<CylinderDisplay> CreateChannelCylinderObj(QString gralName, seeg::Point3D electrodeTip, seeg::Point3D entryPoint, seeg::SEEGElectrodeModel::Pointer electrodeModel);

};

#endif
