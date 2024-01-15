#ifndef BIPOLARCHANNELMODEL_H
#define BIPOLARCHANNELMODEL_H

#include "VolumeTypes.h"
#include "BasicTypes.h"
#include "FileUtils.h"
#include "MathUtils.h"


#include "itkSpatialObjectToImageFilter.h"
#include "itkEllipseSpatialObject.h"
#include "itkTubeSpatialObject.h"
#include "itkGroupSpatialObject.h"

namespace seeg {

    /** class BipolarChannelModel
     * The Channel's volume is modeled as a cilinder between contacts + 2 half spheres.
     * The length equals the distance between the outer tips of the two electrodes and 10mm diameter
     * and one half sphere at each end also of 10mm diameter.
     */

    class BipolarChannelModel
    {
    public:
        /** Defines a smart pointer type for the BipolarChannelModel class */
        typedef mrilSmartPtr<BipolarChannelModel> Pointer;

        /**  To locate channel model in space we need 2 points */
        Point3D m_Contact1Center;
        Point3D m_Contact2Center;
        FloatVolume::SizeType m_VolSize;
        FloatVolume::SpacingType m_VolSpacing;


        static Pointer New() {
            return Pointer(new BipolarChannelModel());
        }


    //    static Pointer New(const int cylinderHeight, const int cylinderRadius, const int sphereRadius) {
    //        return Pointer(new BipolarChannelModel(cylinderHeight, cylinderRadius, sphereRadius, 1)); //Radius & Height in mm*spacing
    //    }

        static Pointer New(Point3D point1, Point3D point2, const int cylinderRadius, const int sphereRadius) {
            return Pointer(new BipolarChannelModel(point1, point2, cylinderRadius, sphereRadius, 1)); //Radius & Height in mm*spacing
        }

    protected:
        BipolarChannelModel();

    //    BipolarChannelModel(const double cylinderHeight, const double cylinderRadius, const double sphereRadius, const int insideValue);

        BipolarChannelModel(Point3D contact1Center, Point3D contact2Center, const double cylinderRadius, const double sphereRadius, const int insideValue);

    private:

        typedef itk::EllipseSpatialObject< 3 >   EllipseType;
        typedef itk::TubeSpatialObject< 3 >       TubeType;
        typedef itk::GroupSpatialObject< 3 >     GroupType;
        typedef itk::SpatialObjectToImageFilter<GroupType, FloatVolume >   SpatialObjectToImageFilterType;

        /** Define a Model variable that groups all the shapes and will be the one accessed */
        GroupType::Pointer m_Model;

        /** Define a Model FloatVolume variable which is an image that contains 1 inside the model and 0 outside*/
        FloatVolume::Pointer m_VolModel;

        //Functions
    public:
        //Getters & Setters
        GroupType::Pointer GetBipolarChannelModel() {
            // returns Group Spatial Object that groups cylinder and spheres
            return m_Model;
        }

        FloatVolume::Pointer GetBipolarChannelVolume() {
            // returns FloatVolume Image that contains the model (cylinder and spheres) as ones and background as zeros
            return m_VolModel;
        }

        Point3D GetContact1Point(){
            return m_Contact1Center;
        }

        Point3D GetContact2Point(){
            return m_Contact2Center;
        }

        void SetContact1Point(Point3D point){
            m_Contact1Center = point;
        }

        void SetContact2Point(Point3D point){
            m_Contact2Center = point;
        }

        // Functions
        void PositionModel(); // uses the position of m_Contact1Center and m_Contact2Center

        void PositionModel(Point3D point1, Point3D point2); //uses specified points

        void GenerateImage();

        void GenerateImage(FloatVolume::Pointer templateVolume, FloatVolume::RegionType channelRegion);

        int GetNumberVoxelsInChannel();

    };
}
#endif // BIPOLARCHANNELMODEL_H
