#ifndef __SIGNED_WINDOWED_MIP_IMAGE_FILTER_TXX__
#define __SIGNED_WINDOWED_MIP_IMAGE_FILTER_TXX__

/**
 * @file SignedWindowedMIPImageFilter.txx
 *
 * Implementation of the template class SignedWindowedMIPImageFilter
 *
 * @author Silvain Beriault
 */

// Header files to include
#include "SignedWindowedMIPImageFilter.h"
#include <math.h>
#include <iostream>
#include "itkImageRegionIteratorWithIndex.h"
#include "itkImageSliceIteratorWithIndex.h"

using namespace std;
using namespace itk;

namespace mril {

    /***** CONSTRUCTOR/DESTRUCTOR ****/

    template <typename TImageType3D>
    SignedWindowedMIPImageFilter<TImageType3D>::SignedWindowedMIPImageFilter() {
        m_WindowSize = 11;
        m_Direction = TRANSVERSE;
        m_UseMask = false;
    }


    /**** IMPLEMENTATION OF SUPERCLASS INTERFACE ****/

    template <typename TImageType3D>
    void SignedWindowedMIPImageFilter<TImageType3D>::GenerateData() {


        // Allocate output image
        this->AllocateOutputs();

        // image pointer and requested region
        ConstImagePointerType inputImage = this->GetInput();
        IntVolume::ConstPointer maskImage = this->GetMaskInput();
        ImagePointerType outputImage = this->GetOutput();
        RegionType region = outputImage->GetRequestedRegion();

        // iterators for input and output images
        typedef ImageSliceConstIteratorWithIndex<TImageType3D> InputIterator;
        typedef ImageSliceConstIteratorWithIndex<IntVolume> MaskIterator;
        typedef ImageRegionIterator<TImageType3D> OutputIterator;
        InputIterator inputIt (inputImage, region);
        OutputIterator outputIt (outputImage, region);
        MaskIterator maskIt;
        if (m_UseMask) {
            maskIt = MaskIterator(maskImage, region);
        }

        // Determine the MIP direction and call SetFirstDirection(), SetSecondDirection()
        // of the ImageSliceIterator (for the input image)
        unsigned int direction[] = {0,0};
        unsigned int projDirection = 0;
        switch (m_Direction) {
        case TRANSVERSE:
            direction[0] = 0; direction[1] = 1; projDirection = 2;
            break;
        case SAGITTAL:
            direction[0] = 1; direction[1] = 2; projDirection = 0;
            break;
        case CORONAL:
            direction[0] = 0; direction[1] = 2; projDirection = 1;
            break;
        }
        inputIt.SetFirstDirection(direction[0]);
        inputIt.SetSecondDirection(direction[1]);
        if (m_UseMask) {
            maskIt.SetFirstDirection(direction[0]);
            maskIt.SetSecondDirection(direction[1]);
        }


        // first, set everything to zero
        for (outputIt.GoToBegin(); !outputIt.IsAtEnd(); ++outputIt) {
            outputIt.Set(0);
        }

        // then compute the windowed MIP
        inputIt.GoToBegin();
        if (m_UseMask) maskIt.GoToBegin();
        while (!inputIt.IsAtEnd()) {

            // Determine the [indexMin, indexMax] window on the output image
            IndexType index = inputIt.GetIndex();
            unsigned int slice = index[projDirection];
            int indexMin = slice - m_WindowSize/2;
            int indexMax = slice + m_WindowSize/2;
            if (!(m_WindowSize%2)) {
                // special case if m_WindowSize is odd
                indexMin++;
            }

            // Also do some boundary checking to make sure we stay in bound of the
            // requested region
            IndexType start = region.GetIndex();
            SizeType size = region.GetSize();
            if (indexMin < (int) start[projDirection]) {
                indexMin = (int) start[projDirection];
            }
            if (indexMax >= (int) (start[projDirection] + size[projDirection])) {
                indexMax = (int) (start[projDirection] + size[projDirection] - 1);
            }

            while (!inputIt.IsAtEndOfSlice()) {

                // For each line... (unfortunately, with the ImageSliceIterator, it seems
                // we have no choice but to also loop line by line)

                while (!inputIt.IsAtEndOfLine()) {
                    // read the current input pixel index and value
                    index = inputIt.GetIndex();
                    PixelType value = inputIt.Get();
                    if (!m_UseMask || maskIt.Get()) {
                        for (int i=indexMin; i<=indexMax; i++) {
                            IndexType outIndex = index;
                            outIndex[projDirection] = i;
                            PixelType currentVal = outputImage->GetPixel(outIndex);
                            if (abs(value) > abs(currentVal)) {
                                outputImage->SetPixel(outIndex, value);
                            }
                        }
                    }
                    ++inputIt;
                    if (m_UseMask) ++maskIt;
                }
                inputIt.NextLine();
                if (m_UseMask) maskIt.NextLine();
            }
            inputIt.NextSlice();
            if (m_UseMask) maskIt.NextSlice();
        }
    }
};

#endif
