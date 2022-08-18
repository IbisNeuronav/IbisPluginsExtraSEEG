/**
     * @file SEEGTrajectoryROIPipeline.cpp
     *
     * Implementation of the SEEGTrajectoryROIPipeline class
     * based on TrajectoryROIPipeline
     *
     * @author Silvain Beriault & Rina Zelmann
     */


// Header files to include
#include "SEEGTrajectoryROIPipeline.h"
#include "itkCastImageFilter.h"
#include <qglobal.h>

namespace seeg {


    /**** PUBLIC FUNCTIONS ****/
    /**** CONSTRUCTORS / DESTRUCTOR ****/
    SEEGTrajectoryROIPipeline::SEEGTrajectoryROIPipeline (const string& templateVolumeFile) {
        m_TemplateVolume = ReadFloatVolume(templateVolumeFile);
        this->InitPipeline();
    }

    SEEGTrajectoryROIPipeline::SEEGTrajectoryROIPipeline (FloatVolume::Pointer templateVolume) {
        m_TemplateVolume = templateVolume;
        this->InitPipeline();
    }

    SEEGTrajectoryROIPipeline::SEEGTrajectoryROIPipeline (IntVolume::Pointer templateVolume) {
        typedef CastImageFilter<IntVolume, FloatVolume> CastFilterType;
        CastFilterType::Pointer castFilter = CastFilterType::New();
        castFilter->SetInput(templateVolume);
        castFilter->Update();
        m_TemplateVolume = castFilter->GetOutput();
        this->InitPipeline();
    }

    SEEGTrajectoryROIPipeline::SEEGTrajectoryROIPipeline (ByteVolume::Pointer templateVolume) {
        typedef CastImageFilter<ByteVolume, FloatVolume> CastFilterType;
        CastFilterType::Pointer castFilter = CastFilterType::New();
        castFilter->SetInput(templateVolume);
        castFilter->Update();
        m_TemplateVolume = castFilter->GetOutput();
        this->InitPipeline();
    }


    SEEGTrajectoryROIPipeline::~SEEGTrajectoryROIPipeline() {
        // Do nothing
    }


    /**** PUBLIC FUNCTIONS ****/

    // Getters and setters
    FloatVolume::Pointer SEEGTrajectoryROIPipeline::GetTemplateVol() {
        return m_TemplateVolume;
    }

    void SEEGTrajectoryROIPipeline::SetTemplateVol(FloatVolume::Pointer templateVol) {
        m_TemplateVolume = templateVol;
    }


    /**** PUBLIC FUNCTIONS ****/
    // Changed by RIZ - to include target as input
    void SEEGTrajectoryROIPipeline::CalcDistanceMap( Point3D entryPointWorld,
                                                     Point3D targetPointWorld,
                                                     float maxRadius,
                                                     bool fullImage) {

        FloatVolume::IndexType entryPointIndex;
        FloatVolume::IndexType targetPointIndex;
        m_TemplateVolume->TransformPhysicalPointToIndex(entryPointWorld, entryPointIndex);
        m_TemplateVolume->TransformPhysicalPointToIndex(targetPointWorld, targetPointIndex);
        FloatVolume::SpacingType spacing;
        spacing = m_TemplateVolume->GetSpacing();

        m_DistFromTrajVolFilt->SetEntryPoint(entryPointIndex);
        m_DistFromTrajVolFilt->SetTargetPoint(targetPointIndex); // RIZ added: CHECK that it is OK that is duplicated

        FloatVolume::RegionType region;

        if (fullImage) {
            region = m_TemplateVolume->GetLargestPossibleRegion();
        } else {
            // Calculate the trajectory's bounding box.
            FloatVolume::RegionType regionMax;
            int min[3];
            int max[3];
            for (int i=0; i<3; i++) {
                if (entryPointIndex[i] < targetPointIndex[i]) {
                    min[i] = (((float)(entryPointIndex[i]*spacing[i]) - maxRadius) / spacing[i]) + 0.5;
                    max[i] = (((float)(targetPointIndex[i]*spacing[i]) + maxRadius) / spacing[i]) + 0.5;
                } else {
                    max[i] = (((float)(entryPointIndex[i]*spacing[i]) + maxRadius) / spacing[i]) + 0.5;
                    min[i] = (((float)(targetPointIndex[i]*spacing[i]) - maxRadius) / spacing[i]) + 0.5;
                }
            }

            regionMax = m_TemplateVolume->GetLargestPossibleRegion();
            FloatVolume::IndexType start;
            FloatVolume::SizeType size;
            start = regionMax.GetIndex();
            size = regionMax.GetSize();

            for (int i=0; i<3; i++) {
                if (min[i] < start[i]) {
                    min[i] = start[i];
                }
                if (max[i] >= (int) (start[i] + size[i])) {
                    max[i] = start[i] + size[i] - 1;
                }

            }
            for (int i=0; i<3; i++) {
                start[i] = min[i];
                size[i] = max[i] - min[i] + 1;
            }
            region.SetIndex(start);
            region.SetSize(size);
        }
    //    cout << "m_DistFromTrajVolFilt Region: " << region.GetIndex() <<" - "<< region.GetSize() << std::endl;

        m_DistFromTrajVolFilt->GetOutput()->SetRequestedRegion(region);
        m_DistFromTrajVolFilt->Update();
        Q_ASSERT( m_DistFromTrajVolFilt );

    }


    FloatVolume::Pointer SEEGTrajectoryROIPipeline::GetLastDistanceMap(bool copy) {
        if (copy) {
            FloatVolumeDuplicator::Pointer vd = FloatVolumeDuplicator::New();
            vd->SetInputImage(m_DistFromTrajVolFilt->GetOutput());
            return vd->GetOutput();
        } else {
            return m_DistFromTrajVolFilt->GetOutput();
        }
    }


/*     void SEEGTrajectoryROIPipeline::GetAllPointsAroundTrajectory(Point3D targetPoint, Point3D entryPoint, float maxRadius, vector<Point3D> &allPointsVec){

         SEEGElectrodeModel::Pointer electrode = SEEGElectrodeModel::New(SEEGElectrodeModel::MNI); //RIZ: keep for now!! probably the whole function will disapear!

     //    double recRadius = electrode->GetRecordingRadius();
         FloatVolume::SpacingType spacing;
         spacing = m_TemplateVolume->GetSpacing();
         float minSpacing=spacing[0];
         for (int i=1;i<3; i++) {
             if(spacing[i]<minSpacing) minSpacing=spacing[i];

         }

         electrode->CalcAllLinePositions(minSpacing, targetPoint, entryPoint, allPointsVec);
         //cout << "target "<<targetPoint<<" entry "<<entryPoint<<"spacing "<<spacing<<endl;
        // cout << "allPoints "<<allPointsVec.<<endl;

     }
*/

    /**** PRIVATE FUNCTIONS ****/

    void SEEGTrajectoryROIPipeline::InitPipeline() {
        m_DistFromTrajVolFilt = DistFromTrajVolumeFilter::New();
        m_DistFromTrajVolFilt->SetInput(m_TemplateVolume);
    }
    }

