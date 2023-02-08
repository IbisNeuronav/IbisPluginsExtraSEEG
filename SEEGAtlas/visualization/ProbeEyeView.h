#ifndef __PROBE_EYE_VIEW_H__
#define __PROBE_EYE_VIEW_H__

// header files to include
#include <vtkSmartPointer.h>
#include "BasicTypes.h"
#include "VolumeTypes.h"
#include "itkImageToVTKImageFilter.h"
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkImageResliceMapper.h>
#include <vtkImageSlice.h>
#include <vtkInteractorStyleImage.h>
#include <vtkPolyData.h>
#include <QVTKRenderWidget.h>
#include <vector>
#include <vtkImageProperty.h>

namespace seeg {


    class ProbeEyeView;

    enum RESLICE_MODE {
        RESLICE_MODE_MIN = VTK_IMAGE_SLAB_MIN,
        RESLICE_MODE_MAX = VTK_IMAGE_SLAB_MAX,
        RESLICE_MODE_MEAN = VTK_IMAGE_SLAB_MEAN,
        RESLICE_MODE_SUM = VTK_IMAGE_SLAB_SUM
    };


    class ProbeEyeInteractorStyle : public vtkInteractorStyleImage {
    public:

        static ProbeEyeInteractorStyle* New();

        vtkTypeMacro(ProbeEyeInteractorStyle, vtkInteractorStyleImage);


        void SetObject(ProbeEyeView *obj) {
            this->obj = obj;
        }

        void OnMouseWheelForward();
        void OnMouseWheelBackward();

        virtual void OnMiddleButtonDown() { }
        virtual void OnMiddleButtonUp() { }
        virtual void OnLeftButtonDown();
        virtual void OnLeftButtonUp();
        virtual void OnChar();



    protected:
        ProbeEyeView *obj;

    };




    class ProbeEyeView {

    public:
        typedef mrilSmartPtr<ProbeEyeView> Pointer;

        static Pointer New(ByteVolume::Pointer volume) {
            return Pointer(new ProbeEyeView(volume));
        }

    protected:

        ProbeEyeView(ByteVolume::Pointer volume);

    public:

        virtual ~ProbeEyeView();


        QVTKRenderWidget * GetProbeEyeWidget() {
            return &m_ProbeEyeWidget;
        }
        ByteVolume::Pointer GetVolume();

        double GetCurrentDistanceToTarget();

        vtkImageProperty* GetImageProperty();


// Probe eye view functions

        void ChangeOrientation(Point3D target, Point3D entry);
        void NextSlice(bool sliceUp);
        void GoToSlice(double distToTarget_world);
        void Render();

        void SetSlabThickness(double thickness);
        void SetResliceMode(RESLICE_MODE mode);


    private:

        void Init();
        void GenerateCursor();
        void UpdateCursor();

    private:

        double GetMaxDistanceToTarget();

        //  private type for conversion from itk to vtk images
        typedef itk::ImageToVTKImageFilter<ByteVolume> GrayConnectorType;

        // volume currently being probe-eyed
        ByteVolume::Pointer m_Volume;

        // vtk pipeline for the probe-eye volume
        GrayConnectorType::Pointer m_Connector;
        vtkSmartPointer<vtkImageSlice> m_ProbeEyeImageSlice;
        vtkSmartPointer<vtkImageResliceMapper> m_ProbeEyeImageResliceMapper;
        vtkSmartPointer<vtkRenderer> m_ProbeEyeRenderer;
        QVTKRenderWidget m_ProbeEyeWidget;
        vtkSmartPointer<ProbeEyeInteractorStyle> m_ProbeEyeInteractorStyle;

        // cursor
        vtkSmartPointer<vtkPolyData> m_ProbeEyeCursorPolyData;
        vtkSmartPointer<vtkPolyDataMapper>  m_ProbeEyeCursorMapper;
        vtkSmartPointer<vtkActor> m_ProbeEyeCursorActor;

        // current target and entry point
        Point3D m_ProbeEyeTarget;
        Point3D m_ProbeEyeEntry;

        bool m_FirstInit;

    };
}


#endif
