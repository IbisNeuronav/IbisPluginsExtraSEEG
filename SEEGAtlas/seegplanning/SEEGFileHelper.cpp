#include "SEEGFileHelper.h"
#include "FileUtils.h"
#include "GeneralTransform.h"
#include "SEEGTrajectoryROIPipeline.h"
//#include "SEEGContactsROIPipeline.h"
//#include "itkStatisticsImageFilter.h"

#include <vector>
#include <iostream>
#include <fstream>


using namespace std;

namespace seeg {
/***************** PUBLIC GLOBAL VARIABLES **********************/
    TrajectoryDef m_AllPlans[MAX_SEEG_PLANS];

    // labelColors is the same as in labelvolumetosurface plugin object
    double labelColors[256][3] = {{0,0,0},
                                            {1,0,0},
                                            {0,1,0},
                                            {0,0,1},
                                            {0,1,1},
                                            {1,0,1},
                                            {1,1,0},
                                            {0.541176,0.168627,0.886275},
                                            {1,0.0784314,0.576471},
                                            {0.678431,1,0.184314},
                                            {0.12549,0.698039,0.666667},
                                            {0.282353,0.819608,0.8},
                                            {0.627451,0.12549,0.941176},
                                            {1,1,1},
                                            {0.4,0,0},
                                            {0.4,0.2,0},
                                            {0.4,0.4,0},
                                            {0.2,0.4,0},
                                            {0,0.4,0},
                                            {0,0.4,0.2},
                                            {0,0.4,0.4},
                                            {0,0.2,0.4},
                                            {0,0,0.4},
                                            {0.2,0,0.4},
                                            {0.4,0,0.4},
                                            {0.4,0,0.2},
                                            {0.4,0,0},
                                            {0.4,0.2,0},
                                            {0.4,0.4,0},
                                            {0.2,0.4,0},
                                            {0,0.4,0},
                                            {0,0.4,0.2},
                                            {0,0.4,0.4},
                                            {0,0.2,0.4},
                                            {0,0,0.4},
                                            {0.2,0,0.4},
                                            {0.4,0,0.4},
                                            {0.4,0,0.2},
                                            {0.4,0,0},
                                            {0.4,0.2,0},
                                            {0.4,0.4,0},
                                            {0.2,0.4,0},
                                            {0,0.4,0},
                                            {0,0.4,0.2},
                                            {0,0.4,0.4},
                                            {0,0.2,0.4},
                                            {0,0,0.4},
                                            {0.2,0,0.4},
                                            {0.4,0,0.4},
                                            {0.4,0,0.2},
                                            {0.701961,0,0},
                                            {0.701961,0.34902,0},
                                            {0.701961,0.701961,0},
                                            {0.34902,0.701961,0},
                                            {0,0.701961,0},
                                            {0,0.701961,0.34902},
                                            {0,0.701961,0.701961},
                                            {0,0.34902,0.701961},
                                            {0,0,0.701961},
                                            {0.34902,0,0.701961},
                                            {0.701961,0,0.701961},
                                            {0.701961,0,0.34902},
                                            {0.4,0,0},
                                            {0.4,0.2,0},
                                            {0.4,0.4,0},
                                            {0.2,0.4,0},
                                            {0,0.4,0},
                                            {0,0.4,0.2},
                                            {0,0.4,0.4},
                                            {0,0.2,0.4},
                                            {0,0,0.4},
                                            {0.2,0,0.4},
                                            {0.4,0,0.4},
                                            {0.4,0,0.2},
                                            {0.54902,0,0},
                                            {0.54902,0.27451,0},
                                            {0.54902,0.54902,0},
                                            {0.27451,0.54902,0},
                                            {0,0.54902,0},
                                            {0,0.54902,0.27451},
                                            {0,0.54902,0.54902},
                                            {0,0.27451,0.54902},
                                            {0,0,0.54902},
                                            {0.27451,0,0.54902},
                                            {0.54902,0,0.54902},
                                            {0.54902,0,0.27451},
                                            {0.701961,0,0},
                                            {0.701961,0.34902,0},
                                            {0.701961,0.701961,0},
                                            {0.34902,0.701961,0},
                                            {0,0.701961,0},
                                            {0,0.701961,0.34902},
                                            {0,0.701961,0.701961},
                                            {0,0.34902,0.701961},
                                            {0,0,0.701961},
                                            {0.34902,0,0.701961},
                                            {0.701961,0,0.701961},
                                            {0.701961,0,0.34902},
                                            {0.85098,0,0},
                                            {0.85098,0.423529,0},
                                            {0.85098,0.85098,0},
                                            {0.423529,0.85098,0},
                                            {0,0.85098,0},
                                            {0,0.85098,0.423529},
                                            {0,0.85098,0.85098},
                                            {0,0.423529,0.85098},
                                            {0,0,0.85098},
                                            {0.423529,0,0.85098},
                                            {0.85098,0,0.85098},
                                            {0.85098,0,0.423529},
                                            {0.4,0,0},
                                            {0.4,0.2,0},
                                            {0.4,0.4,0},
                                            {0.2,0.4,0},
                                            {0,0.4,0},
                                            {0,0.4,0.2},
                                            {0,0.4,0.4},
                                            {0,0.2,0.4},
                                            {0,0,0.4},
                                            {0.2,0,0.4},
                                            {0.4,0,0.4},
                                            {0.4,0,0.2},
                                            {0.47451,0,0},
                                            {0.47451,0.239216,0},
                                            {0.47451,0.47451,0},
                                            {0.239216,0.47451,0},
                                            {0,0.47451,0},
                                            {0,0.47451,0.239216},
                                            {0,0.47451,0.47451},
                                            {0,0.239216,0.47451},
                                            {0,0,0.47451},
                                            {0.239216,0,0.47451},
                                            {0.47451,0,0.47451},
                                            {0.47451,0,0.239216},
                                            {0.54902,0,0},
                                            {0.54902,0.27451,0},
                                            {0.54902,0.54902,0},
                                            {0.27451,0.54902,0},
                                            {0,0.54902,0},
                                            {0,0.54902,0.27451},
                                            {0,0.54902,0.54902},
                                            {0,0.27451,0.54902},
                                            {0,0,0.54902},
                                            {0.27451,0,0.54902},
                                            {0.54902,0,0.54902},
                                            {0.54902,0,0.27451},
                                            {0.623529,0,0},
                                            {0.623529,0.313725,0},
                                            {0.623529,0.623529,0},
                                            {0.313725,0.623529,0},
                                            {0,0.623529,0},
                                            {0,0.623529,0.313725},
                                            {0,0.623529,0.623529},
                                            {0,0.313725,0.623529},
                                            {0,0,0.623529},
                                            {0.313725,0,0.623529},
                                            {0.623529,0,0.623529},
                                            {0.623529,0,0.313725},
                                            {0.701961,0,0},
                                            {0.701961,0.34902,0},
                                            {0.701961,0.701961,0},
                                            {0.34902,0.701961,0},
                                            {0,0.701961,0},
                                            {0,0.701961,0.34902},
                                            {0,0.701961,0.701961},
                                            {0,0.34902,0.701961},
                                            {0,0,0.701961},
                                            {0.34902,0,0.701961},
                                            {0.701961,0,0.701961},
                                            {0.701961,0,0.34902},
                                            {0.776471,0,0},
                                            {0.776471,0.388235,0},
                                            {0.776471,0.776471,0},
                                            {0.388235,0.776471,0},
                                            {0,0.776471,0},
                                            {0,0.776471,0.388235},
                                            {0,0.776471,0.776471},
                                            {0,0.388235,0.776471},
                                            {0,0,0.776471},
                                            {0.388235,0,0.776471},
                                            {0.776471,0,0.776471},
                                            {0.776471,0,0.388235},
                                            {0.85098,0,0},
                                            {0.85098,0.423529,0},
                                            {0.85098,0.85098,0},
                                            {0.423529,0.85098,0},
                                            {0,0.85098,0},
                                            {0,0.85098,0.423529},
                                            {0,0.85098,0.85098},
                                            {0,0.423529,0.85098},
                                            {0,0,0.85098},
                                            {0.423529,0,0.85098},
                                            {0.85098,0,0.85098},
                                            {0.85098,0,0.423529},
                                            {0.92549,0,0},
                                            {0.92549,0.462745,0},
                                            {0.92549,0.92549,0},
                                            {0.462745,0.92549,0},
                                            {0,0.92549,0},
                                            {0,0.92549,0.462745},
                                            {0,0.92549,0.92549},
                                            {0,0.462745,0.92549},
                                            {0,0,0.92549},
                                            {0.462745,0,0.92549},
                                            {0.92549,0,0.92549},
                                            {0.92549,0,0.462745},
                                            {0.4,0,0},
                                            {0.4,0.2,0},
                                            {0.4,0.4,0},
                                            {0.2,0.4,0},
                                            {0,0.4,0},
                                            {0,0.4,0.2},
                                            {0,0.4,0.4},
                                            {0,0.2,0.4},
                                            {0,0,0.4},
                                            {0.2,0,0.4},
                                            {0.4,0,0.4},
                                            {0.4,0,0.2},
                                            {0.439216,0,0},
                                            {0.439216,0.219608,0},
                                            {0.439216,0.439216,0},
                                            {0.219608,0.439216,0},
                                            {0,0.439216,0},
                                            {0,0.439216,0.219608},
                                            {0,0.439216,0.439216},
                                            {0,0.219608,0.439216},
                                            {0,0,0.439216},
                                            {0.219608,0,0.439216},
                                            {0.439216,0,0.439216},
                                            {0.439216,0,0.219608},
                                            {0.47451,0,0},
                                            {0.47451,0.239216,0},
                                            {0.47451,0.47451,0},
                                            {0.239216,0.47451,0},
                                            {0,0.47451,0},
                                            {0,0.47451,0.239216},
                                            {0,0.47451,0.47451},
                                            {0,0.239216,0.47451},
                                            {0,0,0.47451},
                                            {0.239216,0,0.47451},
                                            {0.47451,0,0.47451},
                                            {0.47451,0,0.239216},
                                            {0.513725,0,0},
                                            {0.513725,0.254902,0},
                                            {0.513725,0.513725,0},
                                            {0.254902,0.513725,0},
                                            {0,0.513725,0},
                                            {0,0.513725,0.254902},
                                            {0,0.513725,0.513725},
                                            {0,0.254902,0.513725},
                                            {0,0,0.513725},
                                            {0.254902,0,0.513725},
                                            {0.513725,0,0.513725},
                                            {0.513725,0,0.254902},
                                            {0.54902,0,0},
                                            {0,0,0}};

/***************** PRIVATE GLOBAL VARIABLES **********************/

    //    SEEGPathPlanner::Pointer m_SEEGPathPlanners[MAX_SEEG_PLANS];
        SEEGElectrodesCohort::Pointer m_SEEGElectrodesCohort;

    GroupInfoMapType m_GroupInfoMap;


/***************** PRIVATE FUNCTIONS PROTOTYPE *******************/

    static void CreateMRIGroup (const std::string& groupName);


/***************** PUBLIC FUNCTIONS IMPL *************************/

    void AutoLoadSEEGFromBaseDir(const std::string& basedir, const std::string& hemisphere){
        ClearAll();

        // load T1 - preimplantation
        //LoadVolume(VOL_GROUP_T1, basedir + "/" + FILE_T1_HEAD, VOL_T1_HEAD);
        LoadVolume(VOL_GROUP_T1, basedir + "/" + FILE_T1_PRE, VOL_T1_PRE);        
        // load Image - with electrodes (could be MRI or CT with the lectrodes registered to pre implantation MRI)
        LoadVolume(VOL_GROUP_POS, basedir + "/" + FILE_POS_REGTO_PRE, VOL_POS_REGTO_PRE);

        //Load surface volume
        LoadVolume(VOL_GROUP_T1, basedir + "/" + FILE_T1_BRAIN_SURFACE_OBJ, OBJ_T1_BRAIN_SURFACE_OBJ);

        // Load segmented labels (from warped atlas)
        LoadVolume(VOL_GROUP_T1, basedir + "/" + FILE_PRE_ANATOMICAL_REGIONS, VOL_PRE_ANATOMICAL_REGIONS);
        LoadVolume(VOL_GROUP_TAL, basedir + "/" + FILE_PRE_CSF_WM_GM, VOL_PRE_CSF_WM_GM);

        // load segmented files from T1 (same for both hemispheres)
        LoadVolume(VOL_GROUP_T1, basedir + "/" + FILE_T1_BRAIN, VOL_T1_BRAIN);
        LoadVolume(VOL_GROUP_T1, basedir + "/" + FILE_T1_VENTRICLES, VOL_T1_VENTRICLES);
        LoadVolume(VOL_GROUP_T1, basedir + "/" + FILE_T1_SULCI, VOL_T1_SULCI);
        LoadVolume(VOL_GROUP_T1, basedir + "/" + FILE_T1_MIDLINE, VOL_T1_MIDLINE);
        LoadVolume(VOL_GROUP_T1, basedir + "/" + FILE_T1_CAUDATE, VOL_T1_CAUDATE);
        LoadVolume(VOL_GROUP_T1, basedir + "/" + FILE_T1_CORTICAL_GM, VOL_T1_CORTICAL_GM);
        LoadVolume(VOL_GROUP_T1, basedir + "/" + FILE_T1_BRAIN_SURFACE, VOL_T1_BRAIN_SURFACE);
        LoadVolume(VOL_GROUP_T1, basedir + "/" + FILE_T1_MUSCLE, VOL_T1_MUSCLE);
        LoadVolume(VOL_GROUP_T1, basedir + "/" + FILE_T1_CSF, VOL_T1_CSF);


        // use whole skull for angle calculation
        LoadVolume(VOL_GROUP_VEC, basedir + hemisphere + "/" + string(GRALFILE_VECTOR_NORMSKULL) + string(SKULL_DEFAULT_POSTFIX) + string("_dx.mnc"), string(VECTOR_NORM_SKULL)+ DIM0);
        LoadVolume(VOL_GROUP_VEC, basedir + hemisphere + "/" + string(GRALFILE_VECTOR_NORMSKULL) + string(SKULL_DEFAULT_POSTFIX) + string("_dy.mnc"), string(VECTOR_NORM_SKULL)+ DIM1);
        LoadVolume(VOL_GROUP_VEC, basedir + hemisphere + "/" + string(GRALFILE_VECTOR_NORMSKULL) + string(SKULL_DEFAULT_POSTFIX)+ string("_dz.mnc"), string(VECTOR_NORM_SKULL)+ DIM2);

        // Load also default distmap NC GM (only once)
        LoadVolume(VOL_GROUP_DISTMAP, basedir + hemisphere + "/" + FILE_DISTMAP_GM + GM_DEFAULT_POSTFIX + ".mnc", string(VOL_DISTMAP_GM) + string(GM_DEFAULT_POSTFIX));


        //Also Load Frame (if exist)
        LoadVolume(VOL_GROUP_T1, basedir + hemisphere + "/" + string(FILE_T1_ENTRYPOINT) + string(FRAME_DEFAULT_POSTFIX) + string(".mnc"), string(VOL_T1_ENTRYPOINT) + string(FRAME_DEFAULT_POSTFIX));


        //Load EAR
        LoadVolume(VOL_GROUP_T1, basedir + hemisphere + "/" + FILE_T1_EAR, VOL_T1_EAR);

        // load GADO/CTA files
        LoadVolume(VOL_GROUP_GADO, basedir + "/" + FILE_GADO_RAW, VOL_GADO_RAW);
        LoadVolume(VOL_GROUP_GADO, basedir + "/" + FILE_GADO_VESSELNESS, VOL_GADO_VESSELNESS);
        LoadVolume(VOL_GROUP_GADO, basedir + "/" + FILE_GADO_VESSELNESS_BINARY, VOL_GADO_VESSELNESS_BINARY);
        //LoadVolume(VOL_GROUP_GADO, basedir + "/" + FILE_GADO_VESSELNESS_NOTHRESH, VOL_GADO_VESSELNESS_NOTHRESH);

        LoadVolume(VOL_GROUP_CTA, basedir + "/" + FILE_CTA_RAW_NATIVE, VOL_CTA_RAW_NATIVE);
        LoadVolume(VOL_GROUP_CTA, basedir + "/" + FILE_CTA_VESSELNESS, VOL_CTA_VESSELNESS);
        //LoadVolume(VOL_GROUP_CTA, basedir + "/" + FILE_CTA_VESSELNESS_NOTHRESH, VOL_CTA_VESSELNESS_NOTHRESH);
        LoadVolume(VOL_GROUP_CTA, basedir + "/" + FILE_CTA_VESSELNESS_BINARY, VOL_CTA_VESSELNESS_BINARY); //is in native space!
        LoadVolume(VOL_GROUP_CTA, basedir + "/" + FILE_CTA_VESSELNESS_3D, VOL_CTA_VESSELNESS_3D);

        // Load CT files
        LoadVolume(VOL_GROUP_CT, basedir + "/" + FILE_CT_RAW_NATIVE, VOL_CT_RAW_NATIVE);

        // Load Skull (from CT but in native space)
        LoadVolume(VOL_GROUP_T1, basedir + "/" + FILE_SKULL, VOL_SKULL);
        LoadVolume(VOL_GROUP_T1, basedir + "/" + FILE_SKULL_MASK, VOL_SKULL_MASK);

        // load transform files - RIZ20131219 - removed! NOW everthing is sampled to native space in preprocessing
      //  SetTransform(VOL_GROUP_GADO, basedir + "/" + XFM_GD_TO_REF); //GADO should be in native space!
      //  SetTransform(VOL_GROUP_CTA, basedir + "/" + XFM_CTA_TO_REF); //CTA data is now in native space
      //  SetTransform(VOL_GROUP_CT, basedir + "/" + XFM_CT_TO_REF);

        // Load data in Pos implantation (with electrode) space
        LoadVolume(VOL_GROUP_POS, basedir + "/" + FILE_CT_POS_ORIGINALSPACE, VOL_CT_POS_ORIGINALSPACE);
        LoadVolume(VOL_GROUP_POS, basedir + "/" + FILE_T1_POS_ORIGINALSPACE, VOL_T1_POS_ORIGINALSPACE);
        LoadVolume(VOL_GROUP_POS, basedir + "/" + FILE_T1PRE_REGTO_MRPOS, VOL_T1PRE_REGTO_MRPOS);
        LoadVolume(VOL_GROUP_POS, basedir + "/" + FILE_T1PRE_REGTO_CTPOS, VOL_T1PRE_REGTO_CTPOS);

        // Load data in TAL (template) space
        LoadVolume(VOL_GROUP_TAL, basedir + "/" + FILE_TAL_CSF_WM_GM, VOL_TAL_CSF_WM_GM);
        LoadVolume(VOL_GROUP_TAL, basedir + "/" + FILE_TAL_ANATOMICAL_REGIONS, VOL_TAL_ANATOMICAL_REGIONS);
        LoadVolume(VOL_GROUP_TAL, basedir + "/" + FILE_TAL_T1, VOL_TAL_T1);
    }

    void OpenFloatVectorVolume (const std::string& groupName, const std::string& gralName, vector<FloatVolume::Pointer> &vecVol) {

        //reads 3 volumes (gralName_x.mnc gralName_y.mnc gralName_z.mnc containing component of normals in each direction)
        if (!VolumeExists(groupName, gralName+DIM0)) {
            cout << "Volume Not Found in OpenFloatVectorVolume. group: " << groupName << " name: " << gralName+DIM0 << endl;
        }
        DataSetInfo* ds;
        FloatVolume::Pointer vol;
        string str[3];
        str[0].append(DIM0);
        str[1].append(DIM1);
        str[2].append(DIM2);
        for (int i=0; i<3; i++){
            ds = GetDatasetInfo(groupName, gralName + str[i]);
            vol = ReadFloatVolume(ds->filename);
            vecVol.push_back(vol);
        }
    }

    // reset UI
    void ClearAll() {
        m_GroupInfoMap.clear();
    }

    void LoadVolume(   const std::string& groupName,
                       const std::string& filename,
                       const std::string& name) {

        cout << "In LoadVolume: group: " << groupName << " name: " << name << " File: " << filename << endl;

        // first, check if file exist
        if (!IsFileExists(filename)) {
            cout << "In LoadVolume: File " << filename << " not found" << endl;
            return;
        }

        if (m_GroupInfoMap.find(groupName) == m_GroupInfoMap.end()) {
            CreateMRIGroup(groupName);
        }
        DataSetInfo dsInfo;
        dsInfo.filename = filename;
        dsInfo.name = name;
        m_GroupInfoMap[groupName].datasetMap[name] = dsInfo;
    }

    void SetTransform(const std::string& groupName, const std::string& transformFile) {
        // first, check if file exist
        if (!IsFileExists(transformFile)) {
            return;
        }

        m_GroupInfoMap[groupName].transformFile = transformFile;
    }

    void GetTransformFile(const std::string& groupName, std::string& transformFile) {
        transformFile = m_GroupInfoMap[groupName].transformFile;
    }

    FloatVolume::Pointer OpenFloatVolume (const std::string& groupName, const std::string& name) {

        if (!VolumeExists(groupName, name)) {
            cout << "Volume Not Found in OpenFloatVolume. group: " << groupName << " name: " << name << endl;
            return FloatVolume::Pointer();
        }
        return ReadFloatVolume(m_GroupInfoMap[groupName].datasetMap[name].filename);
    }

    IntVolume::Pointer OpenIntVolume (const std::string& groupName, const std::string& name) {
        if (!VolumeExists(groupName, name)) {
            cout << "Volume Not Found in OpenIntVolume. group: " << groupName << " name: " << name << endl;
            return IntVolume::Pointer();
        }
        return ReadIntVolume(m_GroupInfoMap[groupName].datasetMap[name].filename);
    }

    ByteVolume::Pointer OpenByteVolume (const std::string& groupName, const std::string& name) {
        if (!VolumeExists(groupName, name)) {
            cout << "Volume Not Found in OpenByteVolume. group: " << groupName << " name: " << name << endl;
            return ByteVolume::Pointer();
        }

        return ReadByteVolume(m_GroupInfoMap[groupName].datasetMap[name].filename);
    }

    GeneralTransform::Pointer OpenTransform(const std::string& groupName, bool invert) {
        if (m_GroupInfoMap[groupName].transformFile != "") {
            return GeneralTransform::New(m_GroupInfoMap[groupName].transformFile, invert);
        }
        return GeneralTransform::Pointer();
    }


    void GetMRIGroupList(std::list<std::string>& groupList) {
        GroupInfoMapType::iterator it;
        groupList.clear();
        for (it=m_GroupInfoMap.begin(); it != m_GroupInfoMap.end(); it++) {
            groupList.push_back((*it).first);
        }
    }

    void GetMRIListForGroup(const std::string& groupName, std::list<std::string>& mriList) {
        mriList.clear();
        if (m_GroupInfoMap.find(groupName) == m_GroupInfoMap.end()) {
            return;
        }

        GroupInfo inf = m_GroupInfoMap[groupName];
        DataSetInfoMapType::iterator it;
        for (it = m_GroupInfoMap[groupName].datasetMap.begin(); it != m_GroupInfoMap[groupName].datasetMap.end(); it++) {
            mriList.push_back((*it).first);
        }
    }

    GroupInfo * GetGroupInfo(const std::string& groupName) {
        if (m_GroupInfoMap.find(groupName) == m_GroupInfoMap.end()) {
            return 0;
        } else {
            return &m_GroupInfoMap[groupName];
        }
    }

    DataSetInfo * GetDatasetInfo(const std::string& groupName, const std::string& name) {
        if (!VolumeExists(groupName, name)) {
            return 0;
        } else {
            return &(m_GroupInfoMap[groupName].datasetMap[name]);
        }
    }

    bool VolumeExists(const std::string& groupName, const std::string& name) {
        if (m_GroupInfoMap.find(groupName) != m_GroupInfoMap.end()) {
            DataSetInfoMapType& m = m_GroupInfoMap[groupName].datasetMap;
            return m.find(name) != m.end();
        }
        return false;
    }


// RIZ July2013: Some of these functions are likely to be moved to PathCohort!!!!
/*    SEEGPathPlanner::Pointer GetSEEGPathPlanners(int indTarget) {
        if (!m_SEEGPathPlanners[indTarget]){
           m_SEEGPathPlanners[indTarget] = SEEGPathPlanner::New();
        }
        return m_SEEGPathPlanners[indTarget];
    }
*/

/*    SEEGElectrodesCohort::Pointer GetSEEGElectrodesCohort() {
        if (!m_SEEGElectrodesCohort){
           //FloatVolume::Pointer vol = OpenFloatVolume("T1", "brain"); //it is only for the sizes
           //m_SEEGElectrodesCohort = SEEGElectrodesCohort::New(vol);
           m_SEEGElectrodesCohort = SEEGElectrodesCohort::New(SEEGElectrodeModel::DIXI15, 0.5); //RIZ: HARDCODED for NOW!!!!
        }
        return m_SEEGElectrodesCohort;
    }
*/
    //RIZ: added setter -- is this correct?
/*    void SetSEEGPathPlanner(SEEGPathPlanner::Pointer pathPlanner, int indTarget) {
        m_SEEGPathPlanners[indTarget] = pathPlanner;
    }
*/


/****************** FILE PRIVATE FUNCTION IMPL ****************/

        static void CreateMRIGroup (const std::string& groupName) {
            GroupInfo inf;
            inf.name = groupName;
            m_GroupInfoMap[groupName] = inf;

        }



}
