#ifndef __WINDOWED_MIP_IMAGE_FILTER_H__
#define __WINDOWED_MIP_IMAGE_FILTER_H__

/**
 * @file WindowedMIPImageFilter.h
 *
 * Definition of the WindowedMIPImageFilter class
 *
 * @author Silvain Beriault
 */

#include "itkImageToImageFilter.h"
#include "MathUtils.h"

using namespace itk;

namespace mril {

    /**
     * A subclass of itk::ImageToImageFilter which creates a "windowed" minimum (or maximum)
     * intensity projection (mIP/MIP). This filter takes a 3D volume as input and outputs
     * another 3D volume corresponding to the actual windowed-MIP
     */
    template <typename TImageType3D>
    class ITK_EXPORT WindowedMIPImageFilter : public ImageToImageFilter <TImageType3D,TImageType3D> {

    public:

        // itk types
        typedef WindowedMIPImageFilter Self;
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

        /** enumeration for minimum or maximum intensity projection */
        enum MIP_TYPE {MIN_IP, MAX_IP};

        //itk macros
        itkNewMacro(Self);
        itkTypeMacro(WindowedMIPImageFilter, ImageToImageFilter);

        // itk getters and setters
        itkSetMacro(WindowSize, int);
        itkSetMacro(Direction, MIP_DIR);
        itkSetMacro(MIPType, MIP_TYPE);
        itkSetMacro(UseMask, bool);

        itkGetMacro(WindowSize, int);
        itkGetMacro(Direction, MIP_DIR);
        itkGetMacro(MIPType, MIP_TYPE);
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

        /** minimum or maximum intensity projection */
        MIP_TYPE m_MIPType;

        /** Flag to tell whether or not a mask file is provided */
        bool m_UseMask;


        /**
         * Constructor
         */
        WindowedMIPImageFilter();

        // should probably implement
//        void PrintSelf( std::ostream &os, Indent indent) const;

        /**
         * Implementation of superclass's GenerateData()
         */
        void GenerateData();

    };
}

#if ITK_TEMPLATE_TXX
# include "WindowedMIPImageFilter.txx"
#endif

#endif
