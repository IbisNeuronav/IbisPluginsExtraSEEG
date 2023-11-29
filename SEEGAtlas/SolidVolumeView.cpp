/**
 * @file SolidVolumeView.cpp
 *
 * Implementation of the SolidVolumeView class
 *
 * @author Silvain Beriault
 */


// header files to include
#include "SolidVolumeView.h"
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkPiecewiseFunction.h>
#include <itkImageToVTKImageFilter.h>
#include <vtkVolumeProperty.h>
#include <vtkVolumeRayCastCompositeFunction.h>
#include <vtkVolumeRayCastMapper.h>
#include <vtkVolume.h>
#include <vtkColorTransferFunction.h>
#include "vtkVolumeTextureMapper2D.h"
#include <vtkGPUVolumeRayCastMapper.h>

namespace seeg {



    /*********************************************************************************************
     *  Implementation of SolidVolumeView class
     *********************************************************************************************/



    /**** Constructors/Destructor ****/

    SolidVolumeView::SolidVolumeView(MapperType mapperType) {
        m_MapperType = mapperType;
    }


    SolidVolumeView::~SolidVolumeView() {
        // nothing to do
    }


    /**** ACCESSORS/SETTERS ****/

    vtkSmartPointer<vtkColorTransferFunction> SolidVolumeView::GetColorTransferFunction(const std::string& name) {
        if (m_AllVolumes.find(name) != m_AllVolumes.end()) {
            return m_AllVolumes[name].colorTransfer;
        }
        return 0;
    }

    vtkSmartPointer<vtkPiecewiseFunction> SolidVolumeView::GetOpacityTransferFunction(const std::string& name) {
        if (m_AllVolumes.find(name) != m_AllVolumes.end()) {
            return m_AllVolumes[name].opacityTransfer;
        }
        return 0;
    }

    vtkSmartPointer<vtkActor> SolidVolumeView::GetActor(const std::string& name) {
        if (m_AllActors.find(name) != m_AllActors.end()) {
            return m_AllActors[name];
        }
        return 0;
    }

    void SolidVolumeView::SetVolumeVisibility(const std::string& name, int visibility) {
        if (m_AllVolumes.find(name) != m_AllVolumes.end()) {
            m_AllVolumes[name].volume->SetVisibility(visibility);
        }
    }


    QVTKWidget * SolidVolumeView::GetWidget() {
        return &m_Widget;
    }


    ByteVolume::Pointer SolidVolumeView::GetVolume(const std::string& name) {
        if (m_AllVolumes.find(name) != m_AllVolumes.end()) {
            return m_AllVolumes[name].origVolume;
        }
        return ByteVolume::Pointer();
    }

    void SolidVolumeView::SetBackgroundColor(double *rgb) {
        m_Renderer->SetBackground(rgb[0], rgb[1], rgb[2]);
    }


    /**** PUBLIC FUNCTIONS ****/


    void SolidVolumeView::AddActor (    const std::string& name,
                                        vtkSmartPointer<vtkActor> actor) {
        RemoveActor(name);
        m_AllActors[name] = actor;
        m_Renderer->AddActor(actor);
        m_Widget.renderWindow()->Render();
    }


    void SolidVolumeView::RemoveActor(const std::string& name) {
        if (m_AllActors.find(name) != m_AllActors.end()) {
            m_Renderer->RemoveActor(m_AllActors[name]);
            m_Widget.renderWindow()->Render();
        }
        m_AllActors.erase(name);
    }




    void SolidVolumeView::AddVolume( const std::string& name,
                    ByteVolume::Pointer volume,
                    vtkSmartPointer<vtkVolumeProperty> volumeProperty) {

        // in case volume already exists;
        RemoveVolume(name);


          // convert itk image to VTK format
        ConnectorType::Pointer connector = ConnectorType::New();
        connector->SetInput(volume);
        connector->Update();

        vtkSmartPointer<vtkVolume> vol = vtkSmartPointer<vtkVolume>::New();

#if 0
        if (m_MapperType == MapperTypeRayCast) {
            vtkSmartPointer<vtkVolumeRayCastMapper> volumeMapper = vtkSmartPointer<vtkVolumeRayCastMapper>::New();
            vtkSmartPointer<vtkVolumeRayCastCompositeFunction> rayCastFunction = vtkSmartPointer<vtkVolumeRayCastCompositeFunction>::New();
            volumeMapper->SetInputData (connector->GetOutput());
            volumeMapper->SetVolumeRayCastFunction(rayCastFunction);
            vol->SetMapper(volumeMapper);
        } else {
            vtkSmartPointer<vtkVolumeTextureMapper2D> volumeMapper = vtkSmartPointer<vtkVolumeTextureMapper2D>::New();
            volumeMapper->SetInputData (connector->GetOutput());
            vol->SetMapper(volumeMapper);
        }
#else
		vtkSmartPointer<vtkGPUVolumeRayCastMapper> volumeMapper = vtkSmartPointer<vtkGPUVolumeRayCastMapper>::New();
		volumeMapper->SetInputData(connector->GetOutput());
		vol->SetMapper(volumeMapper);
#endif

        vol->SetProperty(volumeProperty);


        m_AllVolumes[name].origVolume = volume;
        m_AllVolumes[name].connector = connector;
        m_AllVolumes[name].volume = vol;
        m_AllVolumes[name].colorTransfer = volumeProperty->GetRGBTransferFunction();
        m_AllVolumes[name].opacityTransfer = volumeProperty->GetScalarOpacity();


        vtkRenderWindow *renderWindow = m_Widget.renderWindow();
        vtkRenderWindowInteractor *renderWindowInteractor = m_Widget.interactor();

        if (m_Renderer == 0) {
            m_Renderer = vtkSmartPointer<vtkRenderer>::New();
            m_Renderer->AddVolume(vol);
            renderWindow->AddRenderer(m_Renderer);
            renderWindowInteractor->SetRenderWindow(renderWindow);
            vtkSmartPointer<vtkInteractorStyleTrackballCamera> interactorStyle;
            interactorStyle=vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New();
            renderWindowInteractor->SetInteractorStyle(interactorStyle);

        } else {
            m_Renderer->AddVolume(vol);
        }

        m_Widget.renderWindow()->Render();


    }


    void SolidVolumeView::RemoveAll() {
        SolidVolumeSceneMap::iterator it;
        m_Renderer = 0;
        m_AllVolumes.clear();
        m_AllActors.clear();
    }

    void SolidVolumeView::RemoveVolume(const std::string& name) {
        if (m_AllVolumes.find(name) != m_AllVolumes.end()) {
            m_Renderer->RemoveVolume(m_AllVolumes[name].volume);
            m_Widget.renderWindow()->Render();
        }
        m_AllVolumes.erase(name);
    }



    void SolidVolumeView::Render() {
        m_Widget.renderWindow()->Render();
    }

    /**** PROTECTED AND PRIVATE FUNCTIONS ****/


}
