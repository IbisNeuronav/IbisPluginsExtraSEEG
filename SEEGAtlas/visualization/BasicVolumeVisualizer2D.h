#ifndef __BASIC_VOLUME_VISUALIZER_2D_H__
#define __BASIC_VOLUME_VISUALIZER_2D_H__

// header files to include
#include <vtkSmartPointer.h>
#include "BasicTypes.h"
#include "VolumeTypes.h"
#include "itkImageToVTKImageFilter.h"
#include <vtkRenderer.h>
#include <vtkImagePlaneWidget.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkImageSliceMapper.h>
#include <vtkImageResliceMapper.h>
#include <vtkImageSlice.h>
#include <vtkImageActor.h>
#include <vtkImageStack.h>
#include <vtkInteractorStyleImage.h>
#include <QVTKWidget.h>
#include <map>
#include <string>
#include <vtkHandleWidget.h>
#include <vtkPointHandleRepresentation3D.h>
#include <vtkCommand.h>

namespace seeg {


    class BasicVolumeVisualizer2D;


    class TriplanarInteractorStyle : public vtkInteractorStyleImage {
    public:

        static TriplanarInteractorStyle* New();

        vtkTypeMacro(TriplanarInteractorStyle, vtkInteractorStyleImage);

        void SetObject(BasicVolumeVisualizer2D *obj) {
            this->obj = obj;
        }

        void SetSliceDirection(int index) {
            m_SliceDirection = index;
        }

        void OnMouseWheelForward();
        void OnMouseWheelBackward();


    protected:

        BasicVolumeVisualizer2D *obj;

        /** Slice direction (0 for x, 1 for y, 2 for z) */
        int m_SliceDirection;

    };

    class ProbeEyeInteractorStyle : public vtkInteractorStyleImage {
    public:

        static ProbeEyeInteractorStyle* New();

        vtkTypeMacro(ProbeEyeInteractorStyle, vtkInteractorStyleImage);


        void SetObject(BasicVolumeVisualizer2D *obj) {
            this->obj = obj;
        }

        void OnMouseWheelForward();
        void OnMouseWheelBackward();

        virtual void OnMiddleButtonDown() { }
        virtual void OnMiddleButtonUp() { }



    protected:
        BasicVolumeVisualizer2D *obj;

    };




    class BasicVolumeVisualizer2D  : public vtkCommand {

    public:
        typedef mrilSmartPtr<BasicVolumeVisualizer2D> Pointer;

        static Pointer New() {
            return Pointer(new BasicVolumeVisualizer2D());
        }

    protected:

        BasicVolumeVisualizer2D();

    public:

        virtual ~BasicVolumeVisualizer2D();


        void SetMainVolume(ByteVolume::Pointer volume);
        ByteVolume::Pointer GetMainVolume();
        ByteVolume::SizeType GetVolumeSize();


        // Add/remove overlay volumes data
        RGBAVolume::Pointer GetRGBAOverlayVolume(const std::string& label);
        void AddRGBAOverlayVolume(const std::string& label, RGBAVolume::Pointer volume);
        void RemoveRGBAOverlayVolume(const std::string& label);
        bool RGBAOverlayVolumeExists(const std::string &label);
        void SetRGBAOverlayGlobalOpacity(const std::string& label, double opacity);


// Triplanar view functions
        ByteVolume::IndexType GetTriplanarViewingCoord();
        void SetTriplanarViewingCoord(ByteVolume::IndexType coord);
        Point3D GetTriplanarSelectedPointWorld();
        void SetCrosshairToViewingCoord();
        void Execute(vtkObject *caller, unsigned long id, void*);
        void TriplanarRender(int index);
        void TriplanarRender();
        QVTKWidget * GetTriplanarWidget(int index) {
            return &m_TriplanarWidget[index];
        }


// Probe eye view functions

        void ProbeEyeChangeOrientation(Point3D target, Point3D entry);
        void ProbeEyeNextSlice(bool sliceUp);
        void ProbeEyeRender();
        void ProbeEyeSetMIP(float thickness, bool max);
        void ProbeEyeGenerateCursor();
        void ProbeEyeUpdateCursor();



        QVTKWidget * GetProbeEyeWidget() {
            return &m_ProbeEyeWidget;
        }


        void RenderAll();

    private:


        //  private type for conversion from itk to vtk images
        typedef itk::ImageToVTKImageFilter<ByteVolume> GrayConnectorType;
        typedef itk::ImageToVTKImageFilter<RGBAVolume> RGBAConnectorType;

        struct VolumeInfo {
            vtkSmartPointer<vtkImageSlice> m_TriplanarImageSlice[3];
            vtkSmartPointer<vtkImageSliceMapper> m_TriplanarImageSliceMapper[3];

            vtkSmartPointer<vtkImageSlice> m_ProbeEyeImageSlice;
            vtkSmartPointer<vtkImageResliceMapper> m_ProbeEyeImageResliceMapper;
        };


        struct MainVolumeInfo : VolumeInfo {
            ByteVolume::Pointer m_Volume;
            GrayConnectorType::Pointer m_Connector;
        };
        struct OverlayVolumeInfo : VolumeInfo {
            RGBAVolume::Pointer m_Volume;
            RGBAConnectorType::Pointer m_Connector;
        };
        ByteVolume::IndexType m_ViewingCoord;
        ByteVolume::SizeType m_VolumeSize;

        std::map<std::string, OverlayVolumeInfo> m_OverlayVolumeData;
        MainVolumeInfo m_MainVolumeData;


        // Triplanar view
        vtkSmartPointer<vtkRenderer> m_TriplanarRenderer[3];
        vtkSmartPointer<vtkImageStack> m_TriplanarImageStack[3];
        QVTKWidget m_TriplanarWidget[3];
        vtkSmartPointer<TriplanarInteractorStyle> m_TriplanarInteractorStyles[3];
        vtkSmartPointer<vtkHandleWidget> m_CrosshairHandle[3];
        vtkSmartPointer<vtkPointHandleRepresentation3D> m_CrosshairHandleRep[3];


        // Probe eye view
        vtkSmartPointer<vtkRenderer> m_ProbeEyeRenderer;
        vtkSmartPointer<vtkRenderer> m_ProbeEyeOverlayRenderer;
        vtkSmartPointer<vtkImageStack> m_ProbeEyeImageStack;
        QVTKWidget m_ProbeEyeWidget;
        vtkSmartPointer<ProbeEyeInteractorStyle> m_ProbeEyeInteractorStyle;
        vtkSmartPointer<vtkPolyData> m_ProbeEyeCursorPolyData;
        vtkSmartPointer<vtkPolyDataMapper>  m_ProbeEyeCursorMapper;
        vtkSmartPointer<vtkActor> m_ProbeEyeCursorActor;
        Point3D m_ProbeEyeTarget;
        Point3D m_ProbeEyeEntry;

    };
}


#endif
