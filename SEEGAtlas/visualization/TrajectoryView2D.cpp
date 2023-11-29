// header files to include
#include "TrajectoryView2D.h"
#include <vtkTransform.h>
#include <vtkImageReslice.h>
#include <vtkImageActor.h>
#include <vtkObjectFactory.h>
#include <vtkProperty.h>
#include <vtkInteractorStyleImage.h>
#include <vtkRenderWindow.h>
#include <vtkCamera.h>
#include <vtkImageProperty.h>
#include <vtkPolyDataMapper.h>
#include <vtkPlane.h>
#include <vtkCellArray.h>
#include "MathUtils.h"

using namespace itk;
using namespace std;

namespace seeg {

    /*********************************************************************************************
     *  Implementation of TrajectoryView2DInteractorStyle class
     *********************************************************************************************/

    void TrajectoryView2DInteractorStyle::OnLeftButtonDown() {

        // disable pan the camera
        if (this->Interactor->GetShiftKey()) {
            return; // disable Panning the camera
        }
        this->Superclass::OnLeftButtonDown();
    }

    void TrajectoryView2DInteractorStyle::OnChar() {
        vtkRenderWindowInteractor *rwi = this->Interactor;

        // disabling reset to sagittal, coronal, axial views
        switch (rwi->GetKeyCode()) {
        case 'x':
        case 'X':
        case 'y':
        case 'Y':
        case 'z':
        case 'Z':
            return;
        }
        this->Superclass::OnChar();
    }


    vtkStandardNewMacro(TrajectoryView2DInteractorStyle);



    /*********************************************************************************************
     *  Implementation of TrajectoryView2D class
     *********************************************************************************************/


    /**** Constructors/Destructor ****/

    TrajectoryView2D::TrajectoryView2D(ByteVolume::Pointer vol) {
        this->m_Volume = vol;

        double *vect;
        m_InteractorStyle = vtkSmartPointer<TrajectoryView2DInteractorStyle>::New();
        m_InteractorStyle->SetObject(this);
        vect = m_InteractorStyle->GetXViewRightVector();
        vect[0] = 0; vect[1]=-1; vect[2]=0;
        vect = m_InteractorStyle->GetXViewUpVector();
        vect[0] = 0; vect[1]=0; vect[2]=1;
        Init();
        m_FirstInit = true;
    }


    TrajectoryView2D::~TrajectoryView2D() {
        // nothing to do
    }


    /**** ACCESSORS/SETTERS ****/


    vtkImageProperty * TrajectoryView2D::GetImageProperty() {
        return this->m_ImageSlice->GetProperty();
    }



    ByteVolume::Pointer TrajectoryView2D::GetVolume() {
        return this->m_Volume;
    }


    /**** PUBLIC FUNCTIONS ****/

    void TrajectoryView2D::Init() {

        vtkSmartPointer<vtkRenderWindow> window = m_Widget.renderWindow();

        // OPEN VOLUME
        m_Connector = GrayConnectorType::New();
        m_Connector->SetInput(m_Volume);
        m_Connector->Update();

        // CONFIGURE pipeline
        m_Renderer = vtkSmartPointer<vtkRenderer>::New();
        m_ImageSlice = vtkSmartPointer<vtkImageSlice>::New();
        m_ImageResliceMapper = vtkSmartPointer<vtkImageResliceMapper>::New();
        m_ImageResliceMapper->SetSlabThickness(1.0);
        m_ImageResliceMapper->SetInputData(m_Connector->GetOutput());
        m_ImageSlice->SetMapper(m_ImageResliceMapper);
        window->AddRenderer(m_Renderer);
        window->GetInteractor()->SetInteractorStyle(m_InteractorStyle);
        m_InteractorStyle->SetCurrentRenderer(m_Renderer);

        m_CursorPolyData =  vtkSmartPointer<vtkPolyData>::New();
        m_CursorMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
        m_CursorActor = vtkSmartPointer<vtkActor>::New();
        GenerateCursor();
        m_CursorMapper->SetInputData( m_CursorPolyData );
        m_CursorMapper->SetResolveCoincidentTopologyToShiftZBuffer();
        m_CursorActor->SetMapper( m_CursorMapper );
        m_CursorActor->PickableOff();
        m_CursorActor->GetProperty()->SetColor(0.0,0.0,1.0);


        m_Renderer->AddActor(m_CursorActor);
        m_Renderer->AddActor(m_ImageSlice); // add imageslice second to be able to change window level


        // Render adjust orientation so that it looks "good"
        m_InteractorStyle->SetImageOrientation(
                m_InteractorStyle->GetXViewRightVector(),
                m_InteractorStyle->GetXViewUpVector());
    }


    void TrajectoryView2D::ChangeOrientation(Point3D target, Point3D entry) {

        double currentCameraDistance;
        m_Target = target;
        m_Entry = entry;
        double up[3];
        Vector3D_lf targetVect(m_Target[0], m_Target[1], m_Target[2]);


        if (!m_FirstInit) {
            double* camPos = this->m_Renderer->GetActiveCamera()->GetPosition();
            Vector3D_lf camPosVect(camPos[0],camPos[1],camPos[2]);
            currentCameraDistance  = norm(camPosVect - targetVect);
        } else {
            currentCameraDistance = 800;
        }

        m_Renderer->GetActiveCamera()->SetPosition(m_Entry[0], m_Entry[1], m_Entry[2]);
        m_Renderer->GetActiveCamera()->SetFocalPoint(m_Target[0], m_Target[1], m_Target[2]);
        m_Renderer->GetActiveCamera()->OrthogonalizeViewUp();
        m_Renderer->GetActiveCamera()->GetViewUp(up);

        Vector3D_lf n(  m_Entry[0]-m_Target[0],
                        m_Entry[1]-m_Target[1],
                        m_Entry[2]-m_Target[2]);
        Vector3D_lf v1, v2;

        // step 1: Compute first vector
        v1.x = up[0];
        v1.y = up[1];
        v1.z = up[2];

        v2 = v1*n;

        vtkSmartPointer<vtkPlane> plane = vtkSmartPointer<vtkPlane>::New();
        plane->SetOrigin(m_Target[0], m_Target[1], m_Target[2]);
        plane->SetNormal(v2.x, v2.y, v2.z);
        m_ImageResliceMapper->SetSlicePlane(plane);

        v2 = v2 / norm(v2)* currentCameraDistance;

        m_Renderer->GetActiveCamera()->SetPosition(v2.x + m_Target[0],
                                                   v2.y + m_Target[1],
                                                   v2.z + m_Target[2]);
        m_Renderer->GetActiveCamera()->SetFocalPoint(m_Target[0], m_Target[1], m_Target[2]);
        this->UpdateCursor();

        m_FirstInit = false;
    }

    void TrajectoryView2D::Render() {
        m_Widget.interactor()->Render();
        m_Widget.renderWindow()->Render();
    }


    void TrajectoryView2D::GenerateCursor() {

        vtkPoints* points = vtkPoints::New(VTK_DOUBLE);
        points->SetNumberOfPoints(2);
        int i;
        for (i = 0; i < 2; i++)
        {
            points->SetPoint(i,0.0,0.0,0.0);
        }


        vtkCellArray *cells = vtkCellArray::New();
        cells->Allocate(cells->EstimateSize(1,2));
        vtkIdType pts[2];
        pts[0] = 0;
        pts[1] = 1;       // horizontal segment
        cells->InsertNextCell(2,pts);

        m_CursorPolyData->SetPoints(points);
        points->Delete();
        m_CursorPolyData->SetLines(cells);
        cells->Delete();
    }

    void TrajectoryView2D::UpdateCursor() {

        double p[3];

        vtkPoints * cursorPts = m_CursorPolyData->GetPoints();
        p[0] = m_Target[0];
        p[1] = m_Target[1];
        p[2] = m_Target[2];;
        cursorPts->SetPoint(0, p);

        p[0] = m_Entry[0];
        p[1] = m_Entry[1];
        p[2] = m_Entry[2];;
        cursorPts->SetPoint(1, p);

        m_CursorPolyData->Modified();
    }

}

