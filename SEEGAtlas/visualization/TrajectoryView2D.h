#ifndef __TRAJECTORY_VIEW_2D_H__
#define __TRAJECTORY_VIEW_2D_H__

// header files to include
#include <vtkSmartPointer.h>
#include "BasicTypes.h"
#include "VolumeTypes.h"
#include "itkImageToVTKImageFilter.h"
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkImageResliceMapper.h>
#include <vtkImageActor.h>
#include <vtkInteractorStyleImage.h>
#include <vtkPolyData.h>
#include <QVTKRenderWidget.h>
#include <vtkImageProperty.h>
#include <vector>


namespace seeg {


    class TrajectoryView2D;

    class TrajectoryView2DInteractorStyle : public vtkInteractorStyleImage {
    public:

        static TrajectoryView2DInteractorStyle* New();

        vtkTypeMacro(TrajectoryView2DInteractorStyle, vtkInteractorStyleImage);


        void SetObject(TrajectoryView2D *obj) {
            this->obj = obj;
        }

        virtual void OnMiddleButtonDown() { }
        virtual void OnMiddleButtonUp() { }
        virtual void OnLeftButtonDown();
        virtual void OnChar();

    protected:
        TrajectoryView2D *obj;
    };



    class TrajectoryView2D {

    public:
        typedef mrilSmartPtr<TrajectoryView2D> Pointer;

        static Pointer New(ByteVolume::Pointer volume) {
            return Pointer(new TrajectoryView2D(volume));
        }

    protected:

        TrajectoryView2D(ByteVolume::Pointer volume);

    public:

        virtual ~TrajectoryView2D();


        vtkImageProperty * GetImageProperty();


        QVTKRenderWidget * GetTrajectoryViewWidget() {
            return &m_Widget;
        }
        ByteVolume::Pointer GetVolume();


// Probe eye view functions

        void ChangeOrientation(Point3D target, Point3D entry);
        void Render();
    
 private:

        void Init();
        void GenerateCursor();
        void UpdateCursor();

    private:

        //  private type for conversion from itk to vtk images
        typedef itk::ImageToVTKImageFilter<ByteVolume> GrayConnectorType;

        // original volume
        ByteVolume::Pointer m_Volume;

        // vtk pipeline for trajectory view
        GrayConnectorType::Pointer m_Connector;
        vtkSmartPointer<vtkImageSlice> m_ImageSlice;
        vtkSmartPointer<vtkImageResliceMapper> m_ImageResliceMapper;
        vtkSmartPointer<vtkRenderer> m_Renderer;
        QVTKRenderWidget m_Widget;
        vtkSmartPointer<TrajectoryView2DInteractorStyle> m_InteractorStyle;

        // cursor
        vtkSmartPointer<vtkPolyData> m_CursorPolyData;
        vtkSmartPointer<vtkPolyDataMapper>  m_CursorMapper;
        vtkSmartPointer<vtkActor> m_CursorActor;

        // current target and entry point
        Point3D m_Target;
        Point3D m_Entry;

        bool m_FirstInit;

    };
}


#endif
