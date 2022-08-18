#ifndef __FULL_MIP_IMAGE_FILTER_TXX__
#define __FULL_MIP_IMAGE_FILTER_TXX__

/**
 * @file FullMIPImageFilter.txx
 *
 * Implementation of the FullMIPImageFilter template class
 *
 * @author Silvain Beriault
 */

// Header files to include
#include "FullMIPImageFilter.h"
#include <math.h>
#include <iostream>
#include "itkImageLinearIteratorWithIndex.h"
#include "itkImageSliceIteratorWithIndex.h"
#include "itkMinimumMaximumImageCalculator.h"

using namespace std;
using namespace itk;

namespace mril {

    /**** CONSTRUCTOR/DESTRUCTOR ****/

    template <typename TInputImage3D, typename TOutputImage2D>
    FullMIPImageFilter<TInputImage3D, TOutputImage2D>::FullMIPImageFilter() {
        m_Direction = TRANSVERSE;
        m_UseMask = false;
    }


    /**** IMPLEMENTATION OF SUPERCLASS INTERFACE ****/

    template< class TInputImage3D, typename TOutputImage2D >
    void FullMIPImageFilter<TInputImage3D, TOutputImage2D>::GenerateOutputInformation() {

        //Superclass::GenerateOutputInformation();

        InImagePointerType  inputImage  = this->GetInput();
        OutImagePointerType outputImage = this->GetOutput();
        InRegionType inputRegion = inputImage->GetLargestPossibleRegion ();
        InIndexType  inputStart  = inputRegion.GetIndex();
        InSizeType   inputSize   = inputRegion.GetSize();

        OutIndexType  outputStart;
        OutSizeType   outputSize;
        unsigned int direction[] = {0,0};
        unsigned int projDirection;

        projDirection = GetProjDirectionIndex(direction);
        outputStart[0] = inputStart[direction[0]];
        outputStart[1] = inputStart[direction[1]];

        outputSize[0] = inputSize[direction[0]];
        outputSize[1] = inputSize[direction[1]];

        OutRegionType outputRegion;
        outputRegion.SetIndex( outputStart );
        outputRegion.SetSize( outputSize );
        outputImage->SetLargestPossibleRegion( outputRegion );

        InSpacingType inSpacing = inputImage->GetSpacing();
        OutSpacingType outSpacing;
        outSpacing[0] = inSpacing[direction[0]];
        outSpacing[1] = outSpacing[direction[1]];
        outputImage->SetSpacing( outSpacing );

        InPointType inOrigin = inputImage->GetOrigin();
        OutPointType outOrigin;
        outOrigin[0] = inOrigin[direction[0]];
        outOrigin[1] = inOrigin[direction[1]];
        outputImage->SetOrigin(outOrigin);

    }


    template<class TInputImage3D, typename TOutputImage2D>
    void FullMIPImageFilter<TInputImage3D, TOutputImage2D>::GenerateInputRequestedRegion() {
        Superclass::GenerateInputRequestedRegion();

        // use the LargestPossibleRegion for this filter.
        typedef typename TInputImage3D::Pointer ImagePointerType;
        ImagePointerType inputImage  = const_cast< TInputImage3D * > (this->GetInput());
        inputImage->SetRequestedRegion(inputImage->GetLargestPossibleRegion());
    }



    template <typename TInputImage3D, typename TOutputImage2D>
    void FullMIPImageFilter<TInputImage3D, TOutputImage2D>::GenerateData() {

        // Allocate output image
        this->AllocateOutputs();

        // image pointer and image region
        InImagePointerType inputImage = this->GetInput();
        IntVolume::ConstPointer maskImage = this->GetMaskInput();
        OutImagePointerType outputImage = this->GetOutput();
        InRegionType inRegion = inputImage->GetLargestPossibleRegion();
        OutRegionType outRegion = outputImage->GetLargestPossibleRegion();

        // iterators for input and output images
        typedef ImageSliceConstIteratorWithIndex<TInputImage3D> InputIterator;
        typedef ImageSliceConstIteratorWithIndex<IntVolume> MaskIterator;
        typedef ImageLinearIteratorWithIndex<TOutputImage2D> OutputIterator;
        InputIterator inputIt (inputImage, inRegion);
        OutputIterator outputIt (outputImage, outRegion);
        MaskIterator maskIt;
        if (m_UseMask) {
            maskIt = MaskIterator(maskImage, inRegion);
        }

        // Determine the MIP direction and call SetFirstDirection(), SetSecondDirection()
        // of the ImageSliceIterator (for the input image)
        unsigned int direction[] = {0,0};
        unsigned int projDirection;
        projDirection = GetProjDirectionIndex(direction);
        inputIt.SetFirstDirection(direction[0]);
        inputIt.SetSecondDirection(direction[1]);
        if (m_UseMask) {
            maskIt.SetFirstDirection(direction[0]);
            maskIt.SetSecondDirection(direction[1]);
        }

        outputIt.SetDirection(0);


        // Set all pixels in output to minimum value
        typedef MinimumMaximumImageCalculator<TInputImage3D> ImageCalculatorFilterType;
        typename ImageCalculatorFilterType::Pointer filt = ImageCalculatorFilterType::New();
        filt->SetImage(inputImage);
        filt->ComputeMinimum();
        outputIt.GoToBegin();
        while (!outputIt.IsAtEnd()) {
            while (!outputIt.IsAtEndOfLine()) {
                outputIt.Set(filt->GetMinimum());
                ++outputIt;
            }
            outputIt.NextLine();
        }

        // for debugging
        //InIndexType inIndex;
        //OutIndexType outIndex;

        // then compute the MIP
        inputIt.GoToBegin();
        outputIt.GoToBegin();
        if (m_UseMask) maskIt.GoToBegin();
        while (!inputIt.IsAtEnd()) {

            // For each slice...
            while (!inputIt.IsAtEndOfSlice()) {

                // For each line... (unfortunately, with the ImageSliceIterator, it seems
                // we have no choice but to also loop line by line)
                while (!inputIt.IsAtEndOfLine()) {

                    if (!m_UseMask || maskIt.Get()) {

                        OutPixelType outVal = outputIt.Get();
                        InPixelType inVal = inputIt.Get();
                        if (inVal > outVal) {
                            outputIt.Set(static_cast<OutPixelType>(inVal));
                        }
                    }
                    ++inputIt;
                    ++outputIt;
                    if (m_UseMask) ++maskIt;
                }
                inputIt.NextLine();
                outputIt.NextLine();
                if (m_UseMask) maskIt.NextLine();
            }
            inputIt.NextSlice();
            if (m_UseMask) maskIt.NextSlice();
            outputIt.GoToBegin();
        }
    }


    /**** PRIVATE FUNCTIONS ****/

    template <typename TInputImage3D, typename TOutputImage2D>
    unsigned int FullMIPImageFilter<TInputImage3D, TOutputImage2D>::GetProjDirectionIndex(unsigned int otherIndexes[2]) {
        unsigned int projDirection;
        switch (m_Direction) {
        case TRANSVERSE:
            otherIndexes[0] = 0; otherIndexes[1] = 1; projDirection = 2;
            break;
        case SAGITTAL:
            otherIndexes[0] = 1; otherIndexes[1] = 2; projDirection = 0;
            break;
        case CORONAL:
            otherIndexes[0] = 0; otherIndexes[1] = 2; projDirection = 1;
            break;
        }
        return projDirection;
    }
};

#endif
