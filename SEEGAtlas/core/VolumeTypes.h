#ifndef __VOLUME_TYPES_H__
#define __VOLUME_TYPES_H__

/**
 * @file VolumeTypes.h
 *
 * This files defines convenient itk volume types.
 *
 * @author Silvain Beriault
 */

// header file to include
#include "itkImage.h"
#include "itkImageRegionIterator.h"
#include "itkImageRegionConstIterator.h"
#include "itkImageRegionIteratorWithIndex.h"
#include "itkImageRegionConstIteratorWithIndex.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkCastImageFilter.h"
#include "itkImageDuplicator.h"
#include "itkRGBAPixel.h"
#include <string>

namespace seeg {

    /**** types related to a FloatVolume ****/
    typedef itk::Image<float, 3> FloatVolume;
    typedef itk::ImageRegionIteratorWithIndex<FloatVolume> FloatVolumeRegionIteratorWithIndex;
    typedef itk::ImageRegionConstIteratorWithIndex<FloatVolume> FloatVolumeRegionConstIteratorWithIndex;
    typedef itk::ImageRegionIterator<FloatVolume> FloatVolumeRegionIterator;
    typedef itk::ImageRegionConstIterator<FloatVolume> FloatVolumeRegionConstIterator;


    /**** types related to a 16-bit integer volume ****/
    typedef itk::Image<unsigned short, 3> IntVolume;
    typedef itk::ImageRegionIteratorWithIndex<IntVolume> IntVolumeRegionIteratorWithIndex;
    typedef itk::ImageRegionConstIteratorWithIndex<IntVolume> IntVolumeRegionConstIteratorWithIndex;
    typedef itk::ImageRegionIterator<IntVolume> IntVolumeRegionIterator;
    typedef itk::ImageRegionConstIterator<IntVolume> IntVolumeRegionConstIterator;

    /**** types related to a 8-bit byte volume ****/
    typedef itk::Image<unsigned char, 3> ByteVolume;
    typedef itk::ImageRegionIteratorWithIndex<ByteVolume> ByteVolumeRegionIteratorWithIndex;
    typedef itk::ImageRegionConstIteratorWithIndex<ByteVolume> ByteVolumeRegionConstIteratorWithIndex;
    typedef itk::ImageRegionIterator<ByteVolume> ByteVolumeRegionIterator;
    typedef itk::ImageRegionConstIterator<ByteVolume> ByteVolumeRegionConstIterator;


    /** Type used to represent a 3-channel and 4-channel RGB(A) volume */
    typedef itk::RGBPixel<unsigned char> RGB8Pixel;
    typedef itk::RGBAPixel<unsigned char> RGBA8Pixel;
    typedef itk::Image<RGB8Pixel,3> RGBVolume;
    typedef itk::Image<RGBA8Pixel,3> RGBAVolume;
    typedef itk::ImageRegionIteratorWithIndex<RGBVolume> RGBVolumeRegionIteratorWithIndex;
    typedef itk::ImageRegionIterator<RGBVolume> RGBVolumeRegionIterator;
    typedef itk::ImageRegionIteratorWithIndex<RGBAVolume> RGBAVolumeRegionIteratorWithIndex;
    typedef itk::ImageRegionIterator<RGBAVolume> RGBAVolumeRegionIterator;

    /**** types related to a FloatVectorVolume ****/
    /** added by RIZ 20130405 to be able to read/write volumes containing vectors of float (e.g. with normals) **/
    typedef itk::Image<float, 4> FloatVectorVolume; //has 1 more dimension than FloatVolume -> first dim is vector dim
    typedef itk::ImageRegionIteratorWithIndex<FloatVectorVolume> FloatVectorVolumeRegionIteratorWithIndex;
    typedef itk::ImageRegionConstIteratorWithIndex<FloatVectorVolume> FloattVectorVolumeRegionConstIteratorWithIndex;
    typedef itk::ImageRegionIterator<FloatVectorVolume> FloattVectorVolumeRegionIterator;
    typedef itk::ImageRegionConstIterator<FloatVectorVolume> FloattVectorVolumeRegionConstIterator;

    // Defines a 3D point in world coordinates
    typedef itk::Point<double, 3> Point3D;

    /**** Cast filters ****/
    typedef itk::CastImageFilter< FloatVolume, IntVolume > FloatToIntVolumeFilter;
    typedef itk::CastImageFilter< IntVolume, FloatVolume > IntToFloatVolumeFilter;
    typedef itk::CastImageFilter< ByteVolume, FloatVolume > ByteToFloatVolumeFilter;
    typedef itk::CastImageFilter< ByteVolume, IntVolume > ByteToIntVolumeFilter;
    typedef itk::CastImageFilter< FloatVolume, ByteVolume > FloatToByteVolumeFilter;


    /***** Image duplicatior ******/
    typedef itk::ImageDuplicator<FloatVolume> FloatVolumeDuplicator;
    typedef itk::ImageDuplicator<IntVolume> IntVolumeDuplicator;
    typedef itk::ImageDuplicator<ByteVolume> ByteVolumeDuplicator;


    FloatVolume::Pointer ReadFloatVolume(const std::string& filename);
    IntVolume::Pointer ReadIntVolume(const std::string& filename);
    ByteVolume::Pointer ReadByteVolume(const std::string& filename);

    FloatVectorVolume::Pointer ReadFloatVectorVolume(const std::string& filename);//RIZ 20130405 added FloatVector type

    void WriteFloatVolume(const std::string& filename, FloatVolume::Pointer vol);
    void WriteIntVolume(const std::string& filename, IntVolume::Pointer vol);
    void WriteByteVolume(const std::string& filename, ByteVolume::Pointer vol);

    void WriteFloatVectorVolume(const std::string& filename, FloatVectorVolume::Pointer vol); //RIZ 20130405 added FloatVector type

    FloatVolume::Pointer CopyFloatVolume(FloatVolume::Pointer vol);
    IntVolume::Pointer CopyIntVolume(IntVolume::Pointer vol);
    ByteVolume::Pointer CopyByteVolume(ByteVolume::Pointer vol);


    IntVolume::Pointer FloatToIntVolume(FloatVolume::ConstPointer vol);
    FloatVolume::Pointer IntToFloatVolume(IntVolume::ConstPointer vol);
    IntVolume::Pointer FloatToIntVolume(FloatVolume::Pointer vol);
    FloatVolume::Pointer IntToFloatVolume(IntVolume::Pointer vol);

}

#endif
