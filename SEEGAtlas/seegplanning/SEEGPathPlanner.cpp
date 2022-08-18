/**
 * @file SEEGPathPlanner.cpp
 *
 * Implementation of the SEEGPathPlanner class for SEEG depth electrode implantation
 *
 * @author Silvain Beriault & Rina Zelmann
 */

// header files to include
#include "itkCastImageFilter.h"
#include "itkLineIterator.h"

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <cstdlib>

#include "SEEGPathPlanner.h"
#include "VolumeTypes.h"
//#include "Histogram.h"
#include "SEEGElectrodeModel.h"
#include "SEEGContactsROIPipeline.h"
#include "SEEGTrajectoryROIPipeline.h"


using namespace std;

namespace seeg {


    // compare two ElectrodeInfo instances based on their aggregated score (this function is
    // passed to the std::list::sort() function)
    static bool compareSEEGtraj (ElectrodeInfo::Pointer first, ElectrodeInfo::Pointer second) {
        return first->m_AggregatedScore < second->m_AggregatedScore;
    }

    static string m_TestToCompare;

    // compare two ElectrodeInfo instance based on the sum criteria of a specific test (m_TestToCompare)
    static bool compareSEEGSum (ElectrodeInfo::Pointer first, ElectrodeInfo::Pointer second) {
        return first->m_TrajectoryTestScores[m_TestToCompare].scoreSum <
                    second->m_TrajectoryTestScores[m_TestToCompare].scoreSum;
    }

    // compare two ElectrodeInfo instance based on the max criteria of a specific test (m_TestToCompare)
    static bool compareSEEGMax (ElectrodeInfo::Pointer first, ElectrodeInfo::Pointer second) {
        return first->m_TrajectoryTestScores[m_TestToCompare].scoreMax <
                    second->m_TrajectoryTestScores[m_TestToCompare].scoreMax;
    }

    // compare two ElectrodeInfo instance based on agreggated Reward rankings
    static bool compareAgreggatedReward (ElectrodeInfo::Pointer first, ElectrodeInfo::Pointer second) {
        return first->m_AggregatedRewardScore >
                    second->m_AggregatedRewardScore;
    }

    /**** CONSTRUCTOR/DESTRUCTOR ****/

    SEEGPathPlanner::SEEGPathPlanner() {
        // nothing to do
    }

    SEEGPathPlanner::~SEEGPathPlanner() {
        // nothing to do
    }


    /**** ACCESSORS/SETTERS ****/


    Point3D SEEGPathPlanner::GetEntryPoint() {
        return this->m_EntryPoint;
    }

    void SEEGPathPlanner::addManualPlans( const Point3D& entryPoint, const Point3D& targetPoint, const SEEGElectrodeModel::SEEG_ELECTRODE_MODEL_TYPE electrodeType) {
        ElectrodeInfo::Pointer ep = ElectrodeInfo::New(entryPoint, targetPoint, electrodeType);
        m_AllTrajectories.push_back(ep);
        m_ActiveTrajectories.push_back(ep);
    }

    void SEEGPathPlanner::SetTrajectoryRiskTestWeights(const string& testName,
                                               float weightUsingMax,
                                               float weightUsingSum) {
        SetTrajectoryRiskTestWeights(testName, weightUsingMax, weightUsingSum, -1); //-1 means the hard limit was not enabled
    }

    // also set hard limits
    void SEEGPathPlanner::SetTrajectoryRiskTestWeights(const string& testName,
                                               float weightUsingMax,
                                               float weightUsingSum,
                                               float hardLimitValue) {
        this->m_TrajectoryRiskTestWeights[testName].m_WeightUsingMax = weightUsingMax;
        this->m_TrajectoryRiskTestWeights[testName].m_WeightUsingSum = weightUsingSum;
        this->m_TrajectoryRiskTestWeights[testName].m_HardLimit = hardLimitValue;
    }

    void SEEGPathPlanner::SetTrajectoryRewardTestWeights(const string& testName,
                                               float weightUsingMax,
                                               float weightUsingSum) {
        this->m_TrajectoryRewardTestWeights[testName].m_WeightUsingMax = weightUsingMax;
        this->m_TrajectoryRewardTestWeights[testName].m_WeightUsingSum = weightUsingSum;
    }

    map<string, TrajectoryRiskTestWeights>& SEEGPathPlanner::GetTrajectoryRiskTestWeights() {
        return m_TrajectoryRiskTestWeights;
    }

    map<string, TrajectoryTestWeights>& SEEGPathPlanner::GetTrajectoryRewardTestWeights() {
        return m_TrajectoryRewardTestWeights;
    }

    void SEEGPathPlanner::DeleteTrajectoryTest(const string& testName) {
        this->m_TrajectoryRiskTestWeights.erase(testName);
        this->m_TrajectoryRewardTestWeights.erase(testName);
        list<ElectrodeInfo::Pointer>::iterator it;
        for (it = m_AllTrajectories.begin(); it != m_AllTrajectories.end(); it++) {
            ElectrodeInfo::Pointer electrodePoints = *it;
            electrodePoints->m_TrajectoryTestScores.erase(testName);
        }
    }

    void SEEGPathPlanner::SetTrajectoryGlobalWeights(float weightRisk, float weightReward) {
        m_WeightReward = weightReward;
        m_WeightRisk = weightRisk;
    }

    float SEEGPathPlanner::GetGlobalRewardWeight() {
        return m_WeightReward;
    }

    float SEEGPathPlanner::GetGlobalRiskWeight() {
        return m_WeightRisk;
    }

    string SEEGPathPlanner::GetElectrodeName(){
        return m_ElectrodeName;
    }

    void SEEGPathPlanner::SetElectrodeName(const string electrodeName){
        m_ElectrodeName = electrodeName;
    }

    int SEEGPathPlanner::GetNumberOfActiveTrajectories(){
        return this->GetActiveTrajectories().size();
    }


    /**** PUBLIC FUNCTIONS ****/

    void SEEGPathPlanner::Reset() {
        m_AllTrajectories.clear();
        m_ActiveTrajectories.clear();
        m_TrajectoryRiskTestWeights.clear();
        m_TrajectoryRewardTestWeights.clear();
        m_ElectrodeName.clear();
    }


    map<string, TrajectoryRiskTestWeights>& SEEGPathPlanner::GetRiskWeights() {
        return m_TrajectoryRiskTestWeights;
    }

    map<string, TrajectoryTestWeights>& SEEGPathPlanner::GetRewardWeights() {
        return m_TrajectoryRewardTestWeights;
    }

//RIZ -- see how must be changed!
    ElectrodeInfo::Pointer SEEGPathPlanner::FindElectrodeInfo(Point3D targetPoint, Point3D entryPoint, float &sumDistBetweenTraj, GeneralTransform::Pointer nativeToRef) {
        float minDistance = 1000;
        ElectrodeInfo::Pointer closest_match_point;

/*   RIZ: Stay always in native space
     Point3D point_ref(point_native);
        if (nativeToRef) {
            nativeToRef->TransformPoint(point_native, point_ref); // go to ref space
        }
*/
        Vector3D_lf electrodePoints (entryPoint[0] - targetPoint[0],
                                     entryPoint[1] - targetPoint[1],
                                     entryPoint[2] - targetPoint[2]);


        list<ElectrodeInfo::Pointer>::iterator it;

        it = m_AllTrajectories.begin();
        while( it != m_AllTrajectories.end() ) {
            ElectrodeInfo::Pointer electrode = *it;
         //   float angle = this->CalcAngleBetweenTrajectories(electrodePoints, electrode->m_ElectrodeVectorWorld);
               float sumDistance = this->CalcDistanceBetweenPointsInTrajectories(electrodePoints, electrode->m_ElectrodeVectorWorld);
            if (abs(sumDistance) < abs(minDistance) ) {
                minDistance = sumDistance;
                closest_match_point = electrode;
            }
            it++;
        }

        sumDistBetweenTraj = minDistance;
        return closest_match_point;
    }


	// this is really stupid... should merge the two FindEntryPointInfo functions...
    ElectrodeInfo::Pointer SEEGPathPlanner::FindActiveElectrodeInfo(Point3D targetPoint, Point3D entryPoint, float& sumDistBetweenTraj, GeneralTransform::Pointer nativeToRef) {

        float min_SumDistance = 1000;
        ElectrodeInfo::Pointer closest_match_point;

/*   RIZ: Stay always in native space
     Point3D point_ref(point_native);
        if (nativeToRef) {
            nativeToRef->TransformPoint(point_native, point_ref); // go to ref space
        }
*/
        Vector3D_lf electrodePoints (entryPoint[0] - targetPoint[0],
                                     entryPoint[1] - targetPoint[1],
                                     entryPoint[2] - targetPoint[2]);


        list<ElectrodeInfo::Pointer>::iterator it;

        it = m_ActiveTrajectories.begin();
        while( it != m_ActiveTrajectories.end() ) {
            ElectrodeInfo::Pointer electrode = *it;
         //   float angle = this->CalcAngleBetweenTrajectories(electrodePoints, electrode->m_ElectrodeVectorWorld);
               float sumDistance = this->CalcDistanceBetweenPointsInTrajectories(electrodePoints, electrode->m_ElectrodeVectorWorld);
            if (abs(sumDistance) < abs(min_SumDistance) ) {
                min_SumDistance = sumDistance;
                closest_match_point = electrode;
            }
            it++;
        }

        sumDistBetweenTraj = min_SumDistance;
        return closest_match_point;
    }


    void SEEGPathPlanner::InitializeTrajectoriesFromVols(IntVolume::Pointer entryPointMaskVol, IntVolume::Pointer targetMaskVol) {

        // clear current list of entry points
        this->m_AllTrajectories.clear();
        this->m_ActiveTrajectories.clear();

        // iterate through entryPoints and Target Volumes to build a list of possible Trajectory points

        IntVolumeRegionIteratorWithIndex it (entryPointMaskVol, entryPointMaskVol->GetLargestPossibleRegion());
        IntVolumeRegionIteratorWithIndex it2 (targetMaskVol, targetMaskVol->GetLargestPossibleRegion());
        for (it.GoToBegin(); !it.IsAtEnd(); ++it) {
            IntVolume::IndexType voxelIndex = it.GetIndex();
            if (it.Get()) {
                Point3D worldEntryPoint;
                entryPointMaskVol->TransformIndexToPhysicalPoint(voxelIndex, worldEntryPoint);
                for (it2.GoToBegin(); !it2.IsAtEnd(); ++it2) {
                    IntVolume::IndexType voxelIndex = it.GetIndex();
                    if (it2.Get()) {
                        Point3D worldTargetPoint;
                        entryPointMaskVol->TransformIndexToPhysicalPoint(voxelIndex, worldTargetPoint);

                        ElectrodeInfo::Pointer info = ElectrodeInfo::New(worldEntryPoint, worldTargetPoint);
                        info->m_AggregatedRiskScore = -1;
                        info->m_AggregatedRewardScore = -1;
                        info->m_AggregatedScore = -1;
                        m_AllTrajectories.push_back(info);
                        m_ActiveTrajectories.push_back(info);
                    }
                }
            }
        }
    }

    void SEEGPathPlanner::InitializeTrajectoriesFromFile (const string& filename,  const SEEGElectrodeModel::SEEG_ELECTRODE_MODEL_TYPE electrodeType) {
        // clear current list of entry points
        this->m_AllTrajectories.clear();
        this->m_ActiveTrajectories.clear();

        // read file with all points
        ifstream fileAllTraj (filename.c_str(), ios::in|ios::binary); //RIZ: check!
        string line, token;

        if (fileAllTraj.is_open())  {
            while ( fileAllTraj.good() ) {
                getline (fileAllTraj,line);
                if (line.length()>0){
                    stringstream ss (line);

                    Point3D targetPoints;
                    getline(ss, token, ',');
                    targetPoints[0] = atof(token.c_str()); //first 3 points are the target
                    getline(ss, token, ',');
                    targetPoints[1] = atof(token.c_str()); //first 3 points are the target
                    getline(ss, token, ',');
                    targetPoints[2] = atof(token.c_str()); //first 3 points are the target
                    Point3D entryPoints;
                    getline(ss, token, ',');
                    entryPoints[0] = atof(token.c_str()); // last 3 points are the entry points
                    getline(ss, token, ',');
                    entryPoints[1] = atof(token.c_str()); // last 3 points are the entry points
                    getline(ss, token, ',');
                    entryPoints[2] = atof(token.c_str()); // last 3 points are the entry points
                    ElectrodeInfo::Pointer info = ElectrodeInfo::New(entryPoints, targetPoints);
                    info->m_AggregatedRiskScore = -1;
                    info->m_AggregatedRewardScore = -1;
                    info->m_AggregatedScore = -1;
                    info->m_ElectrodeType = electrodeType;
                    m_AllTrajectories.push_back(info);
                    m_ActiveTrajectories.push_back(info);
                }
            }
            fileAllTraj.close();
        }
        else cout << "Unable to open file "<<filename.c_str()<<endl;
    }

    void SEEGPathPlanner::DoSEEGMultiTest ( vector<string>& binTestNames,
                                    vector<IntVolume::Pointer>& binVols,
                                    vector<BinaryTestCfg>& binTestCfgs,

                                    vector<string>& fuzzyTestNames,
                                    vector<FloatVolume::Pointer>& fuzzyVols,
                                    vector<FuzzyTestCfg>& fuzzyTestCfgs,

                                    map<string, float>& extraLengthCfgs,

                                    GeneralTransform::Pointer nativeToRef) {

        DoSEEGMultiTest(    binTestNames,
                        binVols,
                        binTestCfgs,
                        fuzzyTestNames,
                        fuzzyVols,
                        fuzzyTestCfgs,
                        extraLengthCfgs,
                        m_ActiveTrajectories.begin(),
                        m_ActiveTrajectories.end(),
                        nativeToRef);
    }

    void SEEGPathPlanner::DoSEEGMultiTest (  vector<string>& binTestNames,
                                     vector<IntVolume::Pointer>& binVols,
                                     vector<BinaryTestCfg>& binTestCfgs,

                                     vector<string>& fuzzyTestNames,
                                     vector<FloatVolume::Pointer>& fuzzyVols,
                                     vector<FuzzyTestCfg>& fuzzyTestCfgs,

                                     map<string, float>& extraLengthCfgs,

                                     list<ElectrodeInfo::Pointer>::iterator first,
                                     list<ElectrodeInfo::Pointer>::iterator last,
                                     GeneralTransform::Pointer nativeToRef) {

        bool reject;
       // cout<<"inside DoMultiTest"<<endl;
    /*    Point3D targetDestination_native(m_TargetDestination);
        if (nativeToRef) {
            nativeToRef->TransformPointInv(m_TargetDestination, targetDestination_native); // go from ref to native space
        }
*/
        SEEGTrajectoryROIPipeline::Pointer pipeline;
        // one of the 2 vectors must contain a volume
        if (binVols.size()>0) {
            pipeline = SEEGTrajectoryROIPipeline::New(binVols[0]);
        } else {
            pipeline = SEEGTrajectoryROIPipeline::New(fuzzyVols[0]);
        }


        list<ElectrodeInfo::Pointer>::iterator it;

        it = first;
        do {
            reject = false;
            ElectrodeInfo::Pointer electrode = *it;
            Point3D entryPoint_native(electrode->m_EntryPointWorld);
            Point3D targetDestination_native(electrode->m_TargetPointWorld);

            /*if (nativeToRef) {
                nativeToRef->TransformPointInv(electrode->m_EntryPointWorld, entryPoint_native); // go from ref to native space
                nativeToRef->TransformPointInv(electrode->m_TargetPointWorld, targetDestination_native); // go from ref to native space
            }*/
            Point3D entryPoint_extrapolated;

            for (int i=0; i<binTestNames.size() && !reject; i++) {
                this->ExtrapolateEntryPoint(targetDestination_native, entryPoint_native, entryPoint_extrapolated, extraLengthCfgs[binTestNames[i]]); //extrapolate by 50mm to consider ears
                pipeline->CalcDistanceMap(entryPoint_extrapolated,targetDestination_native, binTestCfgs[i].m_MaxDistToEvaluate);

                TrajectoryTestScore& testScore = electrode->m_TrajectoryTestScores[binTestNames[i]];

                reject = TestBinaryOverlap (
                                   pipeline->GetLastDistanceMap(),
                                   binVols[i],
                                   binTestCfgs[i],
                                   testScore,
                                   nativeToRef);
            }

            for (int i=0; i<fuzzyTestNames.size() && !reject; i++) {
                this->ExtrapolateEntryPoint(targetDestination_native, entryPoint_native, entryPoint_extrapolated, extraLengthCfgs[fuzzyTestNames[i]]); //extrapolate by 50mm to consider ears
                pipeline->CalcDistanceMap(entryPoint_extrapolated,targetDestination_native, fuzzyTestCfgs[i].m_MaxDistToEvaluate);

                TrajectoryTestScore& testScore = electrode->m_TrajectoryTestScores[fuzzyTestNames[i]];

                TestFuzzyOverlap (
                                   pipeline->GetLastDistanceMap(),
                                   fuzzyVols[i],
                                   fuzzyTestCfgs[i],
                                   testScore,
                                   nativeToRef);
            }


            if (reject) {
                electrode->m_Valid = false;
                electrode->m_AggregatedRiskScore = -1;
                electrode->m_AggregatedRewardScore = -1;
                electrode->m_AggregatedScore = -1;
            } else {
                electrode->m_Valid = true;
            }
            it++;
        } while (it != last);
    }

    void SEEGPathPlanner::RemoveInvalidPaths() {
        list<ElectrodeInfo::Pointer>::iterator it;
        for (it = m_ActiveTrajectories.begin(); it != m_ActiveTrajectories.end(); ) {
            ElectrodeInfo::Pointer electrode = *it;
            if (!electrode->m_Valid) {
                it = m_ActiveTrajectories.erase(it);
            } else {
                it++;
            }
        }
    }


/*    void SEEGPathPlanner::DoBinaryTest ( const string& testName,
                                     IntVolume::Pointer binaryVol,
                                     const BinaryTestCfg& cfgs,
                                     GeneralTransform::Pointer nativeToRef) {


        DoBinaryTest(   testName,
                        binaryVol,
                        cfgs,
                        m_ActiveTrajectories.begin(),
                        m_ActiveTrajectories.end(),
                        nativeToRef);
    }


    void SEEGPathPlanner::DoBinaryTest (    const string& testName,
                                        IntVolume::Pointer binaryVol,
                                        const BinaryTestCfg& cfgs,
                                        list<ElectrodeInfo::Pointer>::iterator first,
                                        list<ElectrodeInfo::Pointer>::iterator last,
                                        GeneralTransform::Pointer nativeToRef) {


        SEEGTrajectoryROIPipeline::Pointer pipeline = SEEGTrajectoryROIPipeline::New(binaryVol);

        list<ElectrodeInfo::Pointer>::iterator it;


        for (it = first; it != last; it++) {
            ElectrodeInfo::Pointer electrode = *it;
            Point3D entryPoint_native(electrode->m_EntryPointWorld);
            Point3D targetDestination_native(electrode->m_TargetPointWorld);
            if (nativeToRef) {
                nativeToRef->TransformPointInv(electrode->m_EntryPointWorld, entryPoint_native); // go from ref to native space
                nativeToRef->TransformPointInv(electrode->m_TargetPointWorld, targetDestination_native); // go from ref to native space
            }

            Point3D entryPoint_extrapolated;
            this->ExtrapolateEntryPoint(targetDestination_native, entryPoint_native, entryPoint_extrapolated, 50.0); //extrapolate by 50mm to consider ears

            TrajectoryTestScore& testScore = electrode->m_TrajectoryTestScores[testName];

            Point3D targetPoint(electrode->m_TargetPointWorld);

            pipeline->CalcDistanceMap(entryPoint_extrapolated, targetPoint, cfgs.m_MaxDistToEvaluate);
            bool reject = TestBinaryOverlap (
                               pipeline->GetLastDistanceMap(),
                               binaryVol,
                               cfgs,
                               testScore,
                               nativeToRef);


            if (reject) {
                electrode->m_AggregatedRiskScore = -1;
                electrode->m_AggregatedRewardScore = -1;
                electrode->m_AggregatedScore = -1;
                electrode->m_Valid = false;
            }
        }
    }
*/

/*    void SEEGPathPlanner::DoFuzzyTest(  const string& testName,
                                    FloatVolume::Pointer fuzzyVol,
                                    const FuzzyTestCfg& cfgs,
                                    GeneralTransform::Pointer nativeToRef) {

        DoFuzzyTest(    testName,
                        fuzzyVol,
                        cfgs,
                        m_ActiveTrajectories.begin(),
                        m_ActiveTrajectories.end(),
                        nativeToRef);
    }

    void SEEGPathPlanner::DoFuzzyTest(  const string& testName,
                                    FloatVolume::Pointer fuzzyVol,
                                    const FuzzyTestCfg& cfgs,
                                    list<ElectrodeInfo::Pointer>::iterator first,
                                    list<ElectrodeInfo::Pointer>::iterator last,
                                    GeneralTransform::Pointer nativeToRef) {

        Point3D targetDestination_native(m_TargetDestination);
        if (nativeToRef) {
            nativeToRef->TransformPointInv(m_TargetDestination, targetDestination_native); // go from ref to native
        }

        SEEGTrajectoryROIPipeline::Pointer pipeline = SEEGTrajectoryROIPipeline::New(fuzzyVol);

        list<ElectrodeInfo::Pointer>::iterator it;

        for (it = first; it != last; it++) {

            ElectrodeInfo::Pointer electrode = *it;
            Point3D entryPoint_native(electrode->m_EntryPointWorld);
            Point3D targetDestination_native(electrode->m_TargetPointWorld);
            if (nativeToRef) {
                nativeToRef->TransformPointInv(electrode->m_EntryPointWorld, entryPoint_native); // go from ref_to_native
                nativeToRef->TransformPointInv(electrode->m_TargetPointWorld, targetDestination_native); // go from ref_to_native
            }

            Point3D entryPoint_extrapolated;
            this->ExtrapolateEntryPoint(targetDestination_native, entryPoint_native, entryPoint_extrapolated, 50.0); //extrapolate by 50mm to consider ears

            TrajectoryTestScore& testScore = electrode->m_TrajectoryTestScores[testName];

            pipeline->CalcDistanceMap(entryPoint_extrapolated, targetDestination_native, cfgs.m_MaxDistToEvaluate);

            TestFuzzyOverlap(
                                pipeline->GetLastDistanceMap(),
                                fuzzyVol,
                                cfgs,
                                testScore,
                                nativeToRef);
        }
    }
*/


   void SEEGPathPlanner::DoMaximizationTest(  const string& testName,
                                   FloatVolume::Pointer targetDistMap,
                                   const MaximizationTestCfg& cfgs,
                                   GeneralTransform::Pointer nativeToRef) {

       DoMaximizationTest( testName,
                           targetDistMap,
                           cfgs,
                           m_ActiveTrajectories.begin(),
                           m_ActiveTrajectories.end(),
                           nativeToRef);
   }

   void SEEGPathPlanner::DoMaximizationTest(const string& testName,
                                   FloatVolume::Pointer targetDistMap,
                                   const MaximizationTestCfg& cfgs,
                                   list<ElectrodeInfo::Pointer>::iterator first,
                                   list<ElectrodeInfo::Pointer>::iterator last,
                                   GeneralTransform::Pointer nativeToRef) { //RIZ: OBSOLETE! - only considers 1 distMap

       FloatVolumeDuplicator::Pointer volDuplicator = FloatVolumeDuplicator::New();
       volDuplicator->SetInputImage(targetDistMap);
       volDuplicator->Update();
       FloatVolume::Pointer vol = volDuplicator->GetOutput();
       vol->DisconnectPipeline();
       ElectrodeInfo::Pointer electrode = *first;
       SEEGContactsROIPipeline::Pointer pipelineContacts = SEEGContactsROIPipeline::New(vol, electrode->GetElectrodeModelType()); //volume is only to have size,
       pipelineContacts->EmptyTemplateVol();
       FloatVolume::SpacingType spacing = vol->GetSpacing();
       float maxSpacing = max(spacing[0], max(spacing[1], spacing[2]));
       FloatVolume::RegionType region = targetDistMap->GetRequestedRegion();

       list<ElectrodeInfo::Pointer>::iterator it;
       for (it = first; it != last; it++) {

           ElectrodeInfo::Pointer electrode = *it;
           ElectrodeInfo::Pointer currElectInfo = ElectrodeInfo::New(electrode->m_EntryPointWorld, electrode->m_TargetPointWorld); //the one I will modify

           Point3D entryPoint(electrode->m_EntryPointWorld);
           Point3D targetPoint(electrode->m_TargetPointWorld);
 //          if (nativeToRef) {
 //             nativeToRef->TransformPointInv(electrode->m_EntryPointWorld, entryPoint_native); // go from ref_to_native
 //              nativeToRef->TransformPointInv(electrode->m_TargetPointWorld, targetDestination_native); // go from ref_to_native
 //          }

           Point3D targetPoint_extrapolated;
           float currLength = CalcLineLength(targetPoint, entryPoint); // for the first point
           this->ExtrapolateTargetPoint(targetPoint, entryPoint, targetPoint_extrapolated, cfgs.m_MaxDistToEvaluate - currLength); //max length is electrode length -> extrapolate target by m_MaxDistToEvaluate - current length
           //cout << "Analysing electrode: " << "TP="<<targetPoint<< " - EP="<<entryPoint<<endl;
           float vecSizeOrig=1; // The first one should always be analysed - either is within the surface of the target or is a true electrode that might be outside
           if (cfgs.m_NamePreviousTest!=testName) vecSizeOrig=electrode->m_VecTrajectoryTestScores[cfgs.m_NamePreviousTest].size();
           float vecSize=0;
           ElectrodeInfo::VecTrajectoryTestScore vecScores;
           //Consider all target points within volume
           Point3D currTargetPoint = targetPoint;                                           // for the first point
           float maxLength;
           if (cfgs.m_AnalizeMultipleDepths){
               maxLength = CalcLineLength(targetPoint_extrapolated, entryPoint);
           } else {
               maxLength =currLength; // to onl;y do it once!
           }

           FloatVolume::IndexType targetPointIndex;
           targetDistMap->TransformPhysicalPointToIndex(currTargetPoint, targetPointIndex); // for the first point
           SEEGElectrodeModel::Pointer electrodeModel = SEEGElectrodeModel::New(electrode->GetElectrodeModelType()); // all electrodes are assumed to be of the same type! if not -> bring this line inside loop

           while (maxLength-currLength >= 0 && region.IsInside(targetPointIndex)==true){
               currElectInfo->m_TargetPointWorld = currTargetPoint;

               if ((targetDistMap->GetPixel(targetPointIndex)>0 && vecSizeOrig==1) || vecSize<vecSizeOrig) { //left side is for target - right side is for GM or first position
                   // Get contact positions (RIZ: for now only to record how many are inside - later to ONLY use those points!)
                   vector<Point3D> allContactPoints;
                   electrodeModel->CalcAllContactPositions(currTargetPoint, entryPoint, allContactPoints);
                   FloatVolume::Pointer recordedDistMap;
                   pipelineContacts->CalcRecordingMap(targetDistMap, recordedDistMap, currElectInfo);
                   //cout << "rec dis map " <<recordedDistMap->GetPixel(targetPointIndex) << "target dist map" << targetDistMap->GetPixel(targetPointIndex)<<endl;
                   TrajectoryTestScore& testVolScore = electrode->m_TrajectoryTestScores[testName];

                   TestMaximizationOverlap(
                               recordedDistMap,
                               allContactPoints,
                               cfgs,
                               testVolScore,
                               nativeToRef);

                   vecScores.push_back(testVolScore);
                   //electrode->m_VecTrajectoryTestScores.push_back(electrode->m_TrajectoryTestScores);
                   vecSize++;
               }
               Point3D newTargetPt;
               this->ExtrapolateTargetPoint(currTargetPoint, entryPoint, newTargetPt, maxSpacing); //extrapolate target by up to length of electrode
               currTargetPoint = newTargetPt;
               currLength = CalcLineLength(currTargetPoint, entryPoint);
               targetDistMap->TransformPhysicalPointToIndex(currTargetPoint, targetPointIndex);
           }
           electrode->m_VecTrajectoryTestScores[testName] = vecScores;
       }
   }

   void SEEGPathPlanner::DoMaximizationTest(vector<string> &testNames,
                                   const vector<FloatVolume::Pointer> &targetDistMaps,
                                   const MaximizationTestCfg& cfgs,
                                   vector<string>& binTestNames,
                                   vector<IntVolume::Pointer>& binVols,
                                   vector<BinaryTestCfg>& binTestCfgs,
                                   list<ElectrodeInfo::Pointer>::iterator first,
                                   list<ElectrodeInfo::Pointer>::iterator last,
                                   GeneralTransform::Pointer nativeToRef) {

       // Creates empty contacts pipeline -> useful to compute dist map around line
       FloatVolumeDuplicator::Pointer volDuplicator = FloatVolumeDuplicator::New();
       volDuplicator->SetInputImage(targetDistMaps[0]);
       volDuplicator->Update();
       FloatVolume::Pointer vol = volDuplicator->GetOutput();
       vol->DisconnectPipeline();
       ElectrodeInfo::Pointer electrode = *first;
       SEEGContactsROIPipeline::Pointer pipelineContacts = SEEGContactsROIPipeline::New(vol, electrode->GetElectrodeModelType()); //volume is only to have size
       pipelineContacts->EmptyTemplateVol();

       // Create pipeline to check if new target points ought to be rejected or not
       vector<SEEGTrajectoryROIPipeline::Pointer> pipelineTrajectories;
       if (binVols.size()>0) {
           for (int i=0; i< binVols.size();i++){
               SEEGTrajectoryROIPipeline::Pointer pipelineTrajectory;
               pipelineTrajectory = SEEGTrajectoryROIPipeline::New(binVols[i]); //to have for each the appropriate step/origin/size
               pipelineTrajectories.push_back(pipelineTrajectory);
           }
       }

       // Get info from target volume
       FloatVolume::SpacingType spacing = vol->GetSpacing();
       float maxSpacing = max(spacing[0], max(spacing[1], spacing[2]));
       FloatVolume::RegionType region = targetDistMaps[0]->GetRequestedRegion(); //assuming that all have same region
       electrode = *first;
       SEEGElectrodeModel::Pointer electrodeModel = SEEGElectrodeModel::New(electrode->GetElectrodeModelType()); // all electrodes are assumed to be of the same type! if not -> bring this line inside loop

       // Integrates recording within target and GM
       list<ElectrodeInfo::Pointer>::iterator it;
       for (it = first; it != last; it++) {  // Analyse each electrode
           electrode = *it;
           for (int iTarget=0; iTarget<targetDistMaps.size();iTarget++){ //initialize vector of tesValues
               electrode->m_VecTrajectoryTestScores[testNames[iTarget]].clear();
           }
           ElectrodeInfo::Pointer currElectInfo = ElectrodeInfo::New(electrode->m_EntryPointWorld, electrode->m_TargetPointWorld,  electrode->GetElectrodeModelType()); //the one I will modify
           Point3D entryPoint(electrode->m_EntryPointWorld);
           Point3D targetPoint(electrode->m_TargetPointWorld);
           float currLength = CalcLineLength(targetPoint, entryPoint);                  // for the first point
            //cout << "Analysing electrode: " << "TP="<<targetPoint<< " - EP="<<entryPoint<<endl;

           //Consider all target points within volume  if m_AnalizeMultipleDepths is true
           vector<Point3D> targetPtsInTarget, allPointsPerElectrode;
           if (cfgs.m_AnalizeMultipleDepths == true){
               Point3D targetPointExtrapolated;
               float lenExtra = cfgs.m_MaxDistToEvaluate - currLength;
               this->ExtrapolateTargetPoint(targetPoint, entryPoint, targetPointExtrapolated, lenExtra);
               // find all points in electrode
               electrodeModel->CalcAllLinePositions(maxSpacing, targetPointExtrapolated, entryPoint, allPointsPerElectrode);

           } else {
               allPointsPerElectrode.push_back(targetPoint); // to only do it once!
           }

//           WriteFloatVolume("/Users/rzelmann/test/distMap1.mnc", targetDistMaps[0]); //RIZ to test!!

           //while (maxLength-currLength >= 0 && region.IsInside(targetPointIndex)==true){
           int nPtsInTarget=0;
           while (allPointsPerElectrode.size()>0) {
               FloatVolume::IndexType targetPointIndex;
               Point3D currTargetPoint = allPointsPerElectrode.back();
               targetDistMaps[0]->TransformPhysicalPointToIndex(currTargetPoint, targetPointIndex); // for the first point -- assumes that all vols have same image size, origin,etc
               allPointsPerElectrode.pop_back();
               currElectInfo->m_TargetPointWorld = currTargetPoint;
               if (targetDistMaps[0]->GetPixel(targetPointIndex)>0 || cfgs.m_AnalizeMultipleDepths==false) { //Either we analize only 1 point or we have to be inside the target
                   // Check that new target point does not have to be rejected
                   bool reject=false;
                   if (cfgs.m_AnalizeMultipleDepths==true) {  //only for multiple trajectories - for single it was discarded before
                       for (int i=0; i<binTestNames.size() && !reject; i++) {
                           pipelineTrajectories[i]->CalcDistanceMap(entryPoint, currTargetPoint, binTestCfgs[i].m_MaxDistToEvaluate);
                           TrajectoryTestScore& testScore = currElectInfo->m_TrajectoryTestScores[binTestNames[i]];
                           reject = TestBinaryOverlap (
                                       pipelineTrajectories[i]->GetLastDistanceMap(),
                                       binVols[i],
                                       binTestCfgs[i],
                                       testScore,
                                       nativeToRef);
                       }
                   }
                   if (!reject) {
                       // Get contact positions
                       vector<Point3D> allContactPoints;
                       electrodeModel->CalcAllContactPositions(currTargetPoint, entryPoint, allContactPoints);
                       vector<FloatVolume::Pointer> recordedDistMaps;
                       for (int iTarget=0;iTarget<targetDistMaps.size(); iTarget++) {
                           FloatVolume::Pointer recordedDistMap;
                           int nContactsToAnalyse =0;
                           if (cfgs.m_OnlyInsideContacts[iTarget]==true) {
                               //Find contacts within volume if only inside contacts are considered (usually for target not for NC)
                               vector<Point3D> tempContactPoints = allContactPoints;
                               Point3D newTargetPoint;
                               //newTargetPoint[0]=newTargetPoint[1]=newTargetPoint[2]=0;
                               Point3D newEntryPoint;
                               //newEntryPoint[0]=newEntryPoint[1]=newEntryPoint[2]=0;
                               while (!tempContactPoints.empty()) {
                                   Point3D contactPoint = tempContactPoints.back();
                                   FloatVolume::IndexType contactPointIndex;
                                   targetDistMaps[iTarget]->TransformPhysicalPointToIndex(contactPoint, contactPointIndex);
                                   if (targetDistMaps[iTarget]->GetPixel(contactPointIndex)>0) {
                                       //keep point closest to entry point as newEntryPoint
                                       if (nContactsToAnalyse<=0) { //assign first contact since we are looking at the vector from last to first point
                                           newEntryPoint = contactPoint;
                                       }
                                       //keep point closest to target (closest to the tip) as newTarget
                                       newTargetPoint = contactPoint; // keep assiging contacts to keep the last one inside (assuming it will be the closest to the target point because is the first in the vector)
                                       nContactsToAnalyse++;
                                   }
                                   tempContactPoints.pop_back();
                               }
                               currElectInfo->m_TargetPointWorld = newTargetPoint; //first contact inside target
                               currElectInfo->m_EntryPointWorld = newEntryPoint;   // last contact inside target
                           } else {
                               currElectInfo->m_TargetPointWorld = currTargetPoint;  //reset to full electrode
                               currElectInfo->m_EntryPointWorld = electrode->m_EntryPointWorld;
                               nContactsToAnalyse = allContactPoints.size();
                           }
                           // Compute Distance map for each target (beacuse the electrode might be different for each target)
                           if (nContactsToAnalyse >0){
                               pipelineContacts->CalcRecordingMap(targetDistMaps[iTarget], recordedDistMap, currElectInfo);
                           }
                           else {
                               recordedDistMap = FloatVolume::Pointer();
                           }
                           recordedDistMaps.push_back(recordedDistMap);
                       }

                       //targetDistMaps[0] MUST be the target - all other volumes are secondary (e.g. temporal GM)
                     //  vector<FloatVolume::Pointer> recordedDistMaps;
                     //  pipelineContacts->CalcRecordingMap(targetDistMaps, recordedDistMaps, currElectInfo);
                       //cout << "rec dis map " <<recordedDistMap->GetPixel(targetPointIndex) << "target dist map" << targetDistMap->GetPixel(targetPointIndex)<<endl;

                       for (int iTarget=0; iTarget<targetDistMaps.size();iTarget++){
                           TrajectoryTestScore& testVolScore = electrode->m_TrajectoryTestScores[testNames[iTarget]];
                           FloatVolume::Pointer recordedDistMap = recordedDistMaps[iTarget];
                           // WriteFloatVolume("/home/rina/test/recMap.mnc", recordedDistMap);

                           TestMaximizationOverlap(
                                       recordedDistMap,
                                       allContactPoints,
                                       cfgs,
                                       testVolScore,
                                       nativeToRef);

                           // vecScores.push_back(testVolScore);
                           electrode->m_VecTrajectoryTestScores[testNames[iTarget]].push_back(testVolScore);
                       }
                       ++nPtsInTarget;
                   }
               }
           }
           // Point3D newTargetPt;
             //  this->ExtrapolateTargetPoint(currTargetPoint, entryPoint, newTargetPt, maxSpacing); //extrapolate target by 50mm
             //  currTargetPoint = newTargetPt;
             //  currLength = CalcLineLength(currTargetPoint, entryPoint);
             //  targetDistMaps[0]->TransformPhysicalPointToIndex(currTargetPoint, targetPointIndex);
           //}
          // if (region.IsInside(targetPointIndex)==false){
          //     cout << "Electrode: " << "TP="<<targetPoint<< " - current TP="<<currTargetPoint<<" Target outside of region "<<endl;
          // }
         //  electrode->m_VecTrajectoryTestScores[testName] = vecScores;
       //    cout << "Electrode: " << " with TP="<<targetPoint<< " - #points in target="<<nPtsInTarget<<endl;
       }
   }


   void SEEGPathPlanner::DoVectorTest(  const string& testName,
                                   vector<FloatVolume::Pointer> &vectorVol,
                                   const BinaryTestCfg &cfgs,
                                   GeneralTransform::Pointer nativeToRef) {

       DoVectorTest(   testName,
                       vectorVol,
                       cfgs,
                       m_ActiveTrajectories.begin(),
                       m_ActiveTrajectories.end(),
                       nativeToRef);
   }

   void SEEGPathPlanner::DoVectorTest(  const string& testName,
                                   vector<FloatVolume::Pointer> &vectorVol,
                                   const BinaryTestCfg& cfgs,
                                   list<ElectrodeInfo::Pointer>::iterator first,
                                   list<ElectrodeInfo::Pointer>::iterator last,
                                   GeneralTransform::Pointer nativeToRef) {

      bool reject=false;
       /*Point3D targetDestination_native(m_TargetDestination);
       if (nativeToRef) {
           nativeToRef->TransformPointInv(m_TargetDestination, targetDestination_native); // go from ref to native
       }*/

       //FloatVolume::Pointer vol = vectorVol[0];
       //SEEGTrajectoryROIPipeline::Pointer pipeline = SEEGTrajectoryROIPipeline::New(vol);

       list<ElectrodeInfo::Pointer>::iterator it;

       for (it = first; it != last; it++) {

           ElectrodeInfo::Pointer electrode = *it;
           Point3D entryPoint_native(electrode->m_EntryPointWorld);
           Point3D targetDestination_native(electrode->m_TargetPointWorld);
           if (nativeToRef) {
               nativeToRef->TransformPointInv(electrode->m_EntryPointWorld, entryPoint_native); // go from ref_to_native
               nativeToRef->TransformPointInv(electrode->m_TargetPointWorld, targetDestination_native); // go from ref_to_native
           }

        //   Point3D entryPoint_extrapolated;
        //   this->ExtrapolateEntryPoint(targetDestination_native, entryPoint_native, entryPoint_extrapolated, 10.0); //extrapolate by 10mm for vector test (is done on the skull)

           TrajectoryTestScore& testScore = electrode->m_TrajectoryTestScores[testName];


          // pipeline->CalcDistanceMap(entryPoint_extrapolated, targetDestination_native, cfgs.m_MaxDistToEvaluate);

           reject = TestVectorOverlap(
                               electrode->m_ElectrodeVectorWorld,
                               entryPoint_native,
                               vectorVol,
                               cfgs,
                               testScore,
                               nativeToRef);

           //Check whether to keep or reject trajectory
           if (reject) {
               electrode->m_Valid = false;
               electrode->m_AggregatedRiskScore = -1;
               electrode->m_AggregatedRewardScore = -1;
               electrode->m_AggregatedScore = -1;
           } else {
               electrode->m_Valid = true;
           }

       }
   }

   // Functions that aggregate scores
       void SEEGPathPlanner::AggregateRisks(int numBins) {
           map<string, TrajectoryRiskTestWeights>::iterator it;
           list<ElectrodeInfo::Pointer>::iterator it2;
           float minVal, maxVal, stepSize;
           float score;
           float rank;
           ElectrodeInfo::Pointer e;

           if (m_ActiveTrajectories.size() == 0) {
               return;
           }

           // initialization
           for (it2 = m_ActiveTrajectories.begin(); it2 != m_ActiveTrajectories.end(); it2++) {
               e = *it2;
               e->m_AggregatedRiskScore = 0;
           }


           for (it = m_TrajectoryRiskTestWeights.begin(); it != m_TrajectoryRiskTestWeights.end(); it++) {
               m_TestToCompare = (*it).first;
               TrajectoryRiskTestWeights weights = (*it).second;

               // TODO: this is stupid and slow (should instead find min and max values using a single loop)!
               m_ActiveTrajectories.sort(compareSEEGMax);
               minVal = m_ActiveTrajectories.front()->m_TrajectoryTestScores[m_TestToCompare].scoreMax;
               maxVal = m_ActiveTrajectories.back()->m_TrajectoryTestScores[m_TestToCompare].scoreMax;

               cout << "Test: " << m_TestToCompare << endl;
               cout << "ScoreMax min " << minVal << " max: " << maxVal << endl;
               stepSize = (maxVal - minVal) / numBins;


               for (it2 = m_ActiveTrajectories.begin(); it2 != m_ActiveTrajectories.end(); it2++) {
                   e = *it2;

                   if (maxVal-minVal < 0.001) {
                       rank = 1;
                   } else {
                       score = e->m_TrajectoryTestScores[m_TestToCompare].scoreMax;
                       rank = (int)((score - minVal) / stepSize) + 1;
                   }
                   e->m_TrajectoryTestScores[m_TestToCompare].rankingUsingMax = rank;
                   e->m_AggregatedRiskScore += weights.m_WeightUsingMax * rank;
               }


               m_ActiveTrajectories.sort(compareSEEGSum);
               minVal = m_ActiveTrajectories.front()->m_TrajectoryTestScores[m_TestToCompare].scoreSum;
               maxVal = m_ActiveTrajectories.back()->m_TrajectoryTestScores[m_TestToCompare].scoreSum;

               cout << "ScoreSum min " << minVal << " max: " << maxVal << endl;
               stepSize = (maxVal-minVal + 1) / numBins;

               for (it2 = m_ActiveTrajectories.begin(); it2 != m_ActiveTrajectories.end(); it2++) {
                   e = *it2;

                   if (maxVal - minVal < 0.001) {
                       rank = 1;
                   } else {
                       score = e->m_TrajectoryTestScores[m_TestToCompare].scoreSum;
                       rank = (int)((score - minVal) / stepSize) + 1;
                   }
                   e->m_TrajectoryTestScores[m_TestToCompare].rankingUsingSum = rank;
                   e->m_AggregatedRiskScore += weights.m_WeightUsingSum * rank;
               }
           }

           // sorting aggregated list
    //       m_ActiveTrajectories.sort(compareSEEGtraj);
       }

       void SEEGPathPlanner::AggregateRewards(int numBins) {
           map<string, TrajectoryTestWeights>::iterator it;
           list<ElectrodeInfo::Pointer>::iterator it2;
           float minVal, maxVal, stepSize;
           float score;
           float rank;
           ElectrodeInfo::Pointer e;

           if (m_ActiveTrajectories.size() == 0) {
               return;
           }

           // initialization
           for (it2 = m_ActiveTrajectories.begin(); it2 != m_ActiveTrajectories.end(); it2++) {
               e = *it2;
               e->m_AggregatedRewardScore = 0;
           }

           for (it = m_TrajectoryRewardTestWeights.begin(); it != m_TrajectoryRewardTestWeights.end(); it++) {
               m_TestToCompare = (*it).first;
               TrajectoryTestWeights weights = (*it).second;

               // TODO: this is stupid and slow (should instead find min and max values using a single loop)!
               m_ActiveTrajectories.sort(compareSEEGMax);
               minVal = m_ActiveTrajectories.front()->m_TrajectoryTestScores[m_TestToCompare].scoreMax;
               maxVal = m_ActiveTrajectories.back()->m_TrajectoryTestScores[m_TestToCompare].scoreMax;

               cout << "Test: " << m_TestToCompare << endl;
               cout << "ScoreMax min " << minVal << " max: " << maxVal << endl;
               stepSize = (maxVal - minVal) / numBins;



               for (it2 = m_ActiveTrajectories.begin(); it2 != m_ActiveTrajectories.end(); it2++) {
                   e = *it2;

                   if (maxVal-minVal < 0.001) {
                       rank = 1;
                   } else {
                       score = e->m_TrajectoryTestScores[m_TestToCompare].scoreMax;
                       rank = (int)((score - minVal) / stepSize) + 1;
                   }
                   e->m_TrajectoryTestScores[m_TestToCompare].rankingUsingMax = rank;
                   e->m_AggregatedRewardScore += weights.m_WeightUsingMax * rank;
               }


               m_ActiveTrajectories.sort(compareSEEGSum);
               minVal = m_ActiveTrajectories.front()->m_TrajectoryTestScores[m_TestToCompare].scoreSum;
               maxVal = m_ActiveTrajectories.back()->m_TrajectoryTestScores[m_TestToCompare].scoreSum;

               cout << "ScoreSum min " << minVal << " max: " << maxVal << endl;
               stepSize = (maxVal-minVal + 1) / numBins;

               for (it2 = m_ActiveTrajectories.begin(); it2 != m_ActiveTrajectories.end(); it2++) {
                   e = *it2;

                   if (maxVal - minVal < 0.001) {
                       rank = 1;
                   } else {
                       score = e->m_TrajectoryTestScores[m_TestToCompare].scoreSum;
                       rank = (int)((score - minVal) / stepSize) + 1;
                   }
                   e->m_TrajectoryTestScores[m_TestToCompare].rankingUsingSum = rank;
                   e->m_AggregatedRewardScore += weights.m_WeightUsingSum * rank;
               }
           }

       }

       void SEEGPathPlanner::AggregateRewardsAllDepths(int numBins) {
           map<string, TrajectoryTestWeights>::iterator it;
           list<ElectrodeInfo::Pointer>::iterator it2;
           float minScoreMax,maxScoreMax;
           float minScoreSum,maxScoreSum;
           float stepSizeSum, stepSizeMax;
           vector<float> minMaxVec, maxMaxVec;
           vector<float> minSumVec,maxSumVec;
           float score;
           float rankMax, rankSum;
           ElectrodeInfo::Pointer e;
           cout << "AggregateRewardsAllDepths: " << m_ActiveTrajectories.size() << endl;
           if (m_ActiveTrajectories.size() == 0) {
               cout << "AggregateRewardsAllDepths: No Active trajectories" << m_ActiveTrajectories.size() << endl;
               return;
           }
           // Check that there is test to aggregate
           if (m_TrajectoryRewardTestWeights.size() == 0) {
               cout << "AggregateRewardsAllDepths: No Reward tests"<< endl;
               return;
           }

           //Check that there is a vector of trajectories
           it = m_TrajectoryRewardTestWeights.begin();
           m_TestToCompare = (*it).first;
           int isVector=0;
           it2 = m_ActiveTrajectories.begin();
           while ( it2 != m_ActiveTrajectories.end() && isVector<1) {
               e = *it2;
               isVector = e->m_VecTrajectoryTestScores[m_TestToCompare].size();
               it2++;
           }
           cout << "AggregateRewardsAllDepths: #Vectors >= "<<isVector << endl;
           if (isVector < 1){
               cout << "AggregateRewardsAllDepths: Input is not Vector or only 1 point" << endl;
               AggregateRewards(numBins); //RIZ 20140203 -> test if this solves the problem!
               return;
           }

           // initialization
           for (it = m_TrajectoryRewardTestWeights.begin(); it != m_TrajectoryRewardTestWeights.end(); it++) {
               m_TestToCompare = (*it).first;
               minScoreMax=numeric_limits<float>::max();
               minScoreSum= numeric_limits<float>::max();
               maxScoreMax=0;
               maxScoreSum= 0;
               for (it2 = m_ActiveTrajectories.begin(); it2 != m_ActiveTrajectories.end(); it2++) {
                   e = *it2;
                   e->m_AggregatedRewardScore = 0;
                   ElectrodeInfo::VecTrajectoryTestScore vecTraj = e->m_VecTrajectoryTestScores[m_TestToCompare];
                   for (int iTP=0; iTP<vecTraj.size(); iTP++){
                       minScoreMax = min(minScoreMax, vecTraj[iTP].scoreMax);
                       maxScoreMax = max(maxScoreMax, vecTraj[iTP].scoreMax);
                       minScoreSum = min(minScoreSum, vecTraj[iTP].scoreSum);
                       maxScoreSum = max(maxScoreSum, vecTraj[iTP].scoreSum);
                   }
                  // cout << m_TestToCompare << minScoreSum << minScoreMax << " max: "<<maxScoreMax << maxScoreMax << endl;

               }

               minMaxVec.push_back(minScoreMax);
               maxMaxVec.push_back(maxScoreMax);
               minSumVec.push_back(minScoreSum);
               maxSumVec.push_back(maxScoreSum);
               cout << "Test: " << m_TestToCompare << endl;
               cout << "ScoreMax min " << minScoreMax << " max: " << maxScoreMax << endl;
               cout << "ScoreSum min " << minScoreSum << " max: " << maxScoreSum << endl;
           }

           // since all electrodes are of same type compute center of first center with respect to tip only once
           e =  m_ActiveTrajectories.front();
           SEEGElectrodeModel::Pointer electrodeModel = SEEGElectrodeModel::New(e->GetElectrodeModelType()); // all electrodes are assumed to be of the same type! if not -> bring this line inside loop

           //Compute best score combined for each electrode
           for (it2 = m_ActiveTrajectories.begin(); it2 != m_ActiveTrajectories.end(); it2++) {
               e = *it2;
              //cout<<" Electrode - target orig: "<< e->m_TargetPointWorld;

               float nTP = e->m_VecTrajectoryTestScores[m_TestToCompare].size(); //both must have same number of elements
               if (nTP>0){ // RIZ: not sure why sometimes there is NO vecTraj -> maybe completely outside??
                   float bestRankScoreMax=0;
                   float bestRankScoreSum=0;
                   int bestIndexTPSum=0;
                   for (int iTP=0; iTP<nTP; iTP++){
                       int iTest=0;
                       float combinedRankMax=0;
                       float combinedRankSum=0;
                       for (it = m_TrajectoryRewardTestWeights.begin(); it != m_TrajectoryRewardTestWeights.end(); it++) {
                           m_TestToCompare = (*it).first;
                           TrajectoryTestWeights weights = (*it).second;
                           ElectrodeInfo::VecTrajectoryTestScore vecTraj = e->m_VecTrajectoryTestScores[m_TestToCompare];

                           minScoreMax=minMaxVec[iTest];
                           maxScoreMax=maxMaxVec[iTest];
                           minScoreSum=minSumVec[iTest];
                           maxScoreSum=maxSumVec[iTest];

                           stepSizeMax = (maxScoreMax - minScoreMax) / numBins;
                           stepSizeSum = (maxScoreSum-minScoreSum + 1) / numBins;

                           rankMax =rankSum= 1;

                           if (maxScoreMax-minScoreMax > 0.001){
                               score = vecTraj[iTP].scoreMax;
                               rankMax = (int)((score - minScoreMax) / stepSizeMax) + 1;
                           }
                           e->m_VecTrajectoryTestScores[m_TestToCompare][iTP].rankingUsingMax = rankMax;

                           if (maxScoreSum-minScoreSum > 0.001) {
                               score = vecTraj[iTP].scoreSum;
                               rankSum = (int)((score - minScoreSum) / stepSizeSum) + 1;
                           }
                           e->m_VecTrajectoryTestScores[m_TestToCompare][iTP].rankingUsingSum = rankSum;
                           combinedRankMax += weights.m_WeightUsingMax * rankMax;
                           combinedRankSum += weights.m_WeightUsingSum * rankSum;
                           iTest++;
                       }
                       //Keep only max score for each trajectory
                       if (bestRankScoreMax < combinedRankMax) bestRankScoreMax = combinedRankMax;
                       if (bestRankScoreSum < combinedRankSum) {
                           bestRankScoreSum = combinedRankSum;
                           bestIndexTPSum = iTP; // the only important one is sum!
                       }
                   }

                   for (it = m_TrajectoryRewardTestWeights.begin(); it != m_TrajectoryRewardTestWeights.end(); it++) {
                       m_TestToCompare = (*it).first;
                       ElectrodeInfo::VecTrajectoryTestScore vecTraj = e->m_VecTrajectoryTestScores[m_TestToCompare];
                       e->m_TrajectoryTestScores[m_TestToCompare].scoreMax = vecTraj[bestIndexTPSum].scoreMax; // before: bestScoreMax; //assign same score to all tests
                       e->m_TrajectoryTestScores[m_TestToCompare].scoreSum = vecTraj[bestIndexTPSum].scoreSum; // before: bestScoreSum;
                       e->m_TrajectoryTestScores[m_TestToCompare].rankingUsingMax = vecTraj[bestIndexTPSum].rankingUsingMax;
                       e->m_TrajectoryTestScores[m_TestToCompare].rankingUsingSum = vecTraj[bestIndexTPSum].rankingUsingSum;
                       e->m_TrajectoryTestScores[m_TestToCompare].pointAtMaxScore = vecTraj[bestIndexTPSum].pointAtMaxScore; //center of first contact
                       Point3D newTargetPoint = electrodeModel->getElectrodeTipFromContactPosition(0, vecTraj[bestIndexTPSum].pointAtMaxScore, e->m_EntryPointWorld);
                       e->m_TargetPointWorld = newTargetPoint; // tip of electrode (center of 1st contact - contactSize/2 - distance from tip) - distFromTip
                       e->m_AggregatedRewardScore = bestRankScoreMax + bestRankScoreSum;
                   }
                  // cout<<" - New target: "<< e->m_TargetPointWorld << " - index: "<< bestIndexTPSum<<" - score: "<<bestScoreSum<<endl;
               }
           }
           // sort trajectories according to the best combined score for each
          // m_ActiveTrajectories.sort(compareSEEGSum);
            m_ActiveTrajectories.sort(compareAgreggatedReward);
       }

       void SEEGPathPlanner::AggregateAll(int numBins) {
           list<ElectrodeInfo::Pointer>::iterator it2;
           ElectrodeInfo::Pointer e;
           for (it2 = m_ActiveTrajectories.begin(); it2 != m_ActiveTrajectories.end(); it2++) {
               e = *it2;
               e->m_AggregatedScore = (m_WeightRisk * e->m_AggregatedRiskScore) + (m_WeightReward * (numBins - e->m_AggregatedRewardScore));
           }
           cout << "Aggregating All " << endl;

           // sorting aggregated list
          m_ActiveTrajectories.sort(compareSEEGtraj);

       }

  /*     void SEEGPathPlanner::AggregateGlobal(int numBins,  vector<list<ElectrodeInfo::Pointer>> otherActivePlans) {
           list<ElectrodeInfo::Pointer>::iterator it2;
           ElectrodeInfo::Pointer e;
           for (it2 = m_ActiveTrajectories.begin(); it2 != m_ActiveTrajectories.end(); it2++) {
               e = *it2;
               e->m_AggregatedGlobalScore =  e->m_AggregatedScore + min();
           }

           // sorting aggregated list
          m_ActiveTrajectories.sort(compareSEEGtraj);

       }
*/

   // read/write functions
      void SEEGPathPlanner::SaveSEEGPlanningDataToFile (const string& filename) {
          ofstream file;
          file.open(filename.c_str());
    /*      file << "[target] ";
          file << this->m_TargetDestination[0] << " ";
          file << this->m_TargetDestination[1] << " ";
          file << this->m_TargetDestination[2] << endl;
   */
          file << "[fileType] "<< "All" << endl;
          file << "[gralweights] ";
          file <<"Risk "<< m_WeightRisk << " Reward ";
          file << m_WeightReward << endl;
          map<string, TrajectoryRiskTestWeights>::iterator it;
          for (it=m_TrajectoryRiskTestWeights.begin(); it != m_TrajectoryRiskTestWeights.end(); it++) {
              //file << "[risktest] ";
              file << "[riskwithhardlimittest] ";
              file << (*it).first << " ";
              file << (*it).second.m_WeightUsingMax << " ";
              file << (*it).second.m_WeightUsingSum << " ";
              file << (*it).second.m_HardLimit << endl;
          }
          map<string, TrajectoryTestWeights>::iterator it2;
          for (it2=m_TrajectoryRewardTestWeights.begin(); it2 != m_TrajectoryRewardTestWeights.end(); it2++) {
              file << "[rewardtest] ";
              file << (*it2).first << " ";
              file << (*it2).second.m_WeightUsingMax << " ";
              file << (*it2).second.m_WeightUsingSum << endl;
          }

          list<ElectrodeInfo::Pointer>::iterator elecIt;

          for (elecIt = m_AllTrajectories.begin(); elecIt != m_AllTrajectories.end(); elecIt++) {
              ElectrodeInfo::Pointer el = *elecIt;
              file << "[trajectory] ";
              file << el->m_TargetPointWorld[0] << " ";
              file << el->m_TargetPointWorld[1] << " ";
              file << el->m_TargetPointWorld[2] << " ";
              file << el->m_EntryPointWorld[0] << " ";
              file << el->m_EntryPointWorld[1] << " ";
              file << el->m_EntryPointWorld[2] << " ";
              file << el->m_AggregatedScore << " ";
              file << el->m_AggregatedRiskScore << " ";
              file << el->m_AggregatedRewardScore << " ";

              map<string, TrajectoryTestScore>::iterator itScores;
              for (itScores=el->m_TrajectoryTestScores.begin(); itScores !=el->m_TrajectoryTestScores.end(); itScores++) {
                  file << (*itScores).first << " ";
                  file << (*itScores).second.scoreMax << " ";
                  file << (*itScores).second.scoreSum << " ";
                  file << (*itScores).second.rankingUsingMax << " ";
                  file << (*itScores).second.rankingUsingSum << " ";
                  file << (*itScores).second.distAtMaxScore << " ";
                  file << (*itScores).second.pointAtMaxScore[0] << " ";
                  file << (*itScores).second.pointAtMaxScore[1] << " ";
                  file << (*itScores).second.pointAtMaxScore[2] << " ";
              }
              file << endl;
          }

          file << "[order] "<< "targetx targety targetz entryx entryy entryz agregatted aggregRisk aggregReward ";
          file << "testType scoreMax scoreSum rankingMax rankingSum distAtMax xAtMax yAtMax zAtMax" << endl;


          file.close();
      }

      void SEEGPathPlanner::SaveActiveSEEGPlanningDataToFile (const string& filename) {
          // similar to  SavePlanningDataToFile but saves only Active (not rejected) trajectories
          cout<<"Saving trajectory to "<<filename.c_str() << endl;
          ofstream file;
          file.open(filename.c_str());
          file << "[fileType] "<< "Active" << endl;
          file << "[gralweights] ";
          file <<"Risk "<< m_WeightRisk << " Reward ";
          file << m_WeightReward << endl;
          map<string, TrajectoryRiskTestWeights>::iterator it;
          for (it=m_TrajectoryRiskTestWeights.begin(); it != m_TrajectoryRiskTestWeights.end(); it++) {
             // file << "[risktest] ";
              file << "[riskwithhardlimittest] ";
              file << (*it).first << " ";
              file << (*it).second.m_WeightUsingMax << " ";
              file << (*it).second.m_WeightUsingSum << " ";
              file << (*it).second.m_HardLimit << endl;
          }

          map<string, TrajectoryTestWeights>::iterator it2;
          for (it2=m_TrajectoryRewardTestWeights.begin(); it2 != m_TrajectoryRewardTestWeights.end(); it2++) {
              file << "[rewardtest] ";
              file << (*it2).first << " ";
              file << (*it2).second.m_WeightUsingMax << " ";
              file << (*it2).second.m_WeightUsingSum << endl;
          }

           list<ElectrodeInfo::Pointer>::iterator elecIt;

          for (elecIt = m_ActiveTrajectories.begin(); elecIt != m_ActiveTrajectories.end(); elecIt++) {
              ElectrodeInfo::Pointer el = *elecIt;
              file << "[trajectory] ";
              file << el->m_TargetPointWorld[0] << " ";
              file << el->m_TargetPointWorld[1] << " ";
              file << el->m_TargetPointWorld[2] << " ";
              file << el->m_EntryPointWorld[0] << " ";
              file << el->m_EntryPointWorld[1] << " ";
              file << el->m_EntryPointWorld[2] << " ";
              file << el->m_AggregatedScore<< " ";
              file << el->m_AggregatedRiskScore<< " ";
              file << el->m_AggregatedRewardScore<< " ";

              map<string, TrajectoryTestScore>::iterator itScores;
              for (itScores=el->m_TrajectoryTestScores.begin(); itScores !=el->m_TrajectoryTestScores.end(); itScores++) {
                  file << (*itScores).first << " ";
                  file << (*itScores).second.scoreMax << " ";
                  file << (*itScores).second.scoreSum << " ";
                  file << (*itScores).second.rankingUsingMax << " ";
                  file << (*itScores).second.rankingUsingSum << " ";
                  file << (*itScores).second.distAtMaxScore << " ";
                  file << (*itScores).second.pointAtMaxScore[0] << " ";
                  file << (*itScores).second.pointAtMaxScore[1] << " ";
                  file << (*itScores).second.pointAtMaxScore[2] << " ";
              }
              file << endl;
          }
          file << "[order] "<< "targetx targety targetz entryx entryy entryz agregatted aggregRisk aggregReward ";
          file << "testType scoreMax scoreSum rankingMax rankingSum distAtMax xAtMax yAtMax zAtMax" << endl;

          file.close();
      }

      bool SEEGPathPlanner::LoadSEEGPlanningDataFromFile (const string& filename) {
          ifstream file;
          string line;
          string token;
          string testName;

          bool status=false;
          Reset();

          file.open(filename.c_str());

          while(~file.eof()) {
              getline(file, line);
              stringstream ss (line);

              token="";
              getline(ss, token, ' ');

              if (token == "[fileType]") {

                  getline(ss, token, ' ');
                  testName = token;

              } else if (token == "[gralweights]") {

                  float weightRisk;
                  float weightReward;

                  getline(ss, token, ' '); //first is name "Risk" then value
                  getline(ss, token, ' ');
                  weightRisk = atof(token.c_str());

                  getline(ss, token, ' '); //first is name "Reward" then value
                  getline(ss, token, ' ');
                  weightReward = atof(token.c_str());

                  this->SetTrajectoryGlobalWeights(weightRisk, weightReward);

              } else if (token == "[risktest]") {

                  float weightMax;
                  float weightSum;

                  getline(ss, token, ' ');
                  testName = token;

                  getline(ss, token, ' ');
                  weightMax = atof(token.c_str());

                  getline(ss, token, ' ');
                  weightSum = atof(token.c_str());

                  this->SetTrajectoryRiskTestWeights(testName, weightMax, weightSum);

              } else if (token == "[riskwithhardlimittest]") { //newer files save also hardlimit

                  float weightMax;
                  float weightSum;
                  float hardLimitValue;

                  getline(ss, token, ' ');
                  testName = token;

                  getline(ss, token, ' ');
                  weightMax = atof(token.c_str());

                  getline(ss, token, ' ');
                  weightSum = atof(token.c_str());

                  getline(ss, token, ' ');
                  hardLimitValue = atof(token.c_str());

                  this->SetTrajectoryRiskTestWeights(testName, weightMax, weightSum, hardLimitValue);

              } else if (token == "[rewardtest]") {

                  float weightMax;
                  float weightSum;

                  getline(ss, token, ' ');
                  testName = token;

                  getline(ss, token, ' ');
                  weightMax = atof(token.c_str());

                  getline(ss, token, ' ');
                  weightSum = atof(token.c_str());

                  this->SetTrajectoryRewardTestWeights(testName, weightMax, weightSum);

              } else if (token == "[trajectory]") {
                  Point3D entryPointWorld, targetPointWorld;

                  getline(ss, token, ' ');
                  targetPointWorld[0] = atof(token.c_str());
                  getline(ss, token, ' ');
                  targetPointWorld[1] = atof(token.c_str());
                  getline(ss, token, ' ');
                  targetPointWorld[2] = atof(token.c_str());
                  getline(ss, token, ' ');
                  entryPointWorld[0] = atof(token.c_str());
                  getline(ss, token, ' ');
                  entryPointWorld[1] = atof(token.c_str());
                  getline(ss, token, ' ');
                  entryPointWorld[2] = atof(token.c_str());

                  ElectrodeInfo::Pointer ep = ElectrodeInfo::New(entryPointWorld, targetPointWorld);

                  getline(ss, token, ' ');
                  ep->m_AggregatedScore = atof(token.c_str());


                  getline(ss, token, ' ');
                  ep->m_AggregatedRiskScore = atof(token.c_str());

                  getline(ss, token, ' ');
                  ep->m_AggregatedRewardScore = atof(token.c_str());


                  while(ss) {
                      if (!getline(ss, token,' ')) {
                          break;
                      }

                      testName = token;

                      getline(ss, token,' ');
                      ep->m_TrajectoryTestScores[testName].scoreMax = atof(token.c_str());

                      getline(ss, token,' ');
                      ep->m_TrajectoryTestScores[testName].scoreSum = atof(token.c_str());

                      getline(ss, token,' ');
                      ep->m_TrajectoryTestScores[testName].rankingUsingMax = atof(token.c_str());

                      getline(ss, token,' ');
                      ep->m_TrajectoryTestScores[testName].rankingUsingSum = atof(token.c_str());

                      getline(ss, token,' ');
                      ep->m_TrajectoryTestScores[testName].distAtMaxScore = atof(token.c_str());

                      getline(ss, token,' ');
                      ep->m_TrajectoryTestScores[testName].pointAtMaxScore[0] = atof(token.c_str());

                      getline(ss, token,' ');
                      ep->m_TrajectoryTestScores[testName].pointAtMaxScore[1] = atof(token.c_str());

                      getline(ss, token,' ');
                      ep->m_TrajectoryTestScores[testName].pointAtMaxScore[2] = atof(token.c_str());

                  }

                  m_AllTrajectories.push_back(ep);
                  if (ep->m_AggregatedScore >= 0) {
                      m_ActiveTrajectories.push_back(ep);
                  }

              } else {
                  cout << token << " not found: Possible Parse error\n";
                 break;
              }
          }

          file.close();

          m_ActiveTrajectories.sort(compareSEEGtraj);

          if (m_ActiveTrajectories.size()>0){
              ElectrodeInfo::Pointer ep =  m_ActiveTrajectories.front();
              this->m_EntryPoint = ep->m_EntryPointWorld;
              this->m_TargetDestination = ep->m_TargetPointWorld;
              status = true;
          } else{
              cout << "No Active Trajectories Found in " << filename.c_str()<<endl;
          }
          return status;
      }

      FloatVolume::Pointer SEEGPathPlanner::CreateTrajectoryScoreVolume(FloatVolume::Pointer templateVolume) {
          FloatVolumeDuplicator::Pointer volDuplicator = FloatVolumeDuplicator::New();
          volDuplicator->SetInputImage(templateVolume);
          volDuplicator->Update();
          FloatVolume::Pointer vol = volDuplicator->GetOutput();
          vol->DisconnectPipeline();

          FloatVolumeRegionIterator it(vol, vol->GetLargestPossibleRegion());
          for (it.GoToBegin(); !it.IsAtEnd();++it) {
              it.Set(-1);
          }

          list<ElectrodeInfo::Pointer>::iterator it2;

          for (it2 = m_ActiveTrajectories.begin(); it2 != m_ActiveTrajectories.end() ; it2++ ) {
              ElectrodeInfo::Pointer electrode = *it2;
              FloatVolume::IndexType index;
              vol->TransformPhysicalPointToIndex(electrode->m_EntryPointWorld, index);
              vol->SetPixel(index, electrode->m_AggregatedScore);
          }
          return vol;
      }

      //comparative functions
      ElectrodeInfo::Pointer SEEGPathPlanner::FindNearestDistOptimalTrajectory (ElectrodeInfo::Pointer electrode, float distThreshold) {

          list<ElectrodeInfo::Pointer>::iterator it;
          ElectrodeInfo::Pointer best = electrode;
          float scoreBest = electrode->m_AggregatedScore;
          int pntCnt = 0;

          float maxDist = 0;
          float minDist = numeric_limits<float>::max(); // sum dist in mm


          for (it = m_ActiveTrajectories.begin(); it != m_ActiveTrajectories.end(); it++) {

              ElectrodeInfo::Pointer otherElectrode = *it;

              float sumDist = this->CalcDistanceBetweenPointsInTrajectories(electrode->m_ElectrodeVectorWorld, otherElectrode->m_ElectrodeVectorWorld);

              if (abs(sumDist) < abs(minDist)) minDist = sumDist;
              if (abs(sumDist) > abs(maxDist)) maxDist = sumDist;

              if (abs(sumDist) > abs(distThreshold)) {
                  continue;
              }

              pntCnt++;
              if (scoreBest < 0 || scoreBest > otherElectrode->m_AggregatedScore) {
                  scoreBest = otherElectrode->m_AggregatedScore;
                  best = otherElectrode;
              }
          }

          cout << "minDist = " << minDist << " maxDist = " << maxDist << endl;
          cout << "FindNearestDistOptimalTrajectory:: Total number of points tested: " << pntCnt << endl;
          return best;
      }


    /**** PROTECTED AND PRIVATE FUNCTIONS ****/


    void SEEGPathPlanner::TestMaximizationOverlap (
                                            FloatVolume::Pointer recDistanceMap,
                                             vector<Point3D> allContactPoints,
                                            const MaximizationTestCfg& cfgs,
                                            TrajectoryTestScore& score,
                                            GeneralTransform::Pointer nativeToRef) {
        float scoreSum = 0;
        float nContacts=0;
        float distAtMaxScore = -1;
        Point3D pointAtMaxScore = allContactPoints[0]; // keep this first point in the scoring to know to which depth of trajectory it corresponds

        if (!recDistanceMap.IsNull()) {

            StatisticsImageFilterType::Pointer statisticsImageFilter = StatisticsImageFilterType::New ();
            statisticsImageFilter->SetInput(recDistanceMap);
            statisticsImageFilter->Update();
            scoreSum = statisticsImageFilter->GetSum();
            //        scoreMax = statisticsImageFilter->GetMaximum();
            //  cout << "Max: " << scoreMax << " - Sum: " <<scoreSum << std::endl;
            //   WriteFloatVolume("/home/rina/test/recMap2.mnc", recDistanceMap);


            //Compute number of contacts within volume
            while (!allContactPoints.empty()) {
                Point3D contactPoint = allContactPoints.back();
                FloatVolume::IndexType contactPointIndex;
                recDistanceMap->TransformPhysicalPointToIndex(contactPoint, contactPointIndex);
                if (recDistanceMap->GetPixel(contactPointIndex)>0) {
                    nContacts++;
                }
                allContactPoints.pop_back();
            }
        }
        score.scoreMax = nContacts; // use scoreMax to record number of contacts inside volume
        score.scoreSum = scoreSum;  // scoreSum contains the volume recorded
        score.distAtMaxScore = distAtMaxScore;
        if (nativeToRef) {
            nativeToRef->TransformPoint(pointAtMaxScore, score.pointAtMaxScore);
        } else {
            score.pointAtMaxScore = pointAtMaxScore;
        }


    }

    bool SEEGPathPlanner::TestVectorOverlap (
                                            Vector3D_lf electrodeVector,
                                            Point3D entryPoint,
                                            vector<FloatVolume::Pointer> &vectorVol,
                                            const BinaryTestCfg& cfgs,
                                            TrajectoryTestScore& score,
                                            GeneralTransform::Pointer nativeToRef) {
        bool reject=false;
        Point3D pointAtMaxScore;
        float scoreVal =100;

        FloatVolume::IndexType voxelIndexNorm;
        float minDist = 4; // >sqrt(12);
        float minusOne = -1;
        //int count=1; int step =2;

        bool found = vectorVol[0]->TransformPhysicalPointToIndex(entryPoint, voxelIndexNorm);
        Vector3D_lf normalVec(0,0,0);
        float sumNorms=0;


        if (found){
            normalVec.x = vectorVol[0]->GetPixel(voxelIndexNorm);
            normalVec.y = vectorVol[1]->GetPixel(voxelIndexNorm);
            normalVec.z = vectorVol[2]->GetPixel(voxelIndexNorm);
            sumNorms = norm(normalVec);
            //look around voxel and keep the one with highest magnitude
            for (int i=0;i<6;i++){
                FloatVolume::IndexType newIndex = voxelIndexNorm;
                newIndex[i % 3]+= pow(minusOne, i);
                if (newIndex[i % 3] >= 0) {
                    Vector3D_lf v;
                    v.x = vectorVol[0]->GetPixel(newIndex);
                    v.y = vectorVol[1]->GetPixel(newIndex);
                    v.z = vectorVol[2]->GetPixel(newIndex);
                    if (sumNorms < norm(v)) { //assign to vecto the xyz with largest magnitude within or around voxel
                        normalVec = v;
                        sumNorms = norm(normalVec);
                    }
                }
            }
        }
        // if sumnorms=0 it probably means that trajectory is manual and is not on the skull
        if ((sumNorms==0) || (found==false)){
            //Find skull intersect (only within entry point space) of trajectory
            //  by computing the vector between entry point and skull point at each skull point
            float minAngle =90;//the point corresponding to vector of min angle with electrodeVector is the closest
            IntVolume::IndexType indClosestPtInSkull;
            indClosestPtInSkull[0]=0;indClosestPtInSkull[1]=0;indClosestPtInSkull[2]=0;
            Point3D finalPtInSkull;
            FloatVolumeRegionIteratorWithIndex it (vectorVol[0], vectorVol[0]->GetRequestedRegion());
            for (it.GoToBegin(); !it.IsAtEnd(); ++it) {
                IntVolume::IndexType voxelIndex = it.GetIndex();
                if (it.Get()) {
                    Point3D ptInSkull;
                    vectorVol[0]->TransformIndexToPhysicalPoint(voxelIndex, ptInSkull);
                    Vector3D_lf v;
                    v.x = entryPoint[0] - ptInSkull[0];
                    v.y = entryPoint[1] - ptInSkull[1];
                    v.z = entryPoint[2] - ptInSkull[2];
                    float angleSkullPt = CalcAngleBetweenTrajectories(electrodeVector, v);
                    angleSkullPt = min(abs(angleSkullPt), abs(180-angleSkullPt));
                    if (angleSkullPt<minAngle){
                        minAngle=angleSkullPt;
                        indClosestPtInSkull = voxelIndex;
                        finalPtInSkull = ptInSkull;
                        normalVec =v;
                    }
                }
            }
            sumNorms = norm(normalVec);
            voxelIndexNorm=indClosestPtInSkull;
            for (int i=0;i<6;i++){ //look aroud vowel and keep the one with highest magnitude
                FloatVolume::IndexType newIndex = indClosestPtInSkull;
                newIndex[i % 3]+= pow(minusOne, i);
                if (newIndex[i % 3] >= 0) {
                    Vector3D_lf v;
                    v.x = vectorVol[0]->GetPixel(newIndex);
                    v.y = vectorVol[1]->GetPixel(newIndex);
                    v.z = vectorVol[2]->GetPixel(newIndex);
                    if (sumNorms < norm(v)) { //assign to vector the xyz with largest magnitude within or around voxel
                        normalVec = v;
                        sumNorms = norm(normalVec);
                        voxelIndexNorm=newIndex;
                    }
                }
            }
            //cout<<"EP "<<entryPoint <<" not found in Vector file - newSumNorms="<<sumNorms<<endl;
        }
        Vector3D_lf normalUnitVec = normalVec/sumNorms;
        minDist = norm(electrodeVector-normalUnitVec);
        // Find the point closest to entryPoint that has a defined normal??
        // if volume is the same as entry points it should not matter (it would always be included)

        //compute angle between normal and electrode at the closest point
        float angle = CalcAngleBetweenTrajectories(electrodeVector, normalUnitVec);
        if (isnan(angle)) {angle=90;} //the worst case
        angle = min(abs(angle), abs(180-angle));
        if (cfgs.m_HardConstraint && (angle >= cfgs.m_k1 || sumNorms==0)) {
            reject = true;
            //break;
        }
        scoreVal = abs(angle) / 90; //the smaller the better

        float distScore = CalcDistFromTrajFactor(angle, cfgs.m_k1, cfgs.m_k2); //RIZ: THIS should be REMOVED for angle!!
        scoreVal = scoreVal * distScore;

        vectorVol[0]->TransformIndexToPhysicalPoint(voxelIndexNorm, pointAtMaxScore);
        score.scoreMax = scoreVal;
        score.scoreSum = 0; //only compute angle at one point --> only MAX
        score.distAtMaxScore = angle;
        if (nativeToRef) {
            nativeToRef->TransformPoint(pointAtMaxScore, score.pointAtMaxScore);
        } else {
            score.pointAtMaxScore = pointAtMaxScore;
        }

        return reject;
    }

    void SEEGPathPlanner::ExtrapolateTargetPoint(Point3D& targetIn, Point3D& entryPt, Point3D& targetOut, const float lenExtra) {

        Vector3D_lf pTP(targetIn[0], targetIn[1], targetIn[2]);
        Vector3D_lf pEP(entryPt[0], entryPt[1], entryPt[2]);

        float d = norm(pEP-pTP);

        // extrapolate by val mm
        float deltaX = lenExtra * (pEP.x - pTP.x) / d; //for entry points val =10.0mm
        float deltaY = lenExtra * (pEP.y - pTP.y) / d;
        float deltaZ = lenExtra * (pEP.z - pTP.z) / d;


        targetOut[0] = targetIn[0] - deltaX;
        targetOut[1] = targetIn[1] - deltaY;
        targetOut[2] = targetIn[2] - deltaZ;

    }

    void SEEGPathPlanner::ExtrapolateEntryPoint(Point3D& targetPt, Point3D& entryIn, Point3D& entryOut, const float lenExtra) {

        Vector3D_lf pTP(targetPt[0], targetPt[1], targetPt[2]);
        Vector3D_lf pEP(entryIn[0], entryIn[1], entryIn[2]);

        float d = norm(pEP-pTP);

        // extrapolate by val mm
        float deltaX = lenExtra * (pEP.x - pTP.x) / d; //for entry points original val =10.0mm
        float deltaY = lenExtra * (pEP.y - pTP.y) / d;
        float deltaZ = lenExtra * (pEP.z - pTP.z) / d;


        entryOut[0] = entryIn[0] + deltaX;
        entryOut[1] = entryIn[1] + deltaY;
        entryOut[2] = entryIn[2] + deltaZ;
    }

    void SEEGPathPlanner::ExtrapolateEntryPointToFixLength(Point3D& targetPt, Point3D& entryIn, Point3D& entryOut, const float lenFix) {

        Vector3D_lf pTP(targetPt[0], targetPt[1], targetPt[2]);
        Vector3D_lf pEP(entryIn[0], entryIn[1], entryIn[2]);

        float d = norm(pEP-pTP);
        float lenExtra = lenFix - d;
        ExtrapolateEntryPoint(targetPt, entryIn, entryOut, lenExtra);
    }

    float SEEGPathPlanner::CalcAngleBetweenTrajectories(Vector3D_lf& v1, Vector3D_lf& v2) {

  /*      Vector3D_lf v1( p1[0] - this->m_TargetDestination[0],
                        p1[1] - this->m_TargetDestination[1],
                        p1[2] - this->m_TargetDestination[2]);

        Vector3D_lf v2( p2[0] - this->m_TargetDestination[0],
                        p2[1] - this->m_TargetDestination[1],
                        p2[2] - this->m_TargetDestination[2]);
*/

        v1 = v1 / norm(v1);
        v2 = v2 / norm(v2);

        float dotProd = v1.DotProd(v2);
        if (dotProd > 1) dotProd = 1;
        float a = acos(dotProd);
        a = rad2deg(a);
        return a;
    }

    float SEEGPathPlanner::CalcDistanceBetweenTrajectories(vector<Point3D> &v1, vector<Point3D> &v2) {
        // computes summed distance between trajectories - consideres distance between central lines without considering the length
        // thus it is not the point to point distance, but a line to line distance


      //  v1 = v1 / norm(v1);
       // v2 = v2 / norm(v2);

        Point3D closestPt;
        int indexInList;
        float sumDist =0;
        for (int i=0; i<v1.size(); i++) {
            float minDist = FindClosestPoint3D(v1[i], v2, closestPt, &indexInList);
            sumDist += minDist;
        }

        return sumDist;
    }

    float SEEGPathPlanner::CalcDistanceBetweenPointsInTrajectories(Vector3D_lf& v1, Vector3D_lf& v2) {
        // computes summed distance between trajectories - consideres distance between each pair of points
        // thus the distance is simply the euclidean distance (sqrt[(Ax-Bx)^2 + (Ay-By)^2 + (Az-Bz)^2])

       // v1 = v1 / norm(v1);
       // v2 = v2 / norm(v2);

        float sumDist = norm(v1-v2);

        return sumDist;
    }



}


