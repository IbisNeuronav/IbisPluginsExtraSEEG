#ifndef __FULL_MIP_IMAGE_FILTER_H__
#define __FULL_MIP_IMAGE_FILTER_H__

/**
 * @file FullMIPImageFilter.h
 *
 * Defines the template class FullMIPImageFilter
 *
 * @author Silvain Beriault
 */

// Header file to include
#include "itkImageToImageFilter.h"
#include "MathUtils.h"
#include "VolumeTypes.h"

using namespace itk;

namespace mril {

    /**
     * A subclass of itk::ImageToImageFilter which accept a 3D volume as input and
     * outputs the Maximal Intensity Projection (MIP) of the input volume (2D image).
     */
    template <typename TInputImage3D, typename TOutputImage2D>
    class ITK_EXPORT FullMIPImageFilter : public itk::ImageToImageFilter <TInputImage3D, TOutputImage2D> {

    public:

        // itk types
        typedef FullMIPImageFilter Self;
        typedef itk::ImageToImageFilter<TInputImage3D, TOutputImage2D> Superclass;
        typedef itk::SmartPointer<Self> Pointer;
        typedef itk::SmartPointer<const Self> ConstPointer;

        // Other types for convenience
        typedef typename TInputImage3D::ConstPointer InImagePointerType;
        typedef typename TInputImage3D::RegionType InRegionType;
        typedef typename TInputImage3D::PixelType InPixelType;
        typedef typename TInputImage3D::IndexType InIndexType;
        typedef typename TInputImage3D::SizeType InSizeType;
        typedef typename TInputImage3D::PointType InPointType;
        typedef typename TInputImage3D::SpacingType InSpacingType;
        typedef typename TInputImage3D::DirectionType InDirectionType;

        typedef typename TOutputImage2D::Pointer OutImagePointerType;
        typedef typename TOutputImage2D::RegionType OutRegionType;
        typedef typename TOutputImage2D::PixelType OutPixelType;
        typedef typename TOutputImage2D::IndexType OutIndexType;
        typedef typename TOutputImage2D::SizeType OutSizeType;
        typedef typename TOutputImage2D::PointType OutPointType;
        typedef typename TOutputImage2D::SpacingType OutSpacingType;
        typedef typename TOutputImage2D::DirectionType OutDirectionType;

        /** enumeration for the MIP direction (either transverse, sagittal or coronal) */
        enum MIP_DIR {TRANSVERSE, SAGITTAL, CORONAL};

        // itk macros
        itkNewMacro(Self);
        itkTypeMacro(FullMIPImageFilter, ImageToImageFilter);

        // itk getters and setters
        itkSetMacro(Direction, MIP_DIR);
        itkSetMacro(UseMask, bool);

        itkGetMacro(Direction, MIP_DIR);
        itkGetMacro(UseMask, bool);

#if ITK_VERSION_MAJOR > 3
        itkSetInputMacro(MaskInput, IntVolume);
        itkGetInputMacro(MaskInput, IntVolume);
#else
        itkSetInputMacro(Mask, IntVolume,1);
        itkGetInputMacro(Mask, IntVolume,1);
#endif
    protected:

        /** The direction used to compute the MIP image */
        MIP_DIR m_Direction;

        /** Flag to tell whether a binary mask is provided */
        bool m_UseMask;

        /** Constructor */
        FullMIPImageFilter();

        // maybe we should implement this
//        void PrintSelf( std::ostream &os, Indent indent) const;

        /**
         * Override of the SuperClass's GenerateOutputInformation() since output image
         * is of different format than the input image
         */
        void GenerateOutputInformation();

        /**
         * Override of SuperClass's GenerateInputRequestedRegion() since output image
         * is of different format than input image
         */
        void GenerateInputRequestedRegion();

        /**
         * Implementation of the GenerateData() function.
         */
        void GenerateData();


    private:

        /**
         * Private function to return the projection direction dimension index as well as
         * the other dimension indexes. This calculation is made against the m_Direction
         * attribute.
         *
         * @param otherIndexes an array of size 2 where to write the other 2 dimension indexes
         * @return the dimension index corresponding to the projection direction
         */
        unsigned int GetProjDirectionIndex(unsigned int otherIndexes[2]);

    };
}

#if ITK_TEMPLATE_TXX
# include "FullMIPImageFilter.txx"
#endif

#endif
