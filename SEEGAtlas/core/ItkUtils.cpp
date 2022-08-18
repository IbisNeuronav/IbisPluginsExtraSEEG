#include "ItkUtils.h"
#include "itkAddImageFilter.h"
#include <iostream>
#include <fstream>

using namespace std;

namespace seeg {

    void EstimateGaussianModel( FloatVolume::ConstPointer dataVol,
                                ByteVolume::ConstPointer labelVol,
                                FloatVolume::RegionType region,
                                double& mean,
                                double& stdDev,
                                int& voxelCnt,
                                int label) {
        FloatVolumeRegionConstIterator dataIt = FloatVolumeRegionConstIterator(dataVol, region);
        ByteVolumeRegionConstIterator labelIt = ByteVolumeRegionConstIterator(labelVol, region);
        voxelCnt = 0;
        mean = 0;

        // calculate mean
        for (dataIt.GoToBegin(), labelIt.GoToBegin(); !dataIt.IsAtEnd(); ++dataIt, ++labelIt) {
            unsigned char l = labelIt.Get();
            if ( (label<0 && l!=0) || (l==label) ) {
                voxelCnt++;
                // provisional mean! (otherwise, may get numerical instabilities for large regions
                mean += (dataIt.Get() - mean)/voxelCnt;
            }
        }

        // calculate std dev
        double sum = 0;
        for (dataIt.GoToBegin(), labelIt.GoToBegin(); !dataIt.IsAtEnd(); ++dataIt, ++labelIt) {
            unsigned char l = labelIt.Get();
            if ( (label<0 && l!=0) || (l==label) ) {
                double data = dataIt.Get();
                sum+=(data-mean)*(data-mean);
            }
        }
        stdDev = voxelCnt>0 ? sqrt(sum/voxelCnt) : 0;

    }

    int IntVolumeHistogram (IntVolume::Pointer data, std::vector<int>& histogram) {
        return IntVolumeHistogram(data, ByteVolume::Pointer(), histogram);
    }

    int IntVolumeHistogram (IntVolume::Pointer data, ByteVolume::Pointer mask, std::vector<int>& histogram) {

        int cnt = 0;
        // find max value (min value is assumed to be 0)!

        histogram.clear();

        bool useMask = false;
        if (mask.IsNotNull()) {
            useMask = true;
        }


        IntVolumeRegionIterator dataIt(data, data->GetRequestedRegion());
        ByteVolumeRegionIterator maskIt;

        if (useMask) {
            maskIt = ByteVolumeRegionIterator(mask, data->GetRequestedRegion());
        }

        // find max value
        int max = 0;
        if (useMask) {
            maskIt.GoToBegin();
        }
        for (dataIt.GoToBegin(); !dataIt.IsAtEnd(); ++dataIt) {
            if (!useMask || maskIt.Get()) {
                cnt++;
                int pixel = dataIt.Get();
                if (pixel > max) {
                    max = pixel;
                }
            }

            if (useMask) {
                ++maskIt;
            }
        }

        // histogram count set to zero...
        for (int i=0; i<=max; i++) {
            histogram.push_back(0);
        }

        if (useMask) {
            maskIt.GoToBegin();
        }
        for (dataIt.GoToBegin(); !dataIt.IsAtEnd(); ++dataIt) {
            if (!useMask || maskIt.Get()) {

                int pixel = dataIt.Get();
                if (pixel > max) {
                    histogram[max] = histogram[max] + 1; // should never happen.
                } else if (pixel < 0) {
                    histogram[0] = histogram[0] + 1;
                }
                histogram[pixel] = histogram[pixel] + 1;
            }
            if (useMask) {
                ++maskIt;
            }
        }

        return cnt;
    }

    void SaveHistogramToFile (const string& filename, std::vector<int>& histogram) {
        ofstream file;
        file.open(filename.c_str());
        for (int i=0; i<histogram.size(); i++) {
            file << i << "," <<histogram[i] <<endl;
        }
        file.close();
    }



    FloatVolume::Pointer AddTwoFloatVolumes (FloatVolume::Pointer vol1, FloatVolume::Pointer vol2) {
        typedef itk::AddImageFilter<FloatVolume, FloatVolume, FloatVolume> AddImageFilterType;
        AddImageFilterType::Pointer filt = AddImageFilterType::New();
        filt->SetInput1(vol1);
        filt->SetInput2(vol2);
        filt->Update();
        FloatVolume::Pointer output = filt->GetOutput();
        output->DisconnectPipeline();
        return output;
    }

    IntVolume::Pointer InvertBinaryVolume (IntVolume::Pointer vol) {
        IntVolume::Pointer outputVol = CopyIntVolume(vol);
        IntVolumeRegionIterator it(outputVol, outputVol->GetLargestPossibleRegion());
        for (it.GoToBegin(); !it.IsAtEnd(); ++it) {
            it.Set(!it.Get());
        }
        return outputVol;
    }

    FloatVolume::Pointer CreateRectangleKernel(unsigned int radius[3])
    {

        FloatVolume::Pointer kernel = FloatVolume::New();

        FloatVolume::IndexType start;
        start.Fill(0);

        FloatVolume::SizeType size;
        for (int i=0; i<3; i++) {
            size[i] = 2*radius[i] + 1;
        }

        FloatVolume::RegionType region;
        region.SetSize(size);
        region.SetIndex(start);

        kernel->SetRegions(region);
        kernel->Allocate();

        FloatVolumeRegionIterator imageIterator(kernel, region);

        while(!imageIterator.IsAtEnd()){
           imageIterator.Set(1);
           ++imageIterator;
        }

        return kernel;
    }



    FloatVolume::Pointer CreateSquareKernel(unsigned int radius)
    {

        FloatVolume::Pointer kernel = FloatVolume::New();

        FloatVolume::IndexType start;
        start.Fill(0);

        FloatVolume::SizeType size;
        size.Fill(2*radius+1);

        FloatVolume::RegionType region;
        region.SetSize(size);
        region.SetIndex(start);

        kernel->SetRegions(region);
        kernel->Allocate();

        FloatVolumeRegionIterator imageIterator(kernel, region);

        while(!imageIterator.IsAtEnd()){
           imageIterator.Set(1);
           ++imageIterator;
        }

        return kernel;
    }

    FloatVolume::Pointer CreateSphereKernel(unsigned int radius) {

        FloatVolume::Pointer kernel = FloatVolume::New();

        FloatVolume::IndexType start;
        start.Fill(0);

        FloatVolume::SizeType size;
        size.Fill(2*radius+1);

        FloatVolume::RegionType region;
        region.SetSize(size);
        region.SetIndex(start);

        kernel->SetRegions(region);
        kernel->Allocate();

        FloatVolumeRegionIteratorWithIndex imageIterator(kernel, region);

        FloatVolume::IndexType center;
        center[0]=center[1]=center[2]=radius;

        while(!imageIterator.IsAtEnd()){
            FloatVolume::IndexType index = imageIterator.GetIndex();

            float dist = sqrt((float)pow(index[0]-center[0],2) + (float)pow(index[1]-center[1],2) + (float)pow(index[2]-center[2],2));
            imageIterator.Set(dist <= (radius+0.0001));
            ++imageIterator;
        }
        return kernel;
    }

    float SumVolume(FloatVolume::Pointer vol) {
        FloatVolumeRegionIterator it(vol, vol->GetLargestPossibleRegion());
        float sum = 0;
        for (it.GoToBegin(); !it.IsAtEnd(); ++it ) {
            sum += it.Get();
        }
        return sum;
    }

    int SumVolume(IntVolume::Pointer vol) {
        IntVolumeRegionIterator it(vol, vol->GetLargestPossibleRegion());
        int sum = 0;
        for (it.GoToBegin(); !it.IsAtEnd(); ++it ) {
            sum += it.Get();
        }
        return sum;
    }



};
