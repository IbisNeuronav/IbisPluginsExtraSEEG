/**
 * @file TrajectoryROIPipeline.cpp
 *
 * Implementation of the TrajectoryROIPipeline class
 *
 * @author Silvain Beriault
 */


// Header files to include
#include "TrajectoryROIPipeline.h"
#include "itkCastImageFilter.h"


namespace seeg {


    /**** CONSTRUCTORS / DESTRUCTOR ****/
    TrajectoryROIPipeline::TrajectoryROIPipeline (const string& templateVolumeFile, Point3D targetPointWorld) {
        m_TemplateVolume = ReadFloatVolume(templateVolumeFile);
        m_TemplateVolume->TransformPhysicalPointToIndex(targetPointWorld, m_TargetPoint);
        InitPipeline();
    }

    TrajectoryROIPipeline::TrajectoryROIPipeline (FloatVolume::Pointer templateVolume, Point3D targetPointWorld) {
        m_TemplateVolume = templateVolume;
        m_TemplateVolume->TransformPhysicalPointToIndex(targetPointWorld, m_TargetPoint);
        InitPipeline();
    }

    TrajectoryROIPipeline::TrajectoryROIPipeline (IntVolume::Pointer templateVolume, Point3D targetPointWorld) {
        typedef CastImageFilter<IntVolume, FloatVolume> CastFilterType;
        CastFilterType::Pointer castFilter = CastFilterType::New();
        castFilter->SetInput(templateVolume);
        castFilter->Update();
        m_TemplateVolume = castFilter->GetOutput();
        m_TemplateVolume->TransformPhysicalPointToIndex(targetPointWorld, m_TargetPoint);
        InitPipeline();
    }

    TrajectoryROIPipeline::TrajectoryROIPipeline (ByteVolume::Pointer templateVolume, Point3D targetPointWorld) {
        typedef CastImageFilter<ByteVolume, FloatVolume> CastFilterType;
        CastFilterType::Pointer castFilter = CastFilterType::New();
        castFilter->SetInput(templateVolume);
        castFilter->Update();
        m_TemplateVolume = castFilter->GetOutput();
        m_TemplateVolume->TransformPhysicalPointToIndex(targetPointWorld, m_TargetPoint);
        InitPipeline();
    }


    TrajectoryROIPipeline::~TrajectoryROIPipeline() {
        // Do nothing
    }


    /**** PUBLIC FUNCTIONS ****/

    void TrajectoryROIPipeline::SetTargetPoint (Point3D targetPointWorld) {
        m_TemplateVolume->TransformPhysicalPointToIndex(targetPointWorld, m_TargetPoint);
        m_DistFromTrajVolFilt->SetTargetPoint(m_TargetPoint);
    }


    void TrajectoryROIPipeline::CalcDistanceMap( Point3D entryPointWorld,
                              float maxRadius,
                              bool fullImage) {

        FloatVolume::IndexType entryPointIndex;
        m_TemplateVolume->TransformPhysicalPointToIndex(entryPointWorld, entryPointIndex);
        FloatVolume::SpacingType spacing;
        spacing = m_TemplateVolume->GetSpacing();

        m_DistFromTrajVolFilt->SetEntryPoint(entryPointIndex);

        FloatVolume::RegionType region;

        if (fullImage) {
            region = m_TemplateVolume->GetLargestPossibleRegion();
        } else {
            // Calculate the trajectory's bounding box.
            FloatVolume::RegionType regionMax;
            int min[3];
            int max[3];
            for (int i=0; i<3; i++) {
                if (entryPointIndex[i] < m_TargetPoint[i]) {
                    min[i] = (((float)(entryPointIndex[i]*spacing[i]) - maxRadius) / spacing[i]) + 0.5;
                    max[i] = (((float)(m_TargetPoint[i]*spacing[i]) + maxRadius) / spacing[i]) + 0.5;
                } else {
                    max[i] = (((float)(entryPointIndex[i]*spacing[i]) + maxRadius) / spacing[i]) + 0.5;
                    min[i] = (((float)(m_TargetPoint[i]*spacing[i]) - maxRadius) / spacing[i]) + 0.5;
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

        m_DistFromTrajVolFilt->GetOutput()->SetRequestedRegion(region);
        m_DistFromTrajVolFilt->Update();

    }


    FloatVolume::Pointer TrajectoryROIPipeline::GetLastDistanceMap(bool copy) {
        if (copy) {
            FloatVolumeDuplicator::Pointer vd = FloatVolumeDuplicator::New();
            vd->SetInputImage(m_DistFromTrajVolFilt->GetOutput());
            return vd->GetOutput();
        } else {
            return m_DistFromTrajVolFilt->GetOutput();
        }
    }


    /**** PRIVATE FUNCTIONS ****/

    void TrajectoryROIPipeline::InitPipeline() {
        m_DistFromTrajVolFilt = DistFromTrajVolumeFilter::New();
        m_DistFromTrajVolFilt->SetTargetPoint(m_TargetPoint);
        m_DistFromTrajVolFilt->SetInput(m_TemplateVolume);
    }
}

