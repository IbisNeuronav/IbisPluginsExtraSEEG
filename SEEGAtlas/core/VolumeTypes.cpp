#include "VolumeTypes.h"
#include "FileUtils.h"

using namespace itk;
using namespace std;

namespace seeg {


    typedef itk::ImageFileReader<FloatVolume> FloatVolumeReader;
    typedef itk::ImageFileWriter<FloatVolume> FloatVolumeWriter;
    typedef itk::ImageFileReader<IntVolume> IntVolumeReader;
    typedef itk::ImageFileWriter<IntVolume> IntVolumeWriter;
    typedef itk::ImageFileReader<ByteVolume> ByteVolumeReader;
    typedef itk::ImageFileWriter<ByteVolume> ByteVolumeWriter;
    typedef itk::ImageFileReader<FloatVectorVolume> FloatVectorVolumeReader; //RIZ 20130405 added FloatVector type
    typedef itk::ImageFileWriter<FloatVectorVolume> FloatVectorVolumeWriter; //RIZ 20130405 added FloatVector type

    FloatVolume::Pointer ReadFloatVolume(const string& filename) {

        // in case not already done
        //RIZ: MINClib are not availablesince IBIS2.2 itk::RegisterMincIO();

        if (!IsFileExists(filename)) {
            cerr << "File " << filename << " does not exist" << endl;
            return FloatVolume::Pointer();
        }
        try {
            FloatVolumeReader::Pointer reader = FloatVolumeReader::New();
            reader->SetFileName(filename);
            reader->Update();
            FloatVolume::Pointer vol = reader->GetOutput();
            vol->DisconnectPipeline();
            return vol;
        } catch (itk::ExceptionObject &excep) {
            cerr << "Failed to read: " << filename << endl;
            cerr << "Exception: " << excep << endl;
            return FloatVolume::Pointer();
        }
    }

    IntVolume::Pointer ReadIntVolume(const string& filename) {

        // in case not already done
        //RIZ: MINClib are not availablesince IBIS2.2 itk::RegisterMincIO();

        if (!IsFileExists(filename)) {
            cerr << "File " << filename << " does not exist" << endl;
            return IntVolume::Pointer();
        }
        try {
            IntVolumeReader::Pointer reader = IntVolumeReader::New();
            reader->SetFileName(filename);
            reader->Update();
            IntVolume::Pointer vol = reader->GetOutput();
            vol->DisconnectPipeline();
            return vol;
        } catch (itk::ExceptionObject &excep) {
            cerr << "Failed to read: " << filename << endl;
            return IntVolume::Pointer();
        }
    }

    ByteVolume::Pointer ReadByteVolume(const string& filename) {

        // in case not already done
       //RIZ: MINClib are not availablesince IBIS2.2 itk::RegisterMincIO();

        if (!IsFileExists(filename)) {
            cerr << "File " << filename << " does not exist" << endl;
            return ByteVolume::Pointer();
        }
        try {
            ByteVolumeReader::Pointer reader = ByteVolumeReader::New();
            reader->SetFileName(filename);
            reader->Update();
            ByteVolume::Pointer vol = reader->GetOutput();
            vol->DisconnectPipeline();
            return vol;
        } catch (itk::ExceptionObject &excep) {
            cerr << "Failed to read: " << filename << endl;
            return ByteVolume::Pointer();
        }
    }

    FloatVectorVolume::Pointer ReadFloatVectorVolume(const string& filename) {

        // in case not already done
        //RIZ: MINClib are not availablesince IBIS2.2 itk::RegisterMincIO();

        if (!IsFileExists(filename)) {
            cerr << "File " << filename << " does not exist" << endl;
            return FloatVectorVolume::Pointer();
        }
        try {
            FloatVectorVolumeReader::Pointer reader = FloatVectorVolumeReader::New();
            reader->SetFileName(filename);
            reader->Update();
            FloatVectorVolume::Pointer vol = reader->GetOutput();
            vol->DisconnectPipeline();
            return vol;
        } catch (itk::ExceptionObject &excep) {
            cerr << "Failed to read: " << filename << endl;
            return FloatVectorVolume::Pointer();
        }
    }


    void WriteFloatVolume(const string& filename, FloatVolume::Pointer vol) {
        // in case not already done
        //RIZ: MINClib are not availablesince IBIS2.2 itk::RegisterMincIO();

        FloatVolumeWriter::Pointer writer = FloatVolumeWriter::New();
        writer->SetFileName(filename);
        writer->SetInput(vol);
        writer->Update();
    }

    void WriteIntVolume(const string& filename, IntVolume::Pointer vol) {
        // in case not already done
        //RIZ: MINClib are not availablesince IBIS2.2 itk::RegisterMincIO();

        IntVolumeWriter::Pointer writer = IntVolumeWriter::New();
        writer->SetFileName(filename);
        writer->SetInput(vol);
        writer->Update();
    }

    void WriteByteVolume(const string& filename, ByteVolume::Pointer vol) {
        // in case not already done
        //RIZ: MINClib are not availablesince IBIS2.2 	itk::RegisterMincIO();

        ByteVolumeWriter::Pointer writer = ByteVolumeWriter::New();
        writer->SetFileName(filename);
        writer->SetInput(vol);
        writer->Update();
    }

    void WriteFloatVectorVolume(const string& filename, FloatVectorVolume::Pointer vol) {
        // in case not already done
       //RIZ: MINClib are not availablesince IBIS2.2  	itk::RegisterMincIO();

        FloatVectorVolumeWriter::Pointer writer = FloatVectorVolumeWriter::New();
        writer->SetFileName(filename);
        writer->SetInput(vol);
        writer->Update();
    }

    FloatVolume::Pointer CopyFloatVolume(FloatVolume::Pointer vol) {
        FloatVolumeDuplicator::Pointer dup = FloatVolumeDuplicator::New();
        dup->SetInputImage(vol);
        dup->Update();
        FloatVolume::Pointer outVol = dup->GetOutput();
        outVol->DisconnectPipeline();
        return outVol;
    }


    IntVolume::Pointer CopyIntVolume(IntVolume::Pointer vol) {
        IntVolumeDuplicator::Pointer dup = IntVolumeDuplicator::New();
        dup->SetInputImage(vol);
        dup->Update();
        IntVolume::Pointer outVol = dup->GetOutput();
        outVol->DisconnectPipeline();
        return outVol;
    }

    ByteVolume::Pointer CopyByteVolume(ByteVolume::Pointer vol) {
        ByteVolumeDuplicator::Pointer dup = ByteVolumeDuplicator::New();
        dup->SetInputImage(vol);
        dup->Update();
        ByteVolume::Pointer outVol = dup->GetOutput();
        outVol->DisconnectPipeline();
        return outVol;
    }


    IntVolume::Pointer FloatToIntVolume(FloatVolume::Pointer vol) {
        return FloatToIntVolume(FloatVolume::ConstPointer(vol));
    }

    FloatVolume::Pointer IntToFloatVolume(IntVolume::Pointer vol) {
        return IntToFloatVolume(IntVolume::ConstPointer(vol));
    }

    IntVolume::Pointer FloatToIntVolume(FloatVolume::ConstPointer vol) {
        FloatToIntVolumeFilter::Pointer filt = FloatToIntVolumeFilter::New();
        filt->SetInput(vol);
        filt->Update();
        IntVolume::Pointer outVol = filt->GetOutput();
        outVol->DisconnectPipeline();
        return outVol;
    }

    FloatVolume::Pointer IntToFloatVolume(IntVolume::ConstPointer vol) {
        IntToFloatVolumeFilter::Pointer filt = IntToFloatVolumeFilter::New();
        filt->SetInput(vol);
        filt->Update();
        FloatVolume::Pointer outVol = filt->GetOutput();
        outVol->DisconnectPipeline();
        return outVol;
    }

};
