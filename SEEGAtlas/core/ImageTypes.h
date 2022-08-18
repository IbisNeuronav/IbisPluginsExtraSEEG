#ifndef __IMAGE_TYPES_H__
#define __IMAGE_TYPES_H__




// header file to include
#include "itkImage.h"
#include "itkImageRegionIterator.h"
#include "itkImageRegionIteratorWithIndex.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkCastImageFilter.h"
#include "itkImageDuplicator.h"

namespace mril {

    /**** types related to a FloatImage ****/

    /** Type used to represent a 2D float image */
    typedef itk::Image<float, 2> FloatImage;
    /** Type used to read a FloatImage */
    typedef itk::ImageFileReader<FloatImage> FloatImageReader;
    /** Type used to write a FloatImage */
    typedef itk::ImageFileWriter<FloatImage> FloatImageWriter;
    /** Type used to iterate thru a FloatImage using an ImageRegionInteratorWithIndex (slower) */
    typedef itk::ImageRegionIteratorWithIndex<FloatImage> FloatImageRegionIteratorWithIndex;
    /** Type used to iterate thru a FloatImage using an ImageRegionInterator (faster) */
    typedef itk::ImageRegionIterator<FloatImage> FloatImageRegionIterator;


    /**** types related to a 16-bit integer volume ****/


    typedef itk::Image<unsigned short, 2> IntImage;

    typedef itk::ImageFileReader<IntImage> IntImageReader;

    typedef itk::ImageFileWriter<IntImage> IntImageWriter;

    typedef itk::ImageRegionIteratorWithIndex<IntImage> IntImageRegionIteratorWithIndex;

    typedef itk::ImageRegionIterator<IntImage> IntImageRegionIterator;



    typedef itk::Image<unsigned char, 2> ByteImage;

    typedef itk::ImageFileReader<ByteImage> ByteImageReader;

    typedef itk::ImageFileWriter<ByteImage> ByteImageWriter;

    typedef itk::ImageRegionIteratorWithIndex<ByteImage> ByteImageRegionIteratorWithIndex;

    typedef itk::ImageRegionIterator<ByteImage> ByteImageRegionIterator;

}

#endif
