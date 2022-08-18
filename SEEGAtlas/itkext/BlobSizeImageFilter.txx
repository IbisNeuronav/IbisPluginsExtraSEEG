#ifndef __BLOB_SIZE_IMAGE_FILTER_TXX__
#define __BLOB_SIZE_IMAGE_FILTER_TXX__

// Header files to include
#include "BlobSizeImageFilter.h"
#include <math.h>
#include <iostream>
#include "itkImageRegionIteratorWithIndex.h"

using namespace std;
using namespace itk;

namespace mril {

    /***** CONSTRUCTOR/DESTRUCTOR ****/

    template <typename TImageType>
    BlobSizeImageFilter<TImageType>::BlobSizeImageFilter() {
        m_MinBlobSize = 5;
        m_MaxBlobSize = 100;
    }


    /**** IMPLEMENTATION OF SUPERCLASS INTERFACE ****/

    template <typename TImageType>
    void BlobSizeImageFilter<TImageType>::GenerateData() {

        // Allocate output image
        this->AllocateOutputs();

        for (int i=0; i<MAX_BLOB; i++) {
            m_BlobCnt[i] = 0;
        }

        // image pointer and requested region
        ConstImagePointerType inputImage = this->GetInput();
        ImagePointerType outputImage = this->GetOutput();
        RegionType region = outputImage->GetRequestedRegion();

        // iterators for input and output images
        typedef ImageRegionConstIterator<TImageType> InputIterator;
        typedef ImageRegionIterator<TImageType> OutputIterator;
        InputIterator inputIt (inputImage, region);
        OutputIterator outputIt (outputImage, region);

        for (inputIt.GoToBegin(); inputIt.IsAtEnd(); ++inputIt) {
            int pixel = (int)(inputIt.Get());
            if (pixel > 0 && pixel < MAX_BLOB) {
                m_BlobCnt[pixel]++;
            }
        }

        m_NumBlobs = 0;
        for (int i=1; i<MAX_BLOB; i++) {
            if (m_BlobCnt[i] >= m_MinBlobSize && m_BlobCnt[i] <= m_MaxBlobSize) {
                m_FilteredBlobList[m_NumBlobs] = i;
                m_NumBlobs++;
            }
        }

        for (inputIt.GoToBegin(), outputIt.GoToBegin(); inputIt.IsAtEnd(); ++inputIt, ++outputIt) {
            int pixelIn = (int)(inputIt.Get());
            int pixelOut = 0;

            if (pixelIn != 0) {
                for (int i=0; i<m_NumBlobs; i++) {
                    if (pixelIn == m_FilteredBlobList[i]) {
                        pixelOut = i;
                    }
                }
            }
            outputIt.Set(pixelOut);
        }
    }
};

#endif
