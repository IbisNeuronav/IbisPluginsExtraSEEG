#ifndef __SEEG_ROI_PIPELINE__
#define __SEEG_ROI_PIPELINE__

/**
 * @file SEEGROIPipeline.h
 *
 * Basic class from which SEEGTrajectoryROIPipeline.h
 * and SEEGContactsROIPipeline derive
 *
 * @author Silvain Beriault & Rina Zelmann
 */

// Header files to include
#include "BasicTypes.h"
#include "MathUtils.h"
#include "VolumeTypes.h"
#include <string>


using namespace std;
using namespace itk;

namespace mril {


    /**
     * This class contains a small itk pipeline to create a 3D binary mask (e.g. cylinder of interest)
     *  that includes all neighbouring voxels of a linear trajectory (within a specific radius)
     */
    class SEEGROIPipeline {

    public:

        /** SmartPointer type for the SEEGROIPipeline class */
        typedef mrilSmartPtr<SEEGROIPipeline> Pointer;

        /**
         * Create a new instance of SEEGROIPipeline
         *
         * @param templateVolume A FloatVolume used as template (for dimension, spacing etc)
         * @param targetPointWorld The location of the trajectorie's target point in world coordinates
         */
        static Pointer New(FloatVolume::Pointer templateVolume) {
            return Pointer(new SEEGROIPipeline(templateVolume));
        }

        /**
         * Create a new instance of SEEGROIPipeline
         *
         * @param templateVolume An IntVolume used as template (for dimension, spacing etc)
         * @param targetPointWorld The location of the trajectorie's target point in world coordinates
         */
        static Pointer New(IntVolume::Pointer templateVolume) {
            return Pointer(new SEEGROIPipeline(templateVolume));
        }

        /**
         * Create a new instance of SEEGROIPipeline
         *
         * @param templateVolume An ByteVolume used as template (for dimension, spacing etc)
         * @param targetPointWorld The location of the trajectorie's target point in world coordinates
         */
        static Pointer New(ByteVolume::Pointer templateVolume) {
            return Pointer(new SEEGROIPipeline(templateVolume));
        }



        /**
         * Create a new instance of SEEGROIPipeline
         *
         * @param templateVolumeFile Filename for the template volume (used for dimension, spacing, etc)
         * @param targetPointWorld The location of the trajectorie's target point in world coordinates
         */
        static Pointer New(const string& templateVolumeFile) {
            return Pointer(new SEEGROIPipeline(templateVolumeFile));
        }

    protected:

        /**
         * Constructor
         *
         * @param templateVolume A FloatVolume used as template (for dimension, spacing etc)
         * @param targetPointWorld The location of the trajectorie's target point in world coordinates
         */
        SEEGROIPipeline (FloatVolume::Pointer templateVolume);

        /**
         * Constructor
         *
         * @param templateVolume An IntVolume used as template (for dimension, spacing etc)
         * @param targetPointWorld The location of the trajectorie's target point in world coordinates
         */
        SEEGROIPipeline (IntVolume::Pointer templateVolume);

        /**
         * Constructor
         *
         * @param templateVolume A ByteVolume used as template (for dimension, spacing etc)
         * @param targetPointWorld The location of the trajectorie's target point in world coordinates
         */
        SEEGROIPipeline (ByteVolume::Pointer templateVolume);


        /**
         * Constructor
         *
         * @param templateVolumeFile Filename for the template volume (used for dimension, spacing, etc)
         * @param targetPointWorld The location of the trajectorie's target point in world coordinates
         */
        SEEGROIPipeline (const string& templateVolumeFile);

    public:

        /**
         * Destructor
         */
        virtual ~SEEGROIPipeline();

     //   FloatVolume::Pointer GetTemplateVol();
     //   void SetTemplateVol(FloatVolume::Pointer templateVol);


    protected:

        /** Pointer to the template volume */
        FloatVolume::Pointer m_TemplateVolume; // for volume dimension, spacing, etc

  private:
        void InitPipeline();


    };
}

#endif
