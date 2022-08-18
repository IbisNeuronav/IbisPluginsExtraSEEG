#ifndef __DISTANCE_FROM_TRAJECTORY_FILTER_H__
#define __DISTANCE_FROM_TRAJECTORY_FILTER_H__

/**
 * @file DistanceFromTrajectoryImageFilter.h
 *
 * Defines the DistanceFromTrajectoryImageFilter Class
 *
 * @author Silvain Beriault
 */

// Header files to include
#include "itkImageToImageFilter.h"
#include "MathUtils.h"

namespace seeg {

    /**
     * A sub-class of itk::ImageToImageFilter that creates a distance map. This filter
     * takes for input a template volume (for dimension, spacing, etc) and creates an
     * output volume which measure the distance (in mm) of each voxels from a linear
     * trajectory.
     */
    template <typename TInputImage3D, typename TOutputImage3D>
    class ITK_EXPORT DistanceFromTrajectoryImageFilter : public itk::ImageToImageFilter <TInputImage3D, TOutputImage3D> {

    public:

        // itk types
        typedef DistanceFromTrajectoryImageFilter Self;
        typedef itk::ImageToImageFilter<TInputImage3D, TOutputImage3D> Superclass;
        typedef itk::SmartPointer<Self> Pointer;
        typedef itk::SmartPointer<const Self> ConstPointer;

        // other types for convenience
        typedef typename TInputImage3D::ConstPointer InImagePointerType;
        typedef typename TInputImage3D::RegionType InRegionType;
        typedef typename TInputImage3D::IndexType InIndexType;
        typedef typename TInputImage3D::SpacingType InSpacingType;
        typedef typename TOutputImage3D::PixelType OutPixelType;
        typedef typename TOutputImage3D::RegionType OutRegionType;
        typedef typename TOutputImage3D::IndexType OutIndexType;
        typedef typename TOutputImage3D::Pointer OutImagePointerType;

        // itk macro call
        itkNewMacro(Self);
        itkTypeMacro(DistanceFromTrajectoryImageFilter, ImageToImageFilter);

        // itk setters and getters
        itkSetMacro(TargetPoint, InIndexType);
        itkSetMacro(EntryPoint, InIndexType);
        itkGetMacro(TargetPoint, InIndexType);
        itkGetMacro(EntryPoint, InIndexType);


    protected:
        /** Voxel index of the trajectory's target point */
        InIndexType m_TargetPoint;

        /** Voxel index of the trajectory's entry point */
        InIndexType m_EntryPoint;

        /**
         * protected Constructor
         */
        DistanceFromTrajectoryImageFilter();

        // should probably implement this...
//        void PrintSelf( std::ostream &os, Indent indent) const;

        /**
         * Implementation of the sub-classe's GenerateData() function
         */
        void GenerateData();

    };
}

#if ITK_TEMPLATE_TXX
# include "DistanceFromTrajectoryImageFilter.txx"
#endif

#endif
