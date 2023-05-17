#include "SEEGElectrodesCohort.h"

using namespace std;


namespace seeg {

    /**** CONSTRUCTOR/DESTRUCTOR ****/
    SEEGElectrodesCohort::SEEGElectrodesCohort()
    {
        // default: MNI electrode and 0.5mm resolution
        SEEGElectrodesCohort(SEEGElectrodeModel::New(), 0.5);
    }

    SEEGElectrodesCohort::SEEGElectrodesCohort(FloatVolume::Pointer templateVolume){
      // m_PipelineTrajectory = SEEGTrajectoryROIPipeline::New(templateVolume);
    }

    SEEGElectrodesCohort::SEEGElectrodesCohort(SEEGElectrodeModel::Pointer model, float spacing){
        m_ElectrodeModel = model;
        m_SpacingResolution = spacing;
    }

    SEEGElectrodesCohort::~SEEGElectrodesCohort() {
        // nothing to do
    }

    void SEEGElectrodesCohort::Reset() {
        this->m_BestCohort.clear();
        //this->m_Cohort.clear();
    }

    void SEEGElectrodesCohort::ResetBestCohort(){
        this->m_BestCohort.clear();
    }

    /***** Getters and Setters *****/
    //returns the list of best trajectories (full cohort)
    const map<string, ElectrodeInfo::Pointer> &SEEGElectrodesCohort::GetBestCohort() {
        return this->m_BestCohort;
    }

    //returns the trajectory in the best cohort that correponds to the specified location (electrode name)
    ElectrodeInfo::Pointer  SEEGElectrodesCohort::GetTrajectoryInBestCohort(const string& electrodeName) {
        return this->m_BestCohort[electrodeName];
    }

    //returns the list of inidces of best trajectories (to which active trajectry correponds)
    const map<string, int> &SEEGElectrodesCohort::GetBestCohortIndexes() {
        return this->m_BestCohortTrajIndex;
    }

    //returns the index of the trajectory in the best cohort that correponds to the specified location (electrode name)
   int SEEGElectrodesCohort::GetTrajectoryIndexInBestCohort(const string& electrodeName) {
        return this->m_BestCohortTrajIndex[electrodeName];
    }

     //Sets a trajectory in the best cohort that correponds to the specified location (electrode name)
    void SEEGElectrodesCohort::AddTrajectoryToBestCohort(const string& electrodeName, ElectrodeInfo::Pointer electrode, const int indexElectrode) {
        this->m_BestCohort[electrodeName] = electrode;
        this->m_BestCohortTrajIndex[electrodeName] = indexElectrode;
    }

    // Modify name of electrode by removing this electrode an adding a new one (only way since electrode name is the key)
    void SEEGElectrodesCohort::ChangeTrajectoryNameInBestCohort(const string& oldElectrodeName, const string& newElectrodeName) {
       // Find electrode
        ElectrodeInfo::Pointer electrode = GetTrajectoryInBestCohort(oldElectrodeName);
        if( electrode )
        {
            int indexElectrode = GetTrajectoryIndexInBestCohort(oldElectrodeName);
            // remove electrode
            RemoveElectroedFromBestCohort(oldElectrodeName);
            //add electrode back under new name
            this->m_BestCohort[newElectrodeName] = electrode;
            this->m_BestCohortTrajIndex[newElectrodeName] = indexElectrode;
        }
    }

    void SEEGElectrodesCohort::RemoveElectroedFromBestCohort(const string& electrodeName) {
        this->m_BestCohort.erase(electrodeName);
    }

    //Sets all first active trajectories from m_cohort in the best cohort - for the specified lengthElectrode (could be longer than entry point)
/*    void SEEGElectrodesCohort::AddAllFirstTrajToBestCohort(const float extraLength){
        map<string, SEEGPathPlanner::Pointer>::const_iterator it1;
        for (it1 = m_Cohort.begin(); it1 != m_Cohort.end(); it1++) {
            string electrodeName = it1->first;
            list<ElectrodeInfo::Pointer> activeTrajectories = m_Cohort[electrodeName]->GetActiveTrajectories();

            AddTrajectoryToBestCohort(electrodeName, activeTrajectories.front(), 0); //assume they are ordered--> first one is the one with highest score --> index is zero
        }
        // Get bounding box for best cohort - used to compted crossing
        getAllPointsTrajBestCohort(extraLength);
    }
*/

    int SEEGElectrodesCohort::GetNumberOfElectrodesInCohort(){
        return this->m_BestCohort.size();
    }

    vector<string> SEEGElectrodesCohort::GetElectrodeNames(){
        vector<string> electNames;
        for(map<string, ElectrodeInfo::Pointer>::iterator it = m_BestCohort.begin(); it != m_BestCohort.end(); it++) {
          electNames.push_back(it->first);
        }
        return electNames;
    }

    float SEEGElectrodesCohort::GetSpacingResolution(){
         return m_SpacingResolution;
     }

    void SEEGElectrodesCohort::SetSpacingResolution(const float spacing){
         m_SpacingResolution = spacing;
     }

    SEEGElectrodeModel::Pointer SEEGElectrodesCohort::GetElectrodeModel(){
        return m_ElectrodeModel;
    }

    void SEEGElectrodesCohort::SetElectrodeModel(SEEGElectrodeModel::Pointer electrodeModel){
        m_ElectrodeModel = electrodeModel; //assign type of electrode to cohort
        // change also in each electrode in best cohort
        //map<string, ElectrodeInfo::Pointer> bestCohort = GetBestCohort();

        //// Get bounding box of each electrode
        //map<string, ElectrodeInfo::Pointer>::const_iterator it1;
        //int iElec;
        //for (iElec=0,it1 = bestCohort.begin(); it1 != bestCohort.end(); iElec++,it1++) {
        //    ElectrodeInfo::Pointer electrode = it1->second;
        //    if( electrode )
        //    {
        //        electrode->SetElectrodeModel(electrodeModel);
        //    }
        //}
    }

    /*** Points along trajectory ***/
    void SEEGElectrodesCohort::getAllPointsTrajBestCohort(const float extraLength){
        // Get all electrodes in best cohort
        map<string, ElectrodeInfo::Pointer> bestCohort = GetBestCohort();

        // Get bounding box of each electrode
        map<string, ElectrodeInfo::Pointer>::const_iterator it1;
        int iPlan;
        for (iPlan=0,it1 = bestCohort.begin(); it1 != bestCohort.end(); iPlan++,it1++) {
            ElectrodeInfo::Pointer electrode = it1->second;
            if( electrode )
            {
                string electrodeName = it1->first;
                //get all points in each trajectory
                //m_PipelineTrajectory->GetAllPointsAroundTrajectory(electrode->GetTargetPoint(), electrode->GetEntryPoint(), 0, allPointsPerTraj); //3rd argument ius not used-> it should be removed!
                vector<Point3D> allPointsPerTraj;
                Point3D entryPtExtrapolated;
                Point3D tp = electrode->GetTargetPoint();
                Point3D ep = electrode->GetEntryPoint();

                m_BestCohort[electrodeName]->ExtrapolateEntryPoint(tp, ep, entryPtExtrapolated, extraLength);
                m_ElectrodeModel->CalcAllLinePositions(m_SpacingResolution, electrode->GetTargetPoint(), entryPtExtrapolated, allPointsPerTraj);
                m_BestCohortTrPoints[electrodeName] = allPointsPerTraj;
            }
        }
    }

    /*** Save & Load functions ***/
    bool SEEGElectrodesCohort::LoadSEEGBestCohortDataFromFile (const string& filename, const char delimiter, std::vector<seeg::SEEGElectrodeModel::Pointer> modelList) {
        //only load bestCohort - weights are already obtained from each trajectory file
        ifstream file;
        string line("notempty");
        string token;
        string testName;
        SEEGElectrodeModel::Pointer electrodeModel;

        bool status=false;
        ResetBestCohort();

        file.open(filename.c_str());

        while(~file.eof() && line.size()>0) {
            getline(file, line);
            stringstream ss (line);

            token="";
            getline(ss, token, delimiter);

            if (token == "[fileType]") {

                getline(ss, token, delimiter);
                testName = token;

            }  else if(token == "[electrodeType]") { //electrode type line contains electrotype (e.g. MNI) and spacing (e.g. 0.5)
                getline(ss, token, delimiter);
                
                // look for electrode in config dir
                seeg::SEEGElectrodeModel::Pointer electrodeModel;
                bool found = false;
                for( int i = 0; (i < modelList.size()) && !found; ++i )
                {
                    if( modelList[i]->GetElectrodeId().compare(token.c_str()) == 0 )
                    {
                        electrodeModel = modelList[i];
                        found = true;
                    }

                }
                if( !found )
                {
                    // electrode does not exist in config directory
                    //TODO: display warning message to user
                    return false;
                }

                this->SetElectrodeModel(electrodeModel);

                getline(ss, token, delimiter);
                m_SpacingResolution = atof(token.c_str()); //RIZ: this might be unnecessary!!
            } else if (token == "[trajectory]") {
                Point3D entryPointWorld, targetPointWorld;
                string electrodeName;
                int isElectrodeValid;

                getline(ss, token, delimiter);
                electrodeName = token;
                getline(ss, token, delimiter);
                int indexElectrode = atof(token.c_str());

                getline(ss, token, delimiter);
                targetPointWorld[0] = atof(token.c_str());
                getline(ss, token, delimiter);
                targetPointWorld[1] = atof(token.c_str());
                getline(ss, token, delimiter);
                targetPointWorld[2] = atof(token.c_str());
                getline(ss, token, delimiter);
                entryPointWorld[0] = atof(token.c_str());
                getline(ss, token, delimiter);
                entryPointWorld[1] = atof(token.c_str());
                getline(ss, token, delimiter);
                entryPointWorld[2] = atof(token.c_str());
                getline(ss, token, delimiter);
                isElectrodeValid = atoi(token.c_str()); //RIZ20151201: Should we use this to validate??

                ElectrodeInfo::Pointer electrode = ElectrodeInfo::New(entryPointWorld, targetPointWorld, m_ElectrodeModel, electrodeName);
                AddTrajectoryToBestCohort(electrodeName, electrode, indexElectrode);
                status=true;

            } else if (token == "[order]") {
                cout <<"order:"<< token.c_str()<<std::endl;
                getline(ss, token, delimiter);
                break;
            } else {
                cout << token.c_str() << " not found: Possible Parse error\n";
              break;
            }
        }

        file.close();
        return status;
    }


    void SEEGElectrodesCohort::SaveSEEGBestCohortDataToFile(const string& filename, const char delimiter){
        // Saves best trajectory
        cout<<"Saving All Electrodes information to "<<filename<< std::endl;
        ofstream file;
        file.open(filename.c_str());
        file << "[fileType]"<< delimiter<< "Cohort" << std::endl;
        file << "[electrodeType]"<< delimiter<< m_ElectrodeModel->GetElectrodeId() <<delimiter<< m_SpacingResolution << std::endl; //RIZ: this might be removed...

        //Get weights info from first trajectory in full cohort
      //  string electrodeName = m_Cohort.begin()->first;
     //   SEEGPathPlanner::Pointer tr = m_Cohort[electrodeName];

        //Coordinates per trajectory
         map<string,ElectrodeInfo::Pointer>::iterator elecIt;
        for (elecIt = m_BestCohort.begin(); elecIt != m_BestCohort.end(); elecIt++) {
            ElectrodeInfo::Pointer el = elecIt->second;
            if( el )
            {
                string elName = elecIt->first;
                int indexElectrode = GetTrajectoryIndexInBestCohort(elName);
                //file << "[electrodeName] "<< elName.c_str() << delimiter << endl; // include as separate line
                file << "[trajectory]" << delimiter;
                file << elName.c_str() << delimiter;  //include name in trajectory row
                file << indexElectrode << delimiter;  //include index in Active Trajectory list in trajectory row
                file << el->m_TargetPointWorld[0] << delimiter;
                file << el->m_TargetPointWorld[1] << delimiter;
                file << el->m_TargetPointWorld[2] << delimiter;
                file << el->m_EntryPointWorld[0] << delimiter;
                file << el->m_EntryPointWorld[1] << delimiter;
                file << el->m_EntryPointWorld[2] << delimiter;
                file << el->m_Valid << delimiter;

                //finish trajectory line and move to next trajectory in Best Cohort
                file << std::endl;
            }
        }
        file << "[order]"<< delimiter<< "ElectrodeName"<< delimiter<<"indexElectrode"<< delimiter<<"targetx"<< delimiter<<"targety"<< delimiter<<"targetz"<< delimiter<<"entryx"<< delimiter<<"entryy"<< delimiter<<"entryz"<< std::endl;

        file.close();

    }

}

