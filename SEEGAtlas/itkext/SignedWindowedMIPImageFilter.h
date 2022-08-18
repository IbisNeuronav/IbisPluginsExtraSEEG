#ifndef __SIGNED_WINDOWED_MIP_IMAGE_FILTER_H__
#define __SIGNED_WINDOWED_MIP_IMAGE_FILTER_H__

/**
 * @file SignedWindowedMIPImageFilter.h
 *
 * Definition of the SignedWindowedMIPImageFilter class
 *
 * @author Silvain Beriault
 */

#include "itkImageToImageFilter.h"
#include "MathUtils.h"

using namespace itk;

namespace mril {

    /**
     * A subclass of itk::ImageToImageFilter which creates a "windowed" maximum
     * intensity projection for a signed dataset (i.e. the value kept is the one
     * with the maximal absolute value).
     */
    template <typename TImageType3D>
    class ITK_EXPORT SignedWindowedMIPImageFilter : public ImageToImageFilter <TImageType3D,TImageType3D> {

    public:

        // itk types
        typedef SignedWindowedMIPImageFilter Self;
        typedef ImageToImageFilter<TImageType3D, TImageType3D> Superclass;
        typedef SmartPointer<Self> Pointer;
        typedef SmartPointer<const Self> ConstPointer;

        // convenient types
        typedef typename TImageType3D::ConstPointer ConstImagePointerType;
        typedef typename TImageType3D::Pointer ImagePointerType;
        typedef typename TImageType3D::RegionType RegionType;
        typedef typename TImageType3D::PixelType PixelType;
        typedef typename TImageType3D::IndexType IndexType;
        typedef typename TImageType3D::SizeType SizeType;

        /** enumeration for the MIP direction */
        enum MIP_DIR {TRANSVERSE, SAGITTAL, CORONAL};


        //itk macros
        itkNewMacro(Self);
        itkTypeMacro(WindowedMIPImageFilter, ImageToImageFilter);

        // itk getters and setters
        itkSetMacro(WindowSize, int);
        itkSetMacro(Direction, MIP_DIR);
        itkSetMacro(UseMask, bool);

        itkGetMacro(WindowSize, int);
        itkGetMacro(Direction, MIP_DIR);
        itkGetMacro(UseMask, bool);

#if ITK_VERSION_MAJOR > 3
        itkSetInputMacro(MaskInput, IntVolume);
        itkGetInputMacro(MaskInput, IntVolume);
#else
        itkSetInputMacro(Mask, IntVolume, 1);
        itkGetInputMacro(Mask, IntVolume, 1);
#endif

    protected:

        /** The window size (the number of slices to used for each windowed MIP */
        int m_WindowSize;

        /** The MIP direction */
        MIP_DIR m_Direction;

        /** Flag to tell whether or not a mask file is provided */
        bool m_UseMask;


        /**
         * Constructor
         */
        SignedWindowedMIPImageFilter();

        // should probably implement
//        void PrintSelf( std::ostream &os, Indent indent) const;

        /**
         * Implementation of superclass's GenerateData()
         */
        void GenerateData();

    };
}

#if ITK_TEMPLATE_TXX
# include "SignedWindowedMIPImageFilter.txx"
#endif

#endif
