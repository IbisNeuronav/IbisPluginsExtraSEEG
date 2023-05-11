#ifndef SEEGELECTRODESCOHORT_H
#define SEEGELECTRODESCOHORT_H


//#include "SEEGPathPlanner.h"
//#include "SEEGTrajectoryROIPipeline.h"
#include "SEEGElectrodeModel.h"
#include "ElectrodeInfo.h"
#include <SEEGPointRepresentation.h>

using namespace std;
namespace seeg {

class SEEGElectrodesCohort
{
    private:
        map<string, ElectrodeInfo::Pointer> m_BestCohort; //Cohort of best electrodes - key is electrode name (usually corresponds to anatomical location of target)

        map<string,int> m_BestCohortTrajIndex; //index in active trajectories that each best corresponds to

  //      map<string, SEEGPathPlanner::Pointer> m_Cohort; //Cohort including all active trajectories - key is electrode name (usually corresponds to anatomical location of target)

        map<string, vector<Point3D> > m_BestCohortTrPoints; //all points in trajectory corresponding to each electrode - key is electrode name (usually corresponds to anatomical location of target)

        // SEEGTrajectoryROIPipeline::Pointer m_PipelineTrajectory; // Piepeline trajectory object useful to compute all points in trajectories

        SEEGElectrodeModel::Pointer m_ElectrodeModel; // electrodeModel could be selected as MNI or DIXI

        float m_SpacingResolution; // spacing to consider when looking at all the points in a trajectory

        SEEGPointRepresentation::Pointer m_Points;

    public:
        // smart pointer
        typedef mrilSmartPtr<SEEGElectrodesCohort> Pointer;

    public:
         /**
          * Constructor & Destructor
          */
         SEEGElectrodesCohort();
         SEEGElectrodesCohort(FloatVolume::Pointer templateVolume);
         SEEGElectrodesCohort(SEEGElectrodeModel::Pointer model, float spacing);

         static Pointer New() {return Pointer(new SEEGElectrodesCohort());}
         static Pointer New(FloatVolume::Pointer templateVolume) {return Pointer(new SEEGElectrodesCohort(templateVolume));} //to initialize the m_PipelineTrajectory object
         static Pointer New(SEEGElectrodeModel::Pointer model, float spacing) {return Pointer(new SEEGElectrodesCohort(model, spacing));} //to initialize the m_ElectrodeModel object with appropriate electrode model

         virtual ~SEEGElectrodesCohort();

         /**
         * Reset SEEGElectrodesCohort
         */
         void Reset();

         /**
         * Reset only Best cohort -> leave cohort intact
         */
         void ResetBestCohort();

         /*** Getters & Setters ***/
         /**
         * Accessor for the list of trajectories in best cohort
         *
         * @return a list of best trajectories
         */
         const map<string, ElectrodeInfo::Pointer > & GetBestCohort();

         ElectrodeInfo::Pointer GetTrajectoryInBestCohort(const string& electrodeName);

         const map<string, int> & GetBestCohortIndexes();

         int GetTrajectoryIndexInBestCohort(const string& electrodeName);

         void AddTrajectoryToBestCohort(const string& electrodeName, ElectrodeInfo::Pointer electrode, const int indexElectrode);

         void AddAllFirstTrajToBestCohort(float extraLength);

         void ChangeTrajectoryNameInBestCohort(const string& oldElectrodeName, const string& newElectrodeName);

         void RemoveElectroedFromBestCohort(const string& electrodeName);

         int GetNumberOfElectrodesInCohort(); //returns number of locations (electrodes)

         void getAllPointsTrajBestCohort(const float extraLength);

         vector<string> GetElectrodeNames();

         float GetSpacingResolution();

         void SetSpacingResolution(const float spacing);

         SEEGElectrodeModel::Pointer GetElectrodeModel();

         void SetElectrodeModel(SEEGElectrodeModel::Pointer electrodeModel);

         /*** Load & Save ***/
         bool LoadSEEGBestCohortDataFromFile (const string& filename, char delimiter, std::vector<seeg::SEEGElectrodeModel::Pointer> modelList);

         void SaveSEEGBestCohortDataToFile(const string& filename, char delimiter);


};

}
#endif // SEEGELECTRODESCOHORT_H
