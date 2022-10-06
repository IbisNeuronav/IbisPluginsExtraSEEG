// header files to include
#include "ProbeEyeView.h"
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
     *  Implementation of ProbeEyeInteractorStyle class
     *********************************************************************************************/

    void ProbeEyeInteractorStyle::OnMouseWheelForward() {
        obj->NextSlice(true);
        obj->Render();
    }

    void ProbeEyeInteractorStyle::OnMouseWheelBackward() {
        obj->NextSlice(false);
        obj->Render();
    }


    void ProbeEyeInteractorStyle::OnLeftButtonDown() {

        // disable pan the camera
        if (this->Interactor->GetShiftKey()) {
            return; // disable Panning the camera
        }
        this->Superclass::OnLeftButtonDown();
    }

    void ProbeEyeInteractorStyle::OnLeftButtonUp() {
        this->Superclass::OnLeftButtonUp();

        if (this->CurrentImageProperty) {
            cout << "Color window: "<< this->CurrentImageProperty->GetColorWindow() << " Color level: " << this->CurrentImageProperty->GetColorLevel() << endl;
        }

    }


    void ProbeEyeInteractorStyle::OnChar() {
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

    vtkStandardNewMacro(ProbeEyeInteractorStyle);



    /*********************************************************************************************
     *  Implementation of ProbeEyeView class
     *********************************************************************************************/


    /**** Constructors/Destructor ****/

    ProbeEyeView::ProbeEyeView(ByteVolume::Pointer vol) {
        this->m_Volume = vol;

        double *vect;
        m_ProbeEyeInteractorStyle = vtkSmartPointer<ProbeEyeInteractorStyle>::New();
        m_ProbeEyeInteractorStyle->SetObject(this);
        vect = m_ProbeEyeInteractorStyle->GetXViewRightVector();
        vect[0] = 0; vect[1]=-1; vect[2]=0;
        vect = m_ProbeEyeInteractorStyle->GetXViewUpVector();
        vect[0] = 0; vect[1]=0; vect[2]=1;
        Init();
    }


    ProbeEyeView::~ProbeEyeView() {
        // nothing to do
    }


    /**** ACCESSORS/SETTERS ****/


    vtkImageProperty* ProbeEyeView::GetImageProperty() {
		return this->m_ProbeEyeImageSlice->GetProperty();
    }


    ByteVolume::Pointer ProbeEyeView::GetVolume() {
        return this->m_Volume;
    }


    double ProbeEyeView::GetCurrentDistanceToTarget() {
        if (m_FirstInit) {
            return 0;
        }

        vtkPlane * plane = m_ProbeEyeImageResliceMapper->GetSlicePlane();
        double *orig = plane->GetOrigin();
        Vector3D_lf target_vect(m_ProbeEyeTarget[0], m_ProbeEyeTarget[1], m_ProbeEyeTarget[2]);
        Vector3D_lf current_pos(orig[0], orig[1], orig[2]);
        return norm(current_pos - target_vect);
    }


    /**** PUBLIC FUNCTIONS ****/

    void ProbeEyeView::Init() {

        vtkSmartPointer<vtkRenderWindow> probeEyeWindow = m_ProbeEyeWidget.GetRenderWindow();

        // OPEN VOLUME
        m_Connector = GrayConnectorType::New();
        m_Connector->SetInput(m_Volume);
        m_Connector->Update();

        // CONFIGURE PROBE-EYE
        m_ProbeEyeRenderer = vtkSmartPointer<vtkRenderer>::New();
        m_ProbeEyeImageSlice = vtkSmartPointer<vtkImageSlice>::New();
        m_ProbeEyeImageResliceMapper = vtkSmartPointer<vtkImageResliceMapper>::New();
        m_ProbeEyeImageResliceMapper->SetSlabThickness(1.0);
        m_ProbeEyeImageResliceMapper->SetInputData(m_Connector->GetOutput());
        m_ProbeEyeImageSlice->SetMapper(m_ProbeEyeImageResliceMapper);

//        m_ProbeEyeImageResliceMapper->SetResampleToScreenPixels(0);

        probeEyeWindow->AddRenderer(m_ProbeEyeRenderer);
        probeEyeWindow->GetInteractor()->SetInteractorStyle(m_ProbeEyeInteractorStyle);
        m_ProbeEyeInteractorStyle->SetCurrentRenderer(m_ProbeEyeRenderer);

        m_ProbeEyeCursorPolyData =  vtkSmartPointer<vtkPolyData>::New();
        m_ProbeEyeCursorMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
        m_ProbeEyeCursorActor = vtkSmartPointer<vtkActor>::New();
        GenerateCursor();
        m_ProbeEyeCursorMapper->SetInputData( m_ProbeEyeCursorPolyData );
        m_ProbeEyeCursorMapper->SetResolveCoincidentTopologyToShiftZBuffer();
        m_ProbeEyeCursorActor->SetMapper( m_ProbeEyeCursorMapper );
        m_ProbeEyeCursorActor->PickableOff();
        m_ProbeEyeCursorActor->GetProperty()->SetColor(1.0, 0.0, 0.0);
        m_ProbeEyeCursorActor->GetProperty()->SetLineWidth(3);

        m_ProbeEyeRenderer->AddActor(m_ProbeEyeCursorActor);
        m_ProbeEyeRenderer->AddActor(m_ProbeEyeImageSlice);



        // Render adjust orientation so that it looks "good"
        m_ProbeEyeInteractorStyle->SetImageOrientation(
                m_ProbeEyeInteractorStyle->GetXViewRightVector(),
                m_ProbeEyeInteractorStyle->GetXViewUpVector());

        m_FirstInit = true;
    }

    void ProbeEyeView::SetSlabThickness(double thickness) {
        double oldThickness = m_ProbeEyeImageResliceMapper->GetSlabThickness();
        m_ProbeEyeImageResliceMapper->SetSlabThickness(thickness);

        // For MIPS: check that we don't go beyond the target because of
        // changes in the slab thickness
        RESLICE_MODE mode = (RESLICE_MODE)m_ProbeEyeImageResliceMapper->GetSlabType();
        if (thickness > oldThickness && (mode == RESLICE_MODE_MIN || mode == RESLICE_MODE_MAX)) {
            this->GoToSlice(this->GetCurrentDistanceToTarget());
        }
    }


    void ProbeEyeView::SetResliceMode(RESLICE_MODE mode) {
        m_ProbeEyeImageResliceMapper->SetSlabType(mode);
    }

    void ProbeEyeView::NextSlice(bool sliceUp) {

        double dist_to_target = GetCurrentDistanceToTarget();
        double path_length = GetMaxDistanceToTarget();
        double f = m_ProbeEyeImageResliceMapper->GetSlabThickness();
        double new_pos;


        if (sliceUp) {
            new_pos = dist_to_target + f;
        } else {
            new_pos = dist_to_target - f;
        }
        GoToSlice(new_pos);
   }


    void ProbeEyeView::GoToSlice(double distToTarget_world) {

        double maxPath = this->GetMaxDistanceToTarget();
        double f = m_ProbeEyeImageResliceMapper->GetSlabThickness();

        if (m_ProbeEyeImageResliceMapper->GetSlabType() == RESLICE_MODE_MIN ||
            m_ProbeEyeImageResliceMapper->GetSlabType() == RESLICE_MODE_MAX) {
            if (distToTarget_world > (maxPath - f/2.0)) {
                distToTarget_world = maxPath - f/2.0;
            }
            if (distToTarget_world < f/2.0) {
                distToTarget_world = f/2.0;
            }

        } else {
            if (distToTarget_world > (maxPath)) {
                distToTarget_world = maxPath;
            }
            if (distToTarget_world < 0) {
                distToTarget_world = 0;
            }
        }

        vtkPlane * plane = m_ProbeEyeImageResliceMapper->GetSlicePlane();
        Vector3D_lf target_vect(m_ProbeEyeTarget[0], m_ProbeEyeTarget[1], m_ProbeEyeTarget[2]);
        Vector3D_lf entry_vect(m_ProbeEyeEntry[0], m_ProbeEyeEntry[1], m_ProbeEyeEntry[2]);
        float path_length = norm(entry_vect-target_vect);
        Vector3D_lf unit_step = ((entry_vect-target_vect) / path_length);
        Vector3D_lf new_pos;

        new_pos = target_vect + unit_step*distToTarget_world;

        plane->SetOrigin(new_pos.x, new_pos.y, new_pos.z);
        UpdateCursor();
    }


    void ProbeEyeView::ChangeOrientation(Point3D target, Point3D entry) {


        double currentCameraDistance;
        double currentTargetDistance;

        if (!m_FirstInit) {
            double* camPos = this->m_ProbeEyeRenderer->GetActiveCamera()->GetPosition();
            Vector3D_lf camPosVect(camPos[0],camPos[1],camPos[2]);
            Vector3D_lf targetVect(m_ProbeEyeTarget[0], m_ProbeEyeTarget[1], m_ProbeEyeTarget[2]);
            currentCameraDistance  = norm(camPosVect - targetVect);
            currentTargetDistance = GetCurrentDistanceToTarget();
        } else {
            currentCameraDistance = 800;
            currentTargetDistance = 70;
        }

        m_ProbeEyeTarget = target;
        m_ProbeEyeEntry = entry;
        Point3D entry_far;

        Resize3DLineSegmentSingleSide(target, entry, 500, entry_far);
        vtkSmartPointer<vtkPlane> plane = vtkSmartPointer<vtkPlane>::New();
        plane->SetOrigin(target[0], target[1], target[2]);
        plane->SetNormal(entry_far[0], entry_far[1], entry_far[2]);
        m_ProbeEyeImageResliceMapper->SetSlicePlane(plane);
        this->GoToSlice(currentTargetDistance);

        Resize3DLineSegmentSingleSide(target, entry, currentCameraDistance, entry_far);
        this->m_ProbeEyeRenderer->GetActiveCamera()->SetFocalPoint(target[0], target[1], target[2]);
        this->m_ProbeEyeRenderer->GetActiveCamera()->SetPosition(entry_far[0], entry_far[1], entry_far[2]);
        this->m_ProbeEyeRenderer->GetActiveCamera()->OrthogonalizeViewUp();
        this->UpdateCursor();

        m_FirstInit = false;


    }

    void ProbeEyeView::Render() {
        m_ProbeEyeWidget.GetInteractor()->Render();
        m_ProbeEyeWidget.GetRenderWindow()->Render();
    }


    void ProbeEyeView::GenerateCursor() {

        vtkPoints* points = vtkPoints::New(VTK_DOUBLE);
        points->SetNumberOfPoints(4);
        int i;
        for (i = 0; i < 4; i++)
        {
            points->SetPoint(i,0.0,0.0,0.0);
        }

        vtkCellArray *cells = vtkCellArray::New();
        cells->Allocate(cells->EstimateSize(2,2));
        vtkIdType pts[2];
        pts[0] = 0;
        pts[1] = 1;       // horizontal segment
        cells->InsertNextCell(2,pts);
        pts[0] = 2;
        pts[1] = 3;       // vertical segment
        cells->InsertNextCell(2,pts);

        m_ProbeEyeCursorPolyData->SetPoints(points);
        points->Delete();
        m_ProbeEyeCursorPolyData->SetLines(cells);
        cells->Delete();
    }

    void ProbeEyeView::UpdateCursor() {

        double p[3];
        double up[3];
        double origin[3];

        m_ProbeEyeRenderer->GetActiveCamera()->GetViewUp(up);
        vtkPlane *plane = m_ProbeEyeImageResliceMapper->GetSlicePlane();
        plane->GetOrigin(origin);


        Vector3D_lf n(   m_ProbeEyeEntry[0]-m_ProbeEyeTarget[0],
                        m_ProbeEyeEntry[1]-m_ProbeEyeTarget[1],
                        m_ProbeEyeEntry[2]-m_ProbeEyeTarget[2]);
        Vector3D_lf v1, v2;

        // step 1: Compute first vector
        v1.x = up[0];
        v1.y = up[1];
        v1.z = up[2];

        // step 2: compute second vector as cross product of v1 and n
        v2 = v1*n;

        // v1 and v2 have same length
        v2 = v2 / norm(v2);
        v2 = v2*1000;
        v1 = v1 / norm(v1);
        v1 = v1*1000;

        vtkPoints * cursorPts = m_ProbeEyeCursorPolyData->GetPoints();
        p[0] = origin[0] + v1.x;
        p[1] = origin[1] + v1.y;
        p[2] = origin[2] + v1.z;
        cursorPts->SetPoint(0, p);

        p[0] = origin[0] - v1.x;
        p[1] = origin[1] - v1.y;
        p[2] = origin[2] - v1.z;
        cursorPts->SetPoint(1, p);

        p[0] = origin[0] + v2.x;
        p[1] = origin[1] + v2.y;
        p[2] = origin[2] + v2.z;
        cursorPts->SetPoint(2, p);

        p[0] = origin[0] - v2.x;
        p[1] = origin[1] - v2.y;
        p[2] = origin[2] - v2.z;
        cursorPts->SetPoint(3, p);

        m_ProbeEyeCursorPolyData->Modified();
    }

    double ProbeEyeView::GetMaxDistanceToTarget() {
        vtkPlane * plane = m_ProbeEyeImageResliceMapper->GetSlicePlane();
        Vector3D_lf target_vect(m_ProbeEyeTarget[0], m_ProbeEyeTarget[1], m_ProbeEyeTarget[2]);
        Vector3D_lf entry_vect(m_ProbeEyeEntry[0], m_ProbeEyeEntry[1], m_ProbeEyeEntry[2]);
        return norm(entry_vect-target_vect);
    }
}

