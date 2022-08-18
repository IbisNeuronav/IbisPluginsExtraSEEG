#ifndef __DISTANCE_FROM_TRAJECTORY_IMAGE_FILTER_TXX__
#define __DISTANCE_FROM_TRAJECTORY_IMAGE_FILTER_TXX__

/**
 * @file DistanceFromTrajectoryImageFilter.txx
 *
 * Implementation of the template class DistanceFromTrajectoryImageFilter
 *
 * @author Silvain Beriault
 */

// Header files to include
#include "DistanceFromTrajectoryImageFilter.h"
#include <math.h>
#include <iostream>

using namespace std;
using namespace itk;

namespace seeg {

    /**** CONSTRUCTOR / DESTRUCTOR ****/

    template <typename TInputImage3D, typename TOutputImage3D>
    DistanceFromTrajectoryImageFilter<TInputImage3D, TOutputImage3D>::DistanceFromTrajectoryImageFilter() {

    }


    /**** IMPL OF GenerateData() ****/

    template <typename TInputImage3D, typename TOutputImage3D>
    void DistanceFromTrajectoryImageFilter<TInputImage3D, TOutputImage3D>::GenerateData() {

        // this function probably needs some commenting...

        this->AllocateOutputs();
        OutImagePointerType outputImage = this->GetOutput();
        OutRegionType outputRegion = outputImage->GetRequestedRegion();
        InImagePointerType inputImage = this->GetInput();

        InSpacingType spacing = inputImage->GetSpacing();

        Vector3D_f scale = Vector3D_f(spacing[0], spacing[1], spacing[2]);
        Vector3D_f p1 = Vector3D_f(m_EntryPoint[0], m_EntryPoint[1], m_EntryPoint[2]);
        Vector3D_f p2 = Vector3D_f(m_TargetPoint[0], m_TargetPoint[1], m_TargetPoint[2]);

        p1 = p1.DotMult(scale);
        p2 = p2.DotMult(scale);

        float dist_p1p2 = norm(p2-p1);

        typedef ImageRegionIteratorWithIndex< TOutputImage3D > ImageIterator;
        ImageIterator outputIterator( outputImage, outputRegion );

        outputIterator.GoToBegin();

        while( ! outputIterator.IsAtEnd() ) {

            OutIndexType voxelIndex = outputIterator.GetIndex();
            Vector3D_f p0 (voxelIndex[0], voxelIndex[1], voxelIndex[2]);
            p0 = p0.DotMult(scale);

            float dist_p0p1 = norm(p1-p0);
            float dist_p0p2 = norm(p2-p0);
            float d = (norm((p2-p1)*(p1-p0)) / dist_p1p2);

             // Look for special cases: intersection point is outside line segment
             float t = sqrt(pow(max(dist_p0p1, dist_p0p2),2) - pow(d,2));
             if (t > dist_p1p2) {
                 d = min(dist_p0p1, dist_p0p2);
             }

             outputIterator.Set( d );
             ++outputIterator;
        }
    }
};

#endif
