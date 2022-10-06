#ifndef __SEEGFILEHELPER_H__
#define __SEEGFILEHELPER_H__

#include "VolumeTypes.h"
#include "GeneralTransform.h"
#include <string>
#include <sstream>
#include <map>
//#include "SEEGPathPlanner.h"
//#include "Histogram.h"
//#include "PlanningUI.h"
#include "SEEGElectrodesCohort.h"


namespace seeg {

/*************** PUBLIC CONSTANT DEFINITION ************************/

#define DELIMITER_TRAJFILE ',' //char(32)   //',' // ' '=char(32)
#define DELIMITER_TRAJFILE_2OPTION char(32)

// Default group (used by AutoLoadSEEGFromBaseDir())
#define VOL_GROUP_T1 "T1"
#define VOL_GROUP_GADO "GADO"
#define VOL_GROUP_DISTMAP "DistMap"
#define VOL_GROUP_VEC "Vector"
#define VOL_GROUP_CTA "CTA"
#define VOL_GROUP_CT "CT"
#define VOL_GROUP_POS "WithElectrodes" // pos implantation space
#define VOL_GROUP_TAL "Template" // TAL space

//RIZ 20151123 - Added for Atlas SEEG
//These correspond to T1 group
#define VOL_T1_PRE "MRIpriorImplantation"
#define FILE_T1_PRE "input/prior_to_implantation.mnc"
#define VOL_POS_REGTO_PRE "WithElectrodes"              //It could be MRI or CT
#define FILE_POS_REGTO_PRE "input/pos_regto_pre.mnc"

//These correspond to "WithElectrodes" group
#define VOL_T1_POS_ORIGINALSPACE "MRIwithElectrodesOriginalSpace"        // We are not loading the images with electrodes in the original space here
#define FILE_T1_POS_ORIGINALSPACE "input/posspace/MRI_with_electrodes.mnc"
#define VOL_CT_POS_ORIGINALSPACE "CTwithElectrodesOriginalSpace"
#define FILE_CT_POS_ORIGINALSPACE "input/posspace/ctpos_th0_center0.mnc" // instead of CT_with_electrodes.mnc to use a thresholded centered file
#define VOL_T1PRE_REGTO_MRPOS "MRIpriorToMRI"              //MRI prior to implantation registered to data with electrodes (in pos implantation space)
#define FILE_T1PRE_REGTO_MRPOS "input/posspace/mrpre_reg_to_mrpos.mnc"
#define VOL_T1PRE_REGTO_CTPOS "MRIpriorToCT"              //MRI prior to implantation registered to data with electrodes (in pos implantation space)
#define FILE_T1PRE_REGTO_CTPOS "input/posspace/mrpre_reg_to_ctpos.mnc"

//These correspond to group "Template"
#define VOL_TAL_T1 "TAL_MRI"
#define FILE_TAL_T1 "segmentation/tal_anat_t1w.mnc"
#define VOL_TAL_ANATOMICAL_REGIONS "AnatLabels"
#define FILE_TAL_ANATOMICAL_REGIONS "segmentation/tal_seg_lobes_HCAG.mnc" //tal_seg_lobes.mnc" //tal_seg_lobesHCAG.mnc"
#define VOL_TAL_CSF_WM_GM "CSF_WM_GM"
#define FILE_TAL_CSF_WM_GM "segmentation/tal_seg_cls.mnc"

// Segmentation Atlas
#define VOL_PRE_ANATOMICAL_REGIONS "AnatLabels"
#define FILE_PRE_ANATOMICAL_REGIONS "segmentation/seg_lob_labels.mnc" //ICBM251 2009c atlas  - -seg_atlas_labels_gm.mnc" : MICCAI atlas   //seg_oasis_labels.mnc" //tal_seg_lobesHCAG.mnc"
#define VOL_PRE_CSF_WM_GM "CSF_WM_GM"
#define FILE_PRE_CSF_WM_GM "segmentation/seg_cls_labels.mnc"
#define FILE_ATLAS_LABELS "segmentation/label_map_SEEGAtlas.xml" // MUST be at in EVERY patient segmentation dir // options:  label_map_MICCAI.xml for MICAII atlas / label_map_ICBM.xml for ICBM152 2009 atlas

//FIN addition 20151123


#define VOL_T1_BRAIN "brain"
#define FILE_T1_BRAIN "segmentation/seg_brain.mnc"

#define VOL_T1_BRAIN_MASK "brain_mask"
#define FILE_T1_BRAIN_MASK "segmentation/seg_brain_mask.mnc"

//SEGMENTED REGIONS
#define VOL_T1_VENTRICLES "ventricles"
#define FILE_T1_VENTRICLES "segmentation/seg_ventricles_mask.mnc"

#define VOL_T1_SULCI "sulci"
#define FILE_T1_SULCI "segmentation/seg_sulci_mask.mnc"

#define VOL_T1_MIDLINE "midline"
#define FILE_T1_MIDLINE "segmentation/seg_midline_roi.mnc"

#define VOL_T1_CAUDATE "caudate"
#define FILE_T1_CAUDATE "segmentation/seg_caudate_mask.mnc"

#define VOL_T1_CORTICAL_GM "cortical_gm"
#define FILE_T1_CORTICAL_GM "segmentation/seg_cortical_gm_risk_map.mnc"

#define VOL_T1_BRAIN_SURFACE "brain_surface"
#define FILE_T1_BRAIN_SURFACE "segmentation/tmp/seg_brain_surface_mask.mnc"


// Default volume and filenames (used by AutoLoadSEEGFromBaseDir())
// GENERIC -- RIZ: STILL in PROGRESS
#define FILE_LOC_NAMES "electrode_names.txt"                         // file where name of electrodes is (1 name per line - e.g. AG / HCant / HCpos)
#define POS_TRAJ_DIR "10000_Trajectories" // before: posTraj_Angle_5_Res_0"

//TARGET //RIZ:20140130: before dist maps were small - but some of the true traj were outside!
#define VOL_T1_TARGET "target"                                      // example: targetAG
#define FILE_T1_TARGET "segmentation/seg_target_mask_"              // example: segmentation/seg_target_mask_AG.mnc
#define FILE_TRAJ_TARGET "pos_trajectories_"                        // example: pos_trajectories_AG.csv
#define VOL_DISTMAP_TARGET "distmaptarget"                          // example: distmaptargetAG
#define FILE_DISTMAP_TARGET "distancemaps/distmap_target_"            // example: distancemaps/distmap_target_small_AG.mnc
// GM 2ndary Target -- RIZ: change name - for now only for temporal lobe -needs generalization!!
#define VOL_T1_GM "GM"                                              // example: GMtemporal
#define FILE_T1_GM "segmentation/seg_gm_mask_"                      // example: segmentation/seg_gm_mask_temporal.mnc
#define VOL_DISTMAP_GM "distmapgm"                                  // example: distmapgmtemporal
#define FILE_DISTMAP_GM "distancemaps/distmap_GM_"                // example: distancemaps/distmap_gm_small_temporal.mnc
#define GM_DEFAULT_POSTFIX "all"                           //use this value if GM dist map file not specified in electrode_name.txt second column - before: "temporal"
//ENTRY POINTS
#define VOL_T1_ENTRYPOINT "entrypoints"                                    // example: entrypoints0
#define FILE_T1_ENTRYPOINT "segmentation/seg_entry_points_mask_"           // example: segmentation/seg_entry_points_mask_AG.mnc
//SKULL NORMALS
#define VECTOR_NORM_SKULL "vecnormskull"                                    // example: vecnormskull0
#define GRALFILE_VECTOR_NORMSKULL "segmentation/vec_entry_points_normal_small_"   // e.g. segmentation/vec_entry_points_normal_small_AG_dx.mnc
//SKULL Default (if using a general entry point for a trajectory)
#define SKULL_DEFAULT_POSTFIX "skull"                           //use this value if GM dist map file not specified in electrode_name.txt second column

//if using FRAME -> use instead of SKULL entry points
#define FRAME_DEFAULT_POSTFIX "frame"                           //use this value if GM dist map file not specified in electrode_name.txt second column


//SKULL
#define VOL_SKULL "skull"
#define FILE_SKULL "segmentation/seg_skull.mnc"
#define VOL_SKULL_MASK "skullmask"
#define FILE_SKULL_MASK "segmentation/seg_skull_mask.mnc"

//EARS (or more specifically: auditory canal of the corresponding hemisphere)
#define VOL_T1_EAR "ear"
#define FILE_T1_EAR "segmentation/seg_ear.mnc"

//MUSCLE
#define VOL_T1_MUSCLE "muscle"
#define FILE_T1_MUSCLE "segmentation/seg_muscle_ear.mnc"

//CSF (particularly important in patient with previous operation)
#define VOL_T1_CSF "csf"
#define FILE_T1_CSF "segmentation/seg_csf.mnc"

//VESSELS
//GADO
#define VOL_GADO_RAW "gado"
#define FILE_GADO_RAW "input/gado.mnc"

#define VOL_GADO_VESSELNESS "vesselness"
#define FILE_GADO_VESSELNESS "gd_vessels/gd_frangi3d_thresh.mnc"

//DIGITAL SUBSTRACTION ANGIOGRAPHY (CTA)
//#define VOL_CTA_RAW "cta"
//#define FILE_CTA_RAW "input/cta.mnc"
//#define FILE_CTA_DENOISED "cta_vessels/cta_th.mnc" //Alt

#define VOL_CTA_RAW_NATIVE "cta"
#define FILE_CTA_RAW_NATIVE "input/cta_reg_to_native.mnc"

#define VOL_CTA_VESSELNESS "vesselness"
#define FILE_CTA_VESSELNESS "segmentation/cta_frangi_native.mnc" // to compute small vessels distance:cta_frangi_native_bin02.mnc // for score claculation segmentation/cta_frangi_native.mnc // previously cta_vessels/cta_frangi3d_thresh01.mnc
//#define VOL_CTA_VESSELNESS_NOTHRESH "vesselness_nothresh"
//#define FILE_CTA_VESSELNESS_NOTHRESH "cta_vessels/cta_frangi3d_rescaled.mnc"
//#define XFM_CTA_TO_REF "registration/cta_to_anat.xfm"

#define VOL_CTA_VESSELNESS_BINARY "binaryvesselness"
#define FILE_CTA_VESSELNESS_BINARY "segmentation/cta_thresh_bin_native.mnc" //originally cta_vessels/cta_frangi3d_thresh05_bin.mnc - NOW thresholded moved to segmentation because is in native

#define VOL_CTA_VESSELNESS_3D "vesselness3D"
#define FILE_CTA_VESSELNESS_3D "segmentation/cta_blur_vis_native.mnc"

//GADO - Binary with higher threshold for HARD constraint
#define VOL_GADO_VESSELNESS_BINARY "binaryvesselness"
#define FILE_GADO_VESSELNESS_BINARY "gd_vessels/gd_frangi3d_thresh05_bin.mnc"

#define OBJ_T1_BRAIN_SURFACE_OBJ "brain_surface_obj"
#define FILE_T1_BRAIN_SURFACE_OBJ "segmentation/brain_surface.obj" //before: outer_both.obj"

// CT
//#define VOL_CT_RAW "ct"
//#define FILE_CT_RAW "input/ct.mnc"
//#define XFM_CT_TO_REF "registration/ct_to_anat.xfm"
#define VOL_CT_RAW_NATIVE "ct"
#define FILE_CT_RAW_NATIVE "input/ct_reg_to_native.mnc"


#define DIM0 "x"
#define DIM1 "y"
#define DIM2 "z"

//PLANS
#define FILE_RESULTS_TRAJ_BEST "allElectrodes"
#define FILE_RESULTS_TRAJ_GRAL "electrode_"
#define FILE_POS_FIX ".csv"
#define FILE_PATIENT_NAMES "patients_tobatchrun.txt"

//CHANNELS
#define FILE_CHANNELS_GRAL "channels_"

#define MAX_SEEG_PLANS 1000 //before: 20 //INCREASE to generalize!
#define MAX_VISIBLE_PLANS 20 // now separate the number of plans in combo (to mark in each patient to the total number (e.g. to visualize in atlas)

/************** PUBLIC TYPE DEFINITION *****************************/


    struct TrajectoryDef {
        seeg::Point3D entryPoint;
        seeg::Point3D targetPoint;
        bool isTargetSet;
        bool isEntrySet;
        std::string name;

        TrajectoryDef() {
            entryPoint[0]=entryPoint[1]=entryPoint[2]=0;
            targetPoint[0]=targetPoint[1]=targetPoint[2]=0;
            isTargetSet = false;
            isEntrySet = false;
            name = "";
        }
    };

    extern TrajectoryDef m_AllPlans[MAX_SEEG_PLANS];

    extern double labelColors[256][3];

    struct DataSetInfo {
        std::string filename;
        std::string name;
    };

    typedef std::map<std::string, DataSetInfo> DataSetInfoMapType;

    struct GroupInfo {
        std::string name;
        std::string transformFile;
        DataSetInfoMapType datasetMap;
    };

    typedef std::map<std::string, GroupInfo> GroupInfoMapType;

/*************** PUBLIC FUNCTIONS *********************************/

    // reset UI
    void ClearAll();

    // add a new volume and set transform files
    void LoadVolume(const std::string& groupName, const std::string& filename, const std::string& name);
    void SetTransform(const std::string& groupName, const std::string& transformFile);
    void GetTransformFile(const std::string& groupName, std::string& transformFile);

    // Open volume/transform
    FloatVolume::Pointer OpenFloatVolume (const std::string& groupName, const std::string& name);
    IntVolume::Pointer OpenIntVolume (const std::string& groupName, const std::string& name);
    ByteVolume::Pointer OpenByteVolume (const std::string& groupName, const std::string& name);
    GeneralTransform::Pointer OpenTransform(const std::string& groupName, bool invert = false);

    // accessors/setters
    void GetMRIGroupList(std::list<std::string>& groupList);
    void GetMRIListForGroup(const std::string& groupName, std::list<std::string>& mriList);
    GroupInfo * GetGroupInfo(const std::string& groupName);
    DataSetInfo * GetDatasetInfo(const std::string& groupName, const std::string& name);
    bool VolumeExists(const std::string& groupName, const std::string& name);

   // void AutoLoadSEEGFromBaseDir(const std::string& basedir);
    void AutoLoadSEEGFromBaseDir(const std::string& basedir, const std::string& hemisphere);

    // Load data
    void OpenFloatVectorVolume (const std::string& groupName, const std::string& gralName, vector<FloatVolume::Pointer> &vecVol);

    // Accessor for the path planner instance and other planning data
//    SEEGPathPlanner::Pointer GetSEEGPathPlanners(int indTarget);

//    SEEGElectrodesCohort::Pointer GetSEEGElectrodesCohort();

//    void SetSEEGPathPlanner(SEEGPathPlanner::Pointer pathPlanner);


}
#endif

