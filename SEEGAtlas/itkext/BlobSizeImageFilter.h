#ifndef __BLOB_SIZE_IMAGE_FILTER_H__
#define __BLOB_SIZE_IMAGE_FILTER_H__


#include "itkImageToImageFilter.h"
#include "MathUtils.h"

#define MAX_BLOB 256

using namespace itk;

namespace mril {


    template <typename TImageType>
    class ITK_EXPORT BlobSizeImageFilter : public ImageToImageFilter <TImageType, TImageType> {

    public:

        // itk types
        typedef BlobSizeImageFilter Self;
        typedef ImageToImageFilter<TImageType, TImageType> Superclass;
        typedef SmartPointer<Self> Pointer;
        typedef SmartPointer<const Self> ConstPointer;

        // convenient types
        typedef typename TImageType::ConstPointer ConstImagePointerType;
        typedef typename TImageType::Pointer ImagePointerType;
        typedef typename TImageType::RegionType RegionType;
        typedef typename TImageType::PixelType PixelType;
        typedef typename TImageType::IndexType IndexType;
        typedef typename TImageType::SizeType SizeType;


        //itk macros
        itkNewMacro(Self);
        itkTypeMacro(BlobSizeImageFilter, ImageToImageFilter);

        // itk getters and setters
        itkSetMacro(MaxBlobSize, int);
        itkSetMacro(MinBlobSize, int);

    protected:


        int m_MaxBlobSize;
        int m_MinBlobSize;

        int m_BlobCnt[MAX_BLOB];

        int m_FilteredBlobList[MAX_BLOB];
        int m_NumBlobs;


        /**
         * Constructor
         */
        BlobSizeImageFilter();

        // should probably implement
//        void PrintSelf( std::ostream &os, Indent indent) const;

        /**
         * Implementation of superclass's GenerateData()
         */
        void GenerateData();

    };
}

#if ITK_TEMPLATE_TXX
# include "BlobSizeImageFilter.txx"
#endif

#endif
