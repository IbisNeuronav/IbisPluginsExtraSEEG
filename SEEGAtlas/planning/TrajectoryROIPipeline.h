#ifndef __TRAJECTORY_ROI_PIPELINE__
#define __TRAJECTORY_ROI_PIPELINE__

/**
 * @file TrajectoryROIPipeline.h
 *
 * Defines an itk pipeline to create a 3D volume mask (e.g. cylinder of interest) for
 * a specific trajectory
 *
 * @author Silvain Beriault
 */

// Header files to include
#include "BasicTypes.h"
#include "MathUtils.h"
#include "VolumeTypes.h"
#include <string>
#include "itkBinaryThresholdImageFilter.h"
#include "DistanceFromTrajectoryImageFilter.h"


using namespace std;

namespace seeg {


    /**
     * This class contains a small itk pipeline to create a 3D binary mask (e.g. cylinder of interest)
     *  that includes all neighbouring voxels of a linear trajectory (within a specific radius)
     */
    class TrajectoryROIPipeline {

    public:

        /** SmartPointer type for the TrajectoryROIPipeline class */
        typedef mrilSmartPtr<TrajectoryROIPipeline> Pointer;

        /**
         * Create a new instance of TrajectoryROIPipeline
         *
         * @param templateVolume A FloatVolume used as template (for dimension, spacing etc)
         * @param targetPointWorld The location of the trajectorie's target point in world coordinates
         */
        static Pointer New(FloatVolume::Pointer templateVolume, Point3D targetPointWorld) {
            return Pointer(new TrajectoryROIPipeline(templateVolume, targetPointWorld));
        }

        /**
         * Create a new instance of TrajectoryROIPipeline
         *
         * @param templateVolume An IntVolume used as template (for dimension, spacing etc)
         * @param targetPointWorld The location of the trajectorie's target point in world coordinates
         */
        static Pointer New(IntVolume::Pointer templateVolume, Point3D targetPointWorld) {
            return Pointer(new TrajectoryROIPipeline(templateVolume, targetPointWorld));
        }

        /**
         * Create a new instance of TrajectoryROIPipeline
         *
         * @param templateVolume An ByteVolume used as template (for dimension, spacing etc)
         * @param targetPointWorld The location of the trajectorie's target point in world coordinates
         */
        static Pointer New(ByteVolume::Pointer templateVolume, Point3D targetPointWorld) {
            return Pointer(new TrajectoryROIPipeline(templateVolume, targetPointWorld));
        }



        /**
         * Create a new instance of TrajectoryROIPipeline
         *
         * @param templateVolumeFile Filename for the template volume (used for dimension, spacing, etc)
         * @param targetPointWorld The location of the trajectorie's target point in world coordinates
         */
        static Pointer New(const string& templateVolumeFile, Point3D targetPointWorld) {
            return Pointer(new TrajectoryROIPipeline(templateVolumeFile, targetPointWorld));
        }

    protected:

        /**
         * Constructor
         *
         * @param templateVolume A FloatVolume used as template (for dimension, spacing etc)
         * @param targetPointWorld The location of the trajectorie's target point in world coordinates
         */
        TrajectoryROIPipeline (FloatVolume::Pointer templateVolume, Point3D targetPointWorld);

        /**
         * Constructor
         *
         * @param templateVolume An IntVolume used as template (for dimension, spacing etc)
         * @param targetPointWorld The location of the trajectorie's target point in world coordinates
         */
        TrajectoryROIPipeline (IntVolume::Pointer templateVolume, Point3D targetPointWorld);

        /**
         * Constructor
         *
         * @param templateVolume A ByteVolume used as template (for dimension, spacing etc)
         * @param targetPointWorld The location of the trajectorie's target point in world coordinates
         */
        TrajectoryROIPipeline (ByteVolume::Pointer templateVolume, Point3D targetPointWorld);


        /**
         * Constructor
         *
         * @param templateVolumeFile Filename for the template volume (used for dimension, spacing, etc)
         * @param targetPointWorld The location of the trajectorie's target point in world coordinates
         */
        TrajectoryROIPipeline (const string& templateVolumeFile, Point3D targetPointWorld);

    public:

        /**
         * Destructor
         */
        virtual ~TrajectoryROIPipeline();


        /**
         * Setter for the target point
         *
         * @param targetPointWorld The new target point
         */
        void SetTargetPoint (Point3D targetPointWorld);


        void CalcDistanceMap( Point3D entryPointWorld,
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

        /** Voxel coordinates of the target point */
        FloatVolume::IndexType m_TargetPoint;
    };
}

#endif
