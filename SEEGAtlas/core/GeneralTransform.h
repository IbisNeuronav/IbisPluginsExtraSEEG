#ifndef __GENERAL_TRANSFORM_H__
#define __GENERAL_TRANSFORM_H__

/**
 * @file GeneralTransform.h
 *
 * Wrapper class for the vtkTransform framework
 * This  code was never tested !!!!!!!!!!!!!!!!!!
 * The conditions that would activate it is never met in SEEGPathPlanning (Anka, 2014/09/22)
 *
 * @author Silvain Beriault
 * completely modified to use vtk by Anka
 */

// Header files to include
#include <string>
#include "MathUtils.h"
#include "VolumeTypes.h"
#include "BasicTypes.h"

class vtkTransform;

namespace seeg {


    /**
     * The GeneralTransform is a wrapper class for the vtkTransform's framework
     */
    class GeneralTransform {

    public:

        /** Defines a smart pointer for the GeneralTransform object */
        typedef mrilSmartPtr<GeneralTransform> Pointer;

        /**
         * Creates a new instance of the GeneralTransform class
         *
         * @param xfmfile Reference to a transform (.xfm) file
         * @param invert true to invert the transformation, false otherwise
         * @return a smart pointer to the new GeneralTransform instance
         */
        static GeneralTransform::Pointer New(const string& xfmfile, bool invert = false) {
            return Pointer(new GeneralTransform(xfmfile, invert));
        }

        /**
         * Destructor
         */
        virtual ~GeneralTransform();

        /**
         * Transform a 3D point
         *
         * @param pointIn World coordinates of the point
         * @param pointOut World coordinates of the transformed point
         */
        void TransformPoint(const Point3D& pointIn, Point3D& pointOut);

        /**
         * Transform a 3D point (invert transform)
         *
         * @param pointIn World coordinates of the point
         * @param pointOut World coordinates of the transformed point
         */
        void TransformPointInv(const Point3D& pointIn, Point3D& pointOut);

    protected:
        /**
         * Protected constructor
         *
         * @param xfmfile Reference to a transform (.xfm) file
         * @param invert true to invert the transformation, false otherwise
         */
        GeneralTransform(const string& xfmfile, bool invert);

    private:
        /** transform  */
        vtkTransform * m_GeneralTransform;

        /** inverted transform*/
        vtkTransform * m_GeneralTransformInv;
    };

}

#endif
