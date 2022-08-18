#ifndef __ITK_UTILS_H__
#define __ITK_UTILS_H__

// General purpose ITK utilities

#include "VolumeTypes.h"
#include <vector>
#include <string>

namespace seeg {

    // if label < 0 then compute a gaussian model for any non-zero voxel in label vol.
    // otherwise estimate gaussian model for the specified label in labelVol
    void EstimateGaussianModel( FloatVolume::ConstPointer dataVol, // input
                                ByteVolume::ConstPointer labelVol,  // input
                                FloatVolume::RegionType region,    // input
                                double& mean,                      // output
                                double& stdDev,                    // output
                                int& voxelCnt,                     // output
                                int label=-1);                     // optional input



    int IntVolumeHistogram (IntVolume::Pointer data, std::vector<int>& histogram);
    int IntVolumeHistogram (IntVolume::Pointer data, ByteVolume::Pointer mask, std::vector<int>& histogram);

    void SaveHistogramToFile (const std::string& filename, std::vector<int>& histogram);



    FloatVolume::Pointer AddTwoFloatVolumes (FloatVolume::Pointer vol1, FloatVolume::Pointer vol2);
    IntVolume::Pointer InvertBinaryVolume (IntVolume::Pointer vol);


    FloatVolume::Pointer CreateRectangleKernel(unsigned int radius[3]);
    FloatVolume::Pointer CreateSquareKernel(unsigned int radius);
    FloatVolume::Pointer CreateSphereKernel(unsigned int radius);


    float SumVolume(FloatVolume::Pointer vol);
    int SumVolume(IntVolume::Pointer vol);

};



#endif
