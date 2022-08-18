#ifndef __SEEGTRAJECTORY_ROI_PIPELINE__
#define __SEEGTRAJECTORY_ROI_PIPELINE__

/**
 * @file SEEGTrajectoryROIPipeline.h
 *
 * Based on TrajectoryROIPipeline.h which
 * Defines an itk pipeline to create a 3D volume mask (e.g. cylinder of interest) for
 * a specific trajectory
 * the only difference is that DistanceMap are computed with target from electrode list.
 * and constructors do not initilize target point
 *
 * changed to be derived from SEEGROIPipeline
 * @author Silvain Beriault & Rina Zelmann
 */

// Header files to include
#include "BasicTypes.h"
#include "MathUtils.h"
#include "VolumeTypes.h"
#include <string>
//#include "SEEGROIPipeline.h"
#include "itkBinaryThresholdImageFilter.h"
#include "DistanceFromTrajectoryImageFilter.h"
//#include "SEEGElectrodeModel.h"

using namespace std;
using namespace itk;

namespace seeg {


    /**
     * This class contains a small itk pipeline to create a 3D binary mask (e.g. cylinder of interest)
     *  that includes all neighbouring voxels of a linear trajectory (within a specific radius)
     */
    class SEEGTrajectoryROIPipeline {

    public:
        /** SmartPointer type for the SEEGROIPipeline class */
        typedef mrilSmartPtr<SEEGTrajectoryROIPipeline> Pointer;

        static Pointer New(FloatVolume::Pointer templateVolume) { return Pointer(new SEEGTrajectoryROIPipeline(templateVolume)); }

        static Pointer New(IntVolume::Pointer templateVolume) { return Pointer(new SEEGTrajectoryROIPipeline(templateVolume)); }

        static Pointer New(ByteVolume::Pointer templateVolume) { return Pointer(new SEEGTrajectoryROIPipeline(templateVolume)); }

        static Pointer New(FloatVectorVolume::Pointer templateVolume) { return Pointer(new SEEGTrajectoryROIPipeline(templateVolume)); }


    protected:
        //Constructors
       SEEGTrajectoryROIPipeline (FloatVolume::Pointer templateVolume);

       SEEGTrajectoryROIPipeline (IntVolume::Pointer templateVolume);

       SEEGTrajectoryROIPipeline (ByteVolume::Pointer templateVolume);

       SEEGTrajectoryROIPipeline (const string& templateVolumeFile);

       SEEGTrajectoryROIPipeline (FloatVectorVolume::Pointer templateVolume);


    public:
       virtual ~SEEGTrajectoryROIPipeline();

    public:
        /**
         * Changed CalcDistanceMap to have target as iput (RIZ)
         */
        void CalcDistanceMap( Point3D entryPointWorld,
                              Point3D targetPointWorld,
                              float maxRadius,
                              bool fullImage = false);


        /**
         * Returns a pointer to the FloatVolume containing a distance map (the distance of each
         * voxel from the trajectory centerline)
         *
         * @param copy true to duplicate the volume, false to return a direct pointer
         * @return A pointer to the distanceMap FlotVolume
         */
        FloatVolume::Pointer GetLastDistanceMap(bool copy = false);


        /**
         *  Returns all points within a trajectory and around by radius
         */
//        void GetAllPointsAroundTrajectory(Point3D targetPoint, Point3D entryPoint, float maxRadius, vector<Point3D> & allPointsVec);

        //Getters and Setters
        FloatVolume::Pointer GetTemplateVol();

        void SetTemplateVol(FloatVolume::Pointer templateVol);


    private:

        /**
         * Private initialization of the pipeline
         */
        void InitPipeline();


        /** private type for a DistanceFromtrajectoryImageFilter (for 3D FloatVolume) */
        typedef DistanceFromTrajectoryImageFilter<FloatVolume, FloatVolume> DistFromTrajVolumeFilter;

        /** Pointer to the template volume */
        FloatVolume::Pointer m_TemplateVolume; // for volume dimension, spacing, etc

        /** Instance of a DistFromTrajVolumeFilter */
        DistFromTrajVolumeFilter::Pointer m_DistFromTrajVolFilt;

//        SEEGElectrodeModel::Pointer m_ElectrodeModel;

    };
}

#endif
