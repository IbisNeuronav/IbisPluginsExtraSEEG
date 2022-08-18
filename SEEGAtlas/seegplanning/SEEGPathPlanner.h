#ifndef __SEEG_PATH_PLANNER_H__
#define __SEEG_PATH_PLANNER_H__

/**
 * @file SEEGPathPlanner.h
 *
 * Implementation of the SEEGPathPlanner class: the core class that performs automatic
 * trajectory planning for SEEG implantation.
 *
 * @author Silvain Beriault & Rina Zelmann
 */

// Header files to include
#include <vector>
#include <string>
#include "ElectrodeInfo.h"
#include "GeneralTransform.h"
#include "VolumeTypes.h"
#include <map>
#include "itkStatisticsImageFilter.h"
//#include "itkMinimumMaximumImageCalculator.h"
#include "PathPlanner.h"
#include "BasicTypes.h"

using namespace std;


namespace seeg {

    /** RIZ: see what to put here!!!!
     * This struct contains all the configuration parameters to be used
     * along with the SEEGPathPlanner::DoFuzzyTest()
     */
    struct MaximizationTestCfg {

        /** maximal distance to trajectory to evaluate */
        float m_MaxDistToEvaluate;

        /**
         * First constant for the Maximization test cost function. Any trajectory
         * which intersects a foreground voxel at a distance < m_K1 is given
         * a distance weight of 1
         */
        float m_k1;

        /**
         * Second constant for the Maximization test cost function. The cost function
         * of a foreground voxel is inversely proportional to its distance from the
         * trajectorie's centre line (i.e. 1/dist). k2 is a multiplicative factor
         * to reduce or increase the speed at which the const function decreases to 0
         * (i.e. 1 / (k2 * dist))
         */
        float m_k2;

        /**
         * Name of previous test.
         * Use only to obtain number of items in vector of scores
         * in Target Max in order to use the same number in GM max.
         */
        string m_NamePreviousTest;

        /**
          * Wether to analize multiple depth or not
          * in general (and by default) is set to 1
          * when running for true electrodes must be set to 0
          * to only look at the specified depth
          */
        bool m_AnalizeMultipleDepths;

        /**
          * Wether to analizeonly contacts that are inside the volume or not
          * it could be different for target and NC
          */
        vector<bool> m_OnlyInsideContacts;
    };

    /**
     * Weighting used for the SEEGPathPlanner::Aggregate() step for one trajectory test.
     * Similar to PathPlanner::TrajectoryTestWeights but has also the HardLimit Value (useful to store)
     */
    struct TrajectoryRiskTestWeights {

        /** Weight for the Sum criterion */
        float m_WeightUsingSum;

        /** Weight for the Max criterion */
        float m_WeightUsingMax;

        /** Weight for Hard Limit value */
        float m_HardLimit;

        /** Constructor with default weights */
        TrajectoryRiskTestWeights() {
            m_WeightUsingSum = 1;
            m_WeightUsingMax = 1;
            m_HardLimit = -1; //-1 means not enabled
        }
    };


     /**
     * The SEEGPathPlanner class performs the automatic trajectory planning algorithm
     * derives from (Simon's) PathPlanner
     */
    class SEEGPathPlanner : public PathPlanner {

    private:
        /** Electrode Name - Allows to identify to which location each trajectoryPLanner belongs**/
        string m_ElectrodeName;

        /** The entry point in world coordinates */
        Point3D m_EntryPoint;

        /** List of all trajectories to analyze (which includes entry points rejected by the
         * DoBinaryTest() */
        list<ElectrodeInfo::Pointer> m_AllTrajectories;

        /** List of active trajectories (those that were not rejected by any call to the
         * DoBinaryTest() */
        list<ElectrodeInfo::Pointer> m_ActiveTrajectories;

        /** A map of the RISK weights for all trajectory tests. Those weights are used by the
         * AggregateRisk(). The key for the map are string representing each trajectory tests.
         */
        map<string, TrajectoryRiskTestWeights> m_TrajectoryRiskTestWeights;

        /** A map of the REWARD weights for all trajectory tests. Those weights are used by the
         * AggregateReward(). The key for the map are string representing each trajectory tests.
         */
        map<string, TrajectoryTestWeights> m_TrajectoryRewardTestWeights;

        /**
          * Wieght of risk and reward - used by AggregateAll
          */
        float m_WeightRisk;

        float m_WeightReward;

    public:
        // smart pointer
        typedef mrilSmartPtr<SEEGPathPlanner> Pointer;

    public:

         static Pointer New() {
            return Pointer(new SEEGPathPlanner());
        }

        /**
         * Destructor
         */
        virtual ~SEEGPathPlanner();



        /**
         * Reset SEEGPathPlanner
         */
        void Reset();

        /**
         * Accessor for the list of active trajectories
         *
         * @return a list of all active trajectories
         */
        const list<ElectrodeInfo::Pointer>& GetActiveTrajectories() {
            return m_ActiveTrajectories;
        }

        /**
         * Accessor for the list of all trajectories
         *
         * @return a list of all trajectories
         */
        const list<ElectrodeInfo::Pointer>& GetAllTrajectories() {
            return m_AllTrajectories;
        }

        /**
         * Accessor for a particular active trajectories -> indicated by index
         *
         * @return active trajectory that corresponds to index
         */
        const ElectrodeInfo::Pointer& GetActiveTrajectoryByIndex(const int indexTraj) {
            list<ElectrodeInfo::Pointer>::iterator it = m_ActiveTrajectories.begin();
            advance(it, indexTraj);
            return *it;
        }

        /** Add Trajectories that were manually specified as Plans in GUI
          *
          *
          */
        void addManualPlans( const Point3D& entryPoint, const Point3D& targetPoint, const SEEGElectrodeModel::SEEG_ELECTRODE_MODEL_TYPE electrodeType);

        /**
         * Set the weighting for a particular trajectory test (for the Aggregate())
         *
         * @param testName the name of the test
         * @param weightUsingMax the weight for the max criterion
         * @param weightUsingSum the weight for the sum criterion
         */
        void SetTrajectoryRiskTestWeights(const string& testName, float weightUsingMax, float weightUsingSum);

        void SetTrajectoryRiskTestWeights(const string& testName, float weightUsingMax, float weightUsingSum, float hardLimitValue);

        void SetTrajectoryRewardTestWeights(const string& testName, float weightUsingMax, float weightUsingSum);

         map<string, TrajectoryRiskTestWeights>& GetTrajectoryRiskTestWeights();

         map<string, TrajectoryTestWeights>& GetTrajectoryRewardTestWeights();

        void SetTrajectoryGlobalWeights(float weightRisk, float weightReward);


        float GetGlobalRiskWeight();

        float GetGlobalRewardWeight();

        string GetElectrodeName();

        void SetElectrodeName(const string electrodeName);

        int GetNumberOfActiveTrajectories();

        /**
         * Setter for the entry point
         *
         * @param target_native world coordinates of the entry point in native space
         * @param nativeToRef native to ref transform
         */
        void SetEntryPoint(  const Point3D& target_native, GeneralTransform::Pointer nativeToRef = GeneralTransform::Pointer());


        /**
         * Accessor for the entry point
         *
         * @return the world coords of the entry point
         */
        Point3D GetEntryPoint();

        /**
         * Accessor for the weights
         *
         * @return map of test weights
         */
        map<string, TrajectoryRiskTestWeights>& GetRiskWeights();

        map<string, TrajectoryTestWeights>& GetRewardWeights();

        /**
         * Remove the weights info for a specific trajectory test (for the Aggregate())
         *
         * @param testName the name of the test
         */
        void DeleteTrajectoryTest(const string& testName);


        /**
         * Retrieve info about the closest trajectory to the specified Point3D
         *
         * @param entryPoint An entry point in native space
         * @param targetPoint A target point in native space
         * @param angleBetweenTraj A reference to a double where to store the angle between the supplied Point3D
         *                         and the retrieved trajectory
         * @param nativeToRef native to ref transform -- RIZ: it can probably be removed!
         * @return Info about the retrieved trajectory
         */
        ElectrodeInfo::Pointer FindElectrodeInfo( Point3D targetPoint,
                                                  Point3D entryPoint,
                                                  float& angleBetweenTraj,
                                                  GeneralTransform::Pointer nativeToRef = GeneralTransform::Pointer());

        /**
         * Retrieve info about the closest trajectory (but only among active entry points) -- RIZ: does not cinder target point! It will have to be changed!
         *
         * @param entryPoint An entry point in native space
         * @param targetPoint A target point in native space
         * @param angleBetweenTraj A reference to a double where to store the angle between the supplied Point3D
         *                         and the retrieved trajectory
         * @param nativeToRef native to ref transform
         * @return Info about the retrieved trajectory -- RIZ: it can probably be removed!
         */
        ElectrodeInfo::Pointer FindActiveElectrodeInfo( Point3D targetPoint,
                                                        Point3D entryPoint,
                                                        float& angleBetweenTraj,
                                                        GeneralTransform::Pointer nativeToRef = GeneralTransform::Pointer());





        /**
         * This function initializes the m_AllTrajectories and m_ActiveTrajectories using the
         * entryPointMaskVol (a mask containing all entry point voxels to be tested).
         * and the targetMaskVol(a mask containing all target point voxels to be tested).
         *
         * @param entryPointMaskVol Binary volume containing the entry points
         * @param targetMaskVol Binary volume containing the target points
         */
        void InitializeTrajectoriesFromVols(IntVolume::Pointer entryPointMaskVol, IntVolume::Pointer targetMaskVol);

        /**
         * This function initializes the m_AllTrajectories and m_ActiveTrajectories using the
         * .dat file with all the trajectories (.dat file contains all targetXYZ entryPointsXYZ).
         *
         * @param filename .dat file with trajectories (created in matlab: function getTrajectoryFromTargetEntryPoint)
         */
        void InitializeTrajectoriesFromFile(const std::string& filename, const SEEGElectrodeModel::SEEG_ELECTRODE_MODEL_TYPE electrodeType);

        void RemoveInvalidPaths();

        void DoSEEGMultiTest (  vector<string>& binTestNames,
                            vector<IntVolume::Pointer>& binVols,
                            vector<BinaryTestCfg>& binTestCfgs,

                            vector<string>& fuzzyTestNames,
                            vector<FloatVolume::Pointer>& fuzzyVols,
                            vector<FuzzyTestCfg>& fuzzyTestCfgs,

                            map<string, float>& extraLengthCfgs,

                            GeneralTransform::Pointer nativeToRef = GeneralTransform::Pointer());


        void DoSEEGMultiTest (  vector<string>& binTestNames,
                            vector<IntVolume::Pointer>& binVols,
                            vector<BinaryTestCfg>& binTestCfgs,

                            vector<string>& fuzzyTestNames,
                            vector<FloatVolume::Pointer>& fuzzyVols,
                            vector<FuzzyTestCfg>& fuzzyTestCfgs,

                            map<string, float>& extraLengthCfgs,

                            list<ElectrodeInfo::Pointer>::iterator first,
                            list<ElectrodeInfo::Pointer>::iterator last,
                            GeneralTransform::Pointer nativeToRef = GeneralTransform::Pointer());
        /**
         * Apply the binary test
         *
         * @param testName the name of the test
         * @param binaryVol a segmented (binary) volume containing a critical structure to be avoided
         *                  during the planning
         * @param cfgs The configurations for this test
         * @param numThreads number of threads to use
         * @param nativeToRef native to ref transform
         */
/*        void DoBinaryTest ( const string& testName,
                            IntVolume::Pointer binaryVol,
                            const BinaryTestCfg& cfgs,
                            GeneralTransform::Pointer nativeToRef = GeneralTransform::Pointer());


        void DoBinaryTest ( const string& testName,
                            IntVolume::Pointer binaryVol,
                            const BinaryTestCfg& cfgs,
                            list<ElectrodeInfo::Pointer>::iterator first,
                            list<ElectrodeInfo::Pointer>::iterator last,
                            GeneralTransform::Pointer nativeToRef = GeneralTransform::Pointer());
*/

        /**
         * Apply the fuzzy test
         *
         * @param testName the name of the test
         * @param fuzzyVol the fuzzy volume to process
         * @param cfgs The configurations for this test
         * @param nativeToRef native to ref transform
         */
 /*       void DoFuzzyTest(  const string& testname,
                           FloatVolume::Pointer fuzzyVol,
                           const FuzzyTestCfg& cfgs,
                           GeneralTransform::Pointer nativeToRef = GeneralTransform::Pointer());

        void DoFuzzyTest(  const string& testName,
                           FloatVolume::Pointer fuzzyVol,
                           const FuzzyTestCfg& cfgs,
                           list<ElectrodeInfo::Pointer>::iterator first,
                           list<ElectrodeInfo::Pointer>::iterator last,
                           GeneralTransform::Pointer nativeToRef = GeneralTransform::Pointer());

*/
        /**
         * Apply the Positive test
         * - maximize presence of contacts in volume of interest
         * - maximize presence of contacts in gray matter
         * (added by RIZ)
         *
         * @param testName the name of the test
         * @param targetVol the structure volume to process
         * @param cfgs The configurations for this test
         */
        void DoMaximizationTest(  const string& testname,
                                  FloatVolume::Pointer targetDistMap,
                                  const MaximizationTestCfg& cfgs,
                                  GeneralTransform::Pointer nativeToRef = GeneralTransform::Pointer());

        void DoMaximizationTest(  const string& testName,
                                  FloatVolume::Pointer targetDistMap,
                                  const MaximizationTestCfg& cfgs,
                                  list<ElectrodeInfo::Pointer>::iterator first,
                                  list<ElectrodeInfo::Pointer>::iterator last,
                                  GeneralTransform::Pointer nativeToRef = GeneralTransform::Pointer());

        void DoMaximizationTest(  vector<string> &testNames,
                                  const vector<FloatVolume::Pointer> &targetDistMaps,
                                  const MaximizationTestCfg& cfgs,
                                  vector<string>& binTestNames,
                                  vector<IntVolume::Pointer>& binVols,
                                  vector<BinaryTestCfg>& binTestCfgs,
                                  list<ElectrodeInfo::Pointer>::iterator first,
                                  list<ElectrodeInfo::Pointer>::iterator last,
                                  GeneralTransform::Pointer nativeToRef= GeneralTransform::Pointer());

        /**
         * Apply the Vector's test
         * - For angle: minimize angle with normal to skull
         * (added by RIZ)
         *
         * @param testName the name of the test
         * @param vectorVol the vector volume containing the normals to the surface to process
         * @param cfgs The configurations for this test
         */
        void DoVectorTest(  const string& testname,
                                  vector<FloatVolume::Pointer> &vectorVol,
                                  const BinaryTestCfg& cfgs,
                                  GeneralTransform::Pointer nativeToRef = GeneralTransform::Pointer());

        void DoVectorTest(  const string& testName,
                                  vector<FloatVolume::Pointer> &vectorVol,
                                  const BinaryTestCfg &cfgs,
                                  list<ElectrodeInfo::Pointer>::iterator first,
                                  list<ElectrodeInfo::Pointer>::iterator last,
                                  GeneralTransform::Pointer nativeToRef = GeneralTransform::Pointer());


         /**
         * Performs final trajectory rank aggregation in the specified number of bins
         *
         * @param numBins number of bins for the final histogram
         */
        void AggregateRisks(int numBins);

        void AggregateRewards(int numBins);

        void AggregateRewardsAllDepths(int numBins);

        void AggregateAll(int numBins);

      //  void AggregateGlobal(int numBins,   otherActivePlans);

        /**
         * Save all planning data to a text file.
         * (I cannot believe I am doing that rather than using xml or something...)
         *
         * @param filename the name of the text file where to save planning data
         */
        void SaveSEEGPlanningDataToFile (const string& filename);

        /**
         * Save only active planning data to a text file.
         * (I cannot believe I am doing that rather than using xml or something...)
         *
         * @param filename the name of the text file where to save planning data
         */
        void SaveActiveSEEGPlanningDataToFile (const string& filename);

        /**
         * Load planning data from a text file
         *
         * @param filename the name of the text file where to load the planning data.
         */
        bool LoadSEEGPlanningDataFromFile(const string& filename);

        /**
         * Create a volume containing the trajectory scores
         *
         * @param templateVolume A template volume containing resolution, spacing, origin, etc
         * @return A FloatVolume::Pointer containing the trajectory scores.
         */
        FloatVolume::Pointer CreateTrajectoryScoreVolume(FloatVolume::Pointer templateVolume);


        /**
         * Finds best scored trajectory  within radiusAround using CalcDistanceBetweenPointsInTrajectories
         */
        ElectrodeInfo::Pointer FindNearestDistOptimalTrajectory (ElectrodeInfo::Pointer electrode, float distThreshold);

    protected:

        /**
         * Protected constructor
         */
        SEEGPathPlanner();

    private:


        void TestMaximizationOverlap (
                               FloatVolume::Pointer recDistanceMap,
                               vector<Point3D> targetPoint,
                               const MaximizationTestCfg &cfgs,
                               TrajectoryTestScore& score,
                               GeneralTransform::Pointer nativeToRef);




        bool TestVectorOverlap(
                Vector3D_lf electrodeVector,
                Point3D entryPoint,
                vector<FloatVolume::Pointer> &vectorVol,
                const BinaryTestCfg &cfgs,
                TrajectoryTestScore& score,
                GeneralTransform::Pointer nativeToRef);


        /**
         * Computes angle between 2 vectors (using vectors - in PathPlanning idem using points)
         */
        float CalcAngleBetweenTrajectories(Vector3D_lf &v1, Vector3D_lf &v2);

        /**
         * Computes distance between 2 trajectories (considering the central line, not each point)
         */
        float CalcDistanceBetweenTrajectories(vector<Point3D> &v1, vector<Point3D> &v2);

        /**
         * Computes distance between 2 trajectories (considering each point)
         */
        float CalcDistanceBetweenPointsInTrajectories(Vector3D_lf &v1, Vector3D_lf &v2);

        /**
         * Extrapolate Target by lenExtra specified (in mm)
         */
        void ExtrapolateTargetPoint(Point3D& targetIn, Point3D& entryPt, Point3D& targetOut, const float lenExtra);

        /**
         * Extrapolate Entry to achievbe as full length the length specified  by LenFix(in mm)
         */
        void ExtrapolateEntryPointToFixLength(Point3D& target, Point3D& entryIn, Point3D& entryOut, const float lenFix);

        typedef itk::StatisticsImageFilter<FloatVolume> StatisticsImageFilterType;
  //      typedef itk::MinimumMaximumImageCalculator <FloatVolume> ImageCalculatorFilterType;

        //typedef vector<list<ElectrodeInfo::Pointer>> vectorOfTrajectoriesType;

    public:
        /**
         * Extrapolate Entry by lenExtra specified (in mm)
         */
        void ExtrapolateEntryPoint(Point3D& target, Point3D& entryIn, Point3D& entryOut, const float lenExtra);

    };

}

#endif
