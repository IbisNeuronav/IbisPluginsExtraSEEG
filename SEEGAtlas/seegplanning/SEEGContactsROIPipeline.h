#ifndef SEEGCONTACTSROIPIPELINE_H
#define SEEGCONTACTSROIPIPELINE_H

/**
 * @file SEEGContactsROIPipeline.h
 *
 * Based on SEEGContactsROIPipeline.h which
 * Defines an itk pipeline to look for the contacts in the electrode
 * and computes distance maps and area recorded by the contacts
 *
 * @author Silvain Beriault & Rina Zelmann
 */

// Header files to include
#include <string>
#include "BasicTypes.h"
#include "MathUtils.h"
#include "VolumeTypes.h"
//#include "SEEGROIPipeline.h"

#include "itkDanielssonDistanceMapImageFilter.h"
#include "itkBinaryContourImageFilter.h"
#include "itkAddImageFilter.h"
#include "itkMultiplyImageFilter.h"
#include "itkRegionOfInterestImageFilter.h"
#include "itkInvertIntensityImageFilter.h"
#include "itkMinimumMaximumImageCalculator.h"
#include "itkPasteImageFilter.h"
#include "itkNaryAddImageFilter.h"

#include "itkSignedMaurerDistanceMapImageFilter.h"

#include "SEEGElectrodeModel.h"
#include "ElectrodeInfo.h"
#include "BipolarChannelModel.h"

using namespace std;
using namespace itk;

namespace seeg {


  class SEEGContactsROIPipeline {

    public:

      /** SmartPointer type for the SEEGROIPipeline class */
      typedef mrilSmartPtr<SEEGContactsROIPipeline> Pointer;

     static Pointer New(FloatVolume::Pointer templateVolume, SEEGElectrodeModel::SEEG_ELECTRODE_MODEL_TYPE type) { return Pointer(new SEEGContactsROIPipeline(templateVolume, type)); }

     static Pointer New(IntVolume::Pointer templateVolume, SEEGElectrodeModel::SEEG_ELECTRODE_MODEL_TYPE type) { return Pointer(new SEEGContactsROIPipeline(templateVolume, type)); }

     static Pointer New(ByteVolume::Pointer templateVolume, SEEGElectrodeModel::SEEG_ELECTRODE_MODEL_TYPE type) { return Pointer(new SEEGContactsROIPipeline(templateVolume, type)); }



  protected:
     // Constructors

    SEEGContactsROIPipeline (FloatVolume::Pointer templateVolume, SEEGElectrodeModel::SEEG_ELECTRODE_MODEL_TYPE type);

    SEEGContactsROIPipeline (IntVolume::Pointer templateVolume, SEEGElectrodeModel::SEEG_ELECTRODE_MODEL_TYPE type);

    SEEGContactsROIPipeline (ByteVolume::Pointer templateVolume, SEEGElectrodeModel::SEEG_ELECTRODE_MODEL_TYPE type);

    SEEGContactsROIPipeline (const string& templateVolumeFile, SEEGElectrodeModel::SEEG_ELECTRODE_MODEL_TYPE type);

  public:
        virtual ~SEEGContactsROIPipeline();

  public:

// functions
        FloatVolume::Pointer CompDistMapFromEdge(FloatVolume::Pointer inputVol);

        FloatVolume::Pointer CompDistMapFromMask(FloatVolume::Pointer inputVol);

        FloatVolume::Pointer CompSquareDistMapFromPoint(FloatVolume::Pointer inputVol);

        FloatVolume::Pointer CompSquareDistMapFromPoints(FloatVolume::Pointer inputVol); //uses Maurer DistanceMap -> faster than Danielsson

        FloatVolume::Pointer CompSquareDistMapFromPoint(FloatVolume::Pointer inputVol, FloatVolume::RegionType region);

        FloatVolume::Pointer GetRecordedVolPerElectrode(ElectrodeInfo::Pointer electrode);

        void CalcRecordingMap(FloatVolume::Pointer targetDistMap, FloatVolume::Pointer& recordingMap, ElectrodeInfo::Pointer electrode);

        void CalcRecordingMap(vector<FloatVolume::Pointer> targetDistMaps, vector<FloatVolume::Pointer> &recordingMaps, ElectrodeInfo::Pointer electrode);

        void CalcTrajBoundingBox(FloatVolume::IndexType entryPointIndex, FloatVolume::IndexType targetPointIndex, Point3D maxRadius, FloatVolume::RegionType& region);

        vector<int> GetLabelsInContact(int contactIndex, ElectrodeInfo::Pointer electrode, FloatVolume::Pointer anatLabelsMap, int &voxelsInContactVol);

        FloatVolume::Pointer CalcVolOfContact(int contactIndex, ElectrodeInfo::Pointer electrode, FloatVolume::Pointer anatLabelsMap);

        FloatVolume::Pointer CalcVolOfCenterOfContact(int contactIndex, ElectrodeInfo::Pointer electrode, FloatVolume::Pointer anatLabelsMap);

    //Functions channel based
        FloatVolume::Pointer CalcChannelRecordingArea(Point3D contact1Center, Point3D contact2Center, double maxRadius, FloatVolume::RegionType channelRegion);

        FloatVolume::Pointer CalcChannelRecordingArea(Point3D contact1Center, Point3D contact2Center, double maxRadius);

        vector<int> GetLabelsInChannel(int contact1Index, int contact2Index, ElectrodeInfo::Pointer electrode, FloatVolume::Pointer anatLabelsMap);

        FloatVolume::Pointer CalcVolOfChannel(int contact1Index, int contact2Index, ElectrodeInfo::Pointer electrode, FloatVolume::Pointer anatLabelsMap);

//Getters and Setters
        FloatVolume::Pointer GetTemplateVol();

        void SetTemplateVol(FloatVolume::Pointer templateVol);

        void EmptyTemplateVol();

        FloatVolume::Pointer GetChannelModelVol();

        int GetNumberVoxelsInChannelModel();

    private:

        typedef DanielssonDistanceMapImageFilter<FloatVolume,FloatVolume> DistanceWithinStructureFilterType;
        typedef SignedMaurerDistanceMapImageFilter<FloatVolume,FloatVolume> DistanceSignedMauerWithinStructureFilterType;

        typedef MultiplyImageFilter <FloatVolume, FloatVolume > MultiplyImageFilterType;
        typedef RegionOfInterestImageFilter< FloatVolume, FloatVolume > RegionOfInterestFilterType;
        typedef MinimumMaximumImageCalculator <FloatVolume> MaxMinFilterType;
        typedef InvertIntensityImageFilter <FloatVolume> InvertIntensityImageFilterType;
        typedef PasteImageFilter <FloatVolume, FloatVolume > PasteImageFilterType;
      //  typedef itk::NaryAddImageFilter <FloatVolume, FloatVolume > NaryAddImageFilterType;


  private:
        void InitPipeline();

        /** Pointer to the template volume */
        FloatVolume::Pointer m_TemplateVolume; // for volume dimension, spacing, etc
        SEEGElectrodeModel::Pointer m_ElectrodeModel;
        BipolarChannelModel::Pointer m_ChannelModel; // Bipolar channel model
    };
}
#endif // SEEGCONTACTSROIPIPELINE_H
