/**
 * @file SEEGROIPipeline.cpp
 *
 * Implementation of the SEEGROIPipeline class
 * based on TrajectoryROIPipeline
 *
 * @author Silvain Beriault & Rina Zelmann
 */


// Header files to include
#include "itkCastImageFilter.h"
#include "SEEGROIPipeline.h"



namespace mril {


    /**** CONSTRUCTORS / DESTRUCTOR ****/
    SEEGROIPipeline::SEEGROIPipeline (const string& templateVolumeFile) {
        m_TemplateVolume = ReadFloatVolume(templateVolumeFile);
        InitPipeline();
    }

    SEEGROIPipeline::SEEGROIPipeline (FloatVolume::Pointer templateVolume) {
        m_TemplateVolume = templateVolume;
        InitPipeline();
    }

    SEEGROIPipeline::SEEGROIPipeline (IntVolume::Pointer templateVolume) {
        typedef CastImageFilter<IntVolume, FloatVolume> CastFilterType;
        CastFilterType::Pointer castFilter = CastFilterType::New();
        castFilter->SetInput(templateVolume);
        castFilter->Update();
        m_TemplateVolume = castFilter->GetOutput();
        InitPipeline();
    }

    SEEGROIPipeline::SEEGROIPipeline (ByteVolume::Pointer templateVolume) {
        typedef CastImageFilter<ByteVolume, FloatVolume> CastFilterType;
        CastFilterType::Pointer castFilter = CastFilterType::New();
        castFilter->SetInput(templateVolume);
        castFilter->Update();
        m_TemplateVolume = castFilter->GetOutput();
        InitPipeline();
    }


    SEEGROIPipeline::~SEEGROIPipeline() {
        // Do nothing
    }


    /**** PUBLIC FUNCTIONS ****/

 /*   // Getters and setters
    FloatVolume::Pointer SEEGROIPipeline::GetTemplateVol() {
        return m_TemplateVolume;
    }

    void SEEGROIPipeline::SetTemplateVol(FloatVolume::Pointer templateVol) {
        m_TemplateVolume = templateVol;
    }
*/

    // in each of the derived classes


    /**** PRIVATE FUNCTIONS ****/
    void SEEGROIPipeline::InitPipeline() {
        //do nothing

    }

}

