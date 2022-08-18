/**
 * @file BasicVolumeVisualizer2D.cpp
 *
 * Implementation of the BasicVolumeVisualizer2D class
 *
 * @author Silvain Beriault
 */

// TODO: should use vtkPointHandleRepresentation2D and use the ComputeWorldToDisplay method in the interactorStyle.


// header files to include
#include "BasicVolumeVisualizer2D.h"
#include <vtkTransform.h>
#include <vtkImageReslice.h>
#include <vtkImageActor.h>
#include <vtkObjectFactory.h>
#include <vtkProperty.h>
#include <vtkInteractorStyleTrackballCamera.h>
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
        obj->ProbeEyeNextSlice(true);
        obj->RenderAll();
    }

    void ProbeEyeInteractorStyle::OnMouseWheelBackward() {
        obj->ProbeEyeNextSlice(false);
        obj->RenderAll();
    }


    vtkStandardNewMacro(ProbeEyeInteractorStyle);


    /*********************************************************************************************
     *  Implementation of TriplanarInteractorStyle class
     *********************************************************************************************/

    void TriplanarInteractorStyle::OnMouseWheelForward() {
        ByteVolume::IndexType index = obj->GetTriplanarViewingCoord();
        ByteVolume::SizeType size = obj->GetVolumeSize();

        if (index[m_SliceDirection] < (size[m_SliceDirection]-1)) {
            index[m_SliceDirection]++;
        }
        obj->SetTriplanarViewingCoord(index);
        obj->SetCrosshairToViewingCoord();
        obj->RenderAll();
    }

    void TriplanarInteractorStyle::OnMouseWheelBackward() {
        ByteVolume::IndexType index = obj->GetTriplanarViewingCoord();
        ByteVolume::SizeType size = obj->GetVolumeSize();

        if (index[m_SliceDirection] > 0) {
            index[m_SliceDirection]--;
        }
        obj->SetTriplanarViewingCoord(index);
        obj->SetCrosshairToViewingCoord();
        obj->RenderAll();
    }


    vtkStandardNewMacro(TriplanarInteractorStyle);

    /*********************************************************************************************
     *  Implementation of BasicVolumeVisualizer2D class
     *********************************************************************************************/


    /**** Constructors/Destructor ****/

    BasicVolumeVisualizer2D::BasicVolumeVisualizer2D() {

        double *vect;

        for (int i=0; i<3; i++) {

            m_TriplanarInteractorStyles[i] = vtkSmartPointer<TriplanarInteractorStyle>::New();
            m_TriplanarInteractorStyles[i]->SetObject(this);
            m_TriplanarInteractorStyles[i]->SetSliceDirection(i);
            m_TriplanarInteractorStyles[i]->SetInteractionModeToImage2D();

            // rearrange such that left is left, right is right, top is top, bottom is bottom

            vect = m_TriplanarInteractorStyles[i]->GetXViewRightVector();
            vect[0] = 0; vect[1]=1; vect[2]=0;
            vect = m_TriplanarInteractorStyles[i]->GetXViewUpVector();
            vect[0] = 0; vect[1]=0; vect[2]=1;

            vect = m_TriplanarInteractorStyles[i]->GetYViewRightVector();
            vect[0] = 1; vect[1]=0; vect[2]=0;
            vect = m_TriplanarInteractorStyles[i]->GetYViewUpVector();
            vect[0] = 0; vect[1]=0; vect[2]=1;

            vect = m_TriplanarInteractorStyles[i]->GetZViewRightVector();
            vect[0] = 1; vect[1]=0; vect[2]=0;
            vect = m_TriplanarInteractorStyles[i]->GetZViewUpVector();
            vect[0] = 0; vect[1]=1; vect[2]=0;

        }

        m_ProbeEyeInteractorStyle = vtkSmartPointer<ProbeEyeInteractorStyle>::New();
        m_ProbeEyeInteractorStyle->SetObject(this);
        vect = m_ProbeEyeInteractorStyle->GetXViewRightVector();
        vect[0] = 0; vect[1]=-1; vect[2]=0;
        vect = m_ProbeEyeInteractorStyle->GetXViewUpVector();
        vect[0] = 0; vect[1]=0; vect[2]=1;
    }


    BasicVolumeVisualizer2D::~BasicVolumeVisualizer2D() {
        // nothing to do
    }


    /**** ACCESSORS/SETTERS ****/

    void BasicVolumeVisualizer2D::SetRGBAOverlayGlobalOpacity(const string& label, double opacity) {
        if (!RGBAOverlayVolumeExists(label)) {
            return;
        }

        for (int i=0; i<3; i++) {
            m_OverlayVolumeData[label].m_TriplanarImageSlice[i]->GetProperty()->SetOpacity(opacity);
        }
        m_OverlayVolumeData[label].m_ProbeEyeImageSlice->GetProperty()->SetOpacity(opacity);
    }



    ByteVolume::IndexType BasicVolumeVisualizer2D::GetTriplanarViewingCoord() {
        return this->m_ViewingCoord;
    }

    void BasicVolumeVisualizer2D::SetTriplanarViewingCoord(ByteVolume::IndexType coord) {
        m_ViewingCoord = coord;
        for (int i=0; i<3; i++) {
            m_MainVolumeData.m_TriplanarImageSliceMapper[i]->SetSliceNumber(m_ViewingCoord[i]);
        }

        map<string, OverlayVolumeInfo>::iterator it;
        for (it=m_OverlayVolumeData.begin(); it!= m_OverlayVolumeData.end(); it++) {
            for (int i=0; i<3; i++) {
                (*it).second.m_TriplanarImageSliceMapper[i]->SetSliceNumber(m_ViewingCoord[i]);
            }
        }
    }

    Point3D BasicVolumeVisualizer2D::GetTriplanarSelectedPointWorld() {
       Point3D point;
        m_MainVolumeData.m_Volume->TransformIndexToPhysicalPoint(m_ViewingCoord, point);
        return point;
    }


    void BasicVolumeVisualizer2D::Execute(vtkObject *caller, unsigned long id, void*) {
        double *curPoint;
        Point3D pointContinuous;
        Point3D pointDiscrete;
        ByteVolume::IndexType index;

        switch (id) {
        case vtkCommand::InteractionEvent:

            for (int i=0; i<3; i++) {

                if (caller == m_CrosshairHandle[i].GetPointer()) {

                    curPoint = m_CrosshairHandleRep[i]->GetWorldPosition();
                    pointContinuous[0] = curPoint[0];
                    pointContinuous[1] = curPoint[1];
                    pointContinuous[2] = curPoint[2];
                    this->m_MainVolumeData.m_Volume->TransformPhysicalPointToIndex(pointContinuous, index);

                    bool clamped = false;
                    for (int j=0; j<3; j++) {
                        if (index[j] < 0) {
                            index[j] = 0;
                            clamped = true;
                        }
                        if (index[j] >= this->m_VolumeSize[j]) {
                            index[j] = this->m_VolumeSize[j]-1;
                            clamped = true;
                        }
                    }

                    this->m_MainVolumeData.m_Volume->TransformIndexToPhysicalPoint(index, pointDiscrete);
                    if (clamped) {
                        pointContinuous = pointDiscrete;
                    }
                    this->SetTriplanarViewingCoord(index);
                    break;
                }
            }

            double p[3];

            // adjust to new crosshair position in all view
            for (int i=0; i<3; i++) {
                switch (i) {
                case 0:
                    p[0] = pointDiscrete[0];   // slice direction (otherwise, point disappear behind the slice)
                    p[1] = pointContinuous[1];
                    p[2] = pointContinuous[2];
                    break;
                case 1:
                    p[0] = pointContinuous[0];
                    p[1] = pointDiscrete[1];  // slice direction (otherwise, point disappear behind the slice)
                    p[2] = pointContinuous[2];
                    break;
                case 2:
                    p[0] = pointContinuous[0];
                    p[1] = pointContinuous[1];
                    p[2] = pointDiscrete[2];  // slice direction (otherwise, point disappear behind the slice)
                    break;
                }

                this->m_CrosshairHandleRep[i]->SetWorldPosition(p);

            }

            break;
        }

        RenderAll();

    }


    ByteVolume::SizeType BasicVolumeVisualizer2D::GetVolumeSize() {
        return m_VolumeSize;
    }


    ByteVolume::Pointer BasicVolumeVisualizer2D::GetMainVolume() {
        return m_MainVolumeData.m_Volume;
    }

    RGBAVolume::Pointer BasicVolumeVisualizer2D::GetRGBAOverlayVolume(const std::string& label) {
        if (RGBAOverlayVolumeExists(label)) {
            return m_OverlayVolumeData[label].m_Volume;
        }
        return RGBAVolume::Pointer();
    }


    /**** PUBLIC FUNCTIONS ****/


    void BasicVolumeVisualizer2D::SetMainVolume(ByteVolume::Pointer volume) {

        vtkSmartPointer<vtkRenderWindow> renderWindow[3];
        vtkSmartPointer<vtkRenderWindow> probeEyeWindow;


        // CLEANUP
        for (int i=0; i<3; i++) {
            renderWindow[i]= m_TriplanarWidget[i].GetRenderWindow();
            if (m_TriplanarRenderer[i]) {
                renderWindow[i]->RemoveRenderer(m_TriplanarRenderer[i]);
            }
            m_TriplanarRenderer[i] = vtkSmartPointer<vtkRenderer>::New();
            m_TriplanarImageStack[i] = vtkSmartPointer<vtkImageStack>::New();
        }

        probeEyeWindow = m_ProbeEyeWidget.GetRenderWindow();
        if (m_ProbeEyeRenderer) {
            probeEyeWindow->RemoveRenderer(m_ProbeEyeRenderer);
        }
        if (m_ProbeEyeOverlayRenderer) {
            probeEyeWindow->RemoveRenderer(m_ProbeEyeOverlayRenderer);
        }
        m_ProbeEyeRenderer = vtkSmartPointer<vtkRenderer>::New();
        m_ProbeEyeOverlayRenderer = vtkSmartPointer<vtkRenderer>::New();
        m_ProbeEyeRenderer->SetLayer(0);
        m_ProbeEyeOverlayRenderer->SetLayer(1);
        m_ProbeEyeOverlayRenderer->SetActiveCamera(m_ProbeEyeRenderer->GetActiveCamera());
        m_ProbeEyeImageStack = vtkSmartPointer<vtkImageStack>::New();
        m_OverlayVolumeData.clear();


        // OPEN VOLUME
        GrayConnectorType::Pointer conn = GrayConnectorType::New();
        conn->SetInput(volume);
        conn->Update();
        MainVolumeInfo info;
        info.m_Volume = volume;
        info.m_Connector = conn;


        // CONFIGURE TRIPLANAR VIEW
        for (int i=0; i<3; i++) {
            info.m_TriplanarImageSlice[i] = vtkSmartPointer<vtkImageSlice>::New();
            info.m_TriplanarImageSliceMapper[i] = vtkSmartPointer<vtkImageSliceMapper>::New();
            info.m_TriplanarImageSliceMapper[i]->SetOrientation(i);
            info.m_TriplanarImageSliceMapper[i]->SetInputData(conn->GetOutput());
            info.m_TriplanarImageSlice[i]->SetMapper(info.m_TriplanarImageSliceMapper[i]);
            m_TriplanarImageStack[i]->AddImage(info.m_TriplanarImageSlice[i]);
            m_TriplanarImageStack[i]->SetActiveLayer(0);
            m_TriplanarRenderer[i]->AddActor(m_TriplanarImageStack[i]);
            renderWindow[i]->AddRenderer(m_TriplanarRenderer[i]);
            renderWindow[i]->GetInteractor()->SetInteractorStyle(m_TriplanarInteractorStyles[i]);
            m_TriplanarInteractorStyles[i]->SetCurrentRenderer(m_TriplanarRenderer[i]);
        }

        // CONFIGURE PROBE-EYE
        info.m_ProbeEyeImageSlice = vtkSmartPointer<vtkImageSlice>::New();
        info.m_ProbeEyeImageResliceMapper = vtkSmartPointer<vtkImageResliceMapper>::New();
        info.m_ProbeEyeImageResliceMapper->SetSlabThickness(1.0);
        info.m_ProbeEyeImageResliceMapper->SetInputData(conn->GetOutput());
        info.m_ProbeEyeImageSlice->SetMapper(info.m_ProbeEyeImageResliceMapper);
        m_ProbeEyeImageStack->AddImage(info.m_ProbeEyeImageSlice);
        m_ProbeEyeImageStack->SetActiveLayer(0);
        m_ProbeEyeRenderer->AddActor(m_ProbeEyeImageStack);
        probeEyeWindow->AddRenderer(m_ProbeEyeRenderer);
        //probeEyeWindow->AddRenderer(m_ProbeEyeOverlayRenderer);
        probeEyeWindow->GetInteractor()->SetInteractorStyle(m_ProbeEyeInteractorStyle);
        m_ProbeEyeInteractorStyle->SetCurrentRenderer(m_ProbeEyeRenderer);

        m_ProbeEyeCursorPolyData=  vtkSmartPointer<vtkPolyData>::New();
        ProbeEyeGenerateCursor();
        m_ProbeEyeCursorMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
        m_ProbeEyeCursorActor = vtkSmartPointer<vtkActor>::New();
        m_ProbeEyeCursorMapper->SetInputData( this->m_ProbeEyeCursorPolyData );
        m_ProbeEyeCursorMapper->SetResolveCoincidentTopologyToShiftZBuffer();
        m_ProbeEyeCursorActor->SetMapper( m_ProbeEyeCursorMapper );
        m_ProbeEyeCursorActor->PickableOff();
        m_ProbeEyeRenderer->AddActor(m_ProbeEyeCursorActor);


        // SAVE INFO TO m_MainVolumeData
        m_MainVolumeData = info;


        // Render now (before setting the image orientation) so that camera is placed at a good distance
        // from the dataset
        RenderAll();

        for (int i=0; i<3; i++) {
            m_CrosshairHandle[i] = vtkSmartPointer<vtkHandleWidget>::New();
            m_CrosshairHandleRep[i] = vtkSmartPointer<vtkPointHandleRepresentation3D>::New();
            m_CrosshairHandle[i]->SetRepresentation(m_CrosshairHandleRep[i]);
            m_CrosshairHandle[i]->EnableAxisConstraintOff();
            m_CrosshairHandleRep[i]->GetProperty()->SetColor(0.0,0.0,1.0);
            m_CrosshairHandleRep[i]->GetProperty()->SetLineWidth(1.0);
            m_CrosshairHandleRep[i]->SetHandleSize(100);
            m_CrosshairHandleRep[i]->GetSelectedProperty()->SetColor(0.0, 0.0, 1.0);
            m_CrosshairHandleRep[i]->GetSelectedProperty()->SetLineWidth(1.0);
            m_CrosshairHandle[i]->AddObserver(vtkCommand::InteractionEvent, this);
            m_CrosshairHandle[i]->AddObserver(vtkCommand::EndInteractionEvent, this);
            m_CrosshairHandle[i]->SetInteractor(renderWindow[i]->GetInteractor());
            m_CrosshairHandle[i]->EnabledOn();
        }


        // adjust image orientation
        m_TriplanarInteractorStyles[0]->SetImageOrientation(
                m_TriplanarInteractorStyles[0]->GetXViewRightVector(),
                m_TriplanarInteractorStyles[0]->GetXViewUpVector());

        m_TriplanarInteractorStyles[1]->SetImageOrientation(
                m_TriplanarInteractorStyles[1]->GetYViewRightVector(),
                m_TriplanarInteractorStyles[1]->GetYViewUpVector());

        m_TriplanarInteractorStyles[2]->SetImageOrientation(
                m_TriplanarInteractorStyles[2]->GetZViewRightVector(),
                m_TriplanarInteractorStyles[2]->GetZViewUpVector());

        m_ProbeEyeInteractorStyle->SetImageOrientation(
                m_ProbeEyeInteractorStyle->GetXViewRightVector(),
                m_ProbeEyeInteractorStyle->GetXViewUpVector());



        ByteVolume::RegionType region = volume->GetLargestPossibleRegion();
        m_VolumeSize = region.GetSize();
        ByteVolume::IndexType viewingCoord;
        viewingCoord[0] = m_VolumeSize[0]/2;
        viewingCoord[1] = m_VolumeSize[1]/2;
        viewingCoord[2] = m_VolumeSize[2]/2;
        SetTriplanarViewingCoord(viewingCoord);
        SetCrosshairToViewingCoord();

        RenderAll();

    }

    void BasicVolumeVisualizer2D::ProbeEyeSetMIP(float thickness, bool max) {
        this->m_MainVolumeData.m_ProbeEyeImageResliceMapper->SetSlabThickness(thickness);
        if (max) {
            this->m_MainVolumeData.m_ProbeEyeImageResliceMapper->SetSlabTypeToMax();
        } else {
            this->m_MainVolumeData.m_ProbeEyeImageResliceMapper->SetSlabTypeToMin();
        }
    }



    void BasicVolumeVisualizer2D::AddRGBAOverlayVolume(const string& label, RGBAVolume::Pointer volume) {


        RemoveRGBAOverlayVolume(label);

        // convert itk image to VTK format
        RGBAConnectorType::Pointer conn = RGBAConnectorType::New();
        conn->SetInput(volume);
        conn->Update();



        OverlayVolumeInfo info;
        info.m_Volume = volume;
        info.m_Connector = conn;
        for (int i=0; i<3; i++) {
            info.m_TriplanarImageSlice[i] = vtkSmartPointer<vtkImageSlice>::New();
            info.m_TriplanarImageSliceMapper[i] = vtkSmartPointer<vtkImageSliceMapper>::New();

            info.m_ProbeEyeImageSlice = vtkSmartPointer<vtkImageSlice>::New();
            info.m_ProbeEyeImageResliceMapper = vtkSmartPointer<vtkImageResliceMapper>::New();
        }


        for (int i=0; i<3; i++) {
            info.m_TriplanarImageSliceMapper[i]->SetOrientation(i);
            info.m_TriplanarImageSliceMapper[i]->SetInputData(conn->GetOutput());
            info.m_TriplanarImageSlice[i]->SetMapper(info.m_TriplanarImageSliceMapper[i]);
            m_TriplanarImageStack[i]->AddImage(info.m_TriplanarImageSlice[i]);
        }

        info.m_ProbeEyeImageResliceMapper->SetInputData(conn->GetOutput());
        info.m_ProbeEyeImageResliceMapper->SetSlabThickness(1);
        info.m_ProbeEyeImageResliceMapper->SetSlabTypeToMax();
        info.m_ProbeEyeImageSlice->SetMapper(info.m_ProbeEyeImageResliceMapper);
        m_ProbeEyeImageStack->AddImage(info.m_ProbeEyeImageSlice);

        m_OverlayVolumeData[label] = info;

        SetTriplanarViewingCoord(m_ViewingCoord);
    }



    void BasicVolumeVisualizer2D::RemoveRGBAOverlayVolume(const std::string& label) {
        if (RGBAOverlayVolumeExists(label)) {
            for (int i=0; i<3; i++) {
                m_TriplanarImageStack[i]->RemoveImage(m_OverlayVolumeData[label].m_TriplanarImageSlice[i]);
            }
            m_OverlayVolumeData.erase(label);
        }

    }

    void BasicVolumeVisualizer2D::TriplanarRender() {
        for (int i=0; i<3; i++) {
            TriplanarRender(i);
        }
    }

    void BasicVolumeVisualizer2D::TriplanarRender(int index) {
        if (index < 0 || index>=3) {
            return;
        }
        m_TriplanarWidget[index].GetInteractor()->Render();
        m_TriplanarWidget[index].GetRenderWindow()->Render();
    }

    void BasicVolumeVisualizer2D::ProbeEyeNextSlice(bool sliceUp) {
        vtkPlane * plane = this->m_MainVolumeData.m_ProbeEyeImageResliceMapper->GetSlicePlane();

        double *orig = plane->GetOrigin();
        Vector3D_f target_vect(m_ProbeEyeTarget[0], m_ProbeEyeTarget[1], m_ProbeEyeTarget[2]);
        Vector3D_f entry_vect(m_ProbeEyeEntry[0], m_ProbeEyeEntry[1], m_ProbeEyeEntry[2]);
        float path_length = norm(entry_vect-target_vect);
        Vector3D_f unit_step = ((entry_vect-target_vect) / path_length);
        Vector3D_f current_pos(orig[0], orig[1], orig[2]);
        Vector3D_f new_pos;

        float f = 1;

        if (sliceUp) {
            new_pos = current_pos + unit_step*f;
            if (norm(new_pos-target_vect) > path_length) {
                new_pos = entry_vect;
            }
        } else {
            new_pos = current_pos - unit_step*f;
            if (norm(new_pos-entry_vect) > path_length) {
                new_pos = target_vect;
            }
        }

        plane->SetOrigin(new_pos.x, new_pos.y, new_pos.z);

        map<string, OverlayVolumeInfo>::iterator it;
        for (it=m_OverlayVolumeData.begin(); it!= m_OverlayVolumeData.end(); it++) {
            plane = (*it).second.m_ProbeEyeImageResliceMapper->GetSlicePlane();
            plane->SetOrigin(new_pos.x, new_pos.y, new_pos.z);
        }

        this->ProbeEyeUpdateCursor();
    }

    void BasicVolumeVisualizer2D::ProbeEyeChangeOrientation(Point3D target, Point3D entry) {

        m_ProbeEyeTarget = target;
        m_ProbeEyeEntry = entry;

        Point3D entry_far;
        Resize3DLineSegmentSingleSide(target, entry, 200, entry_far);


        cout << "target: " << m_ProbeEyeTarget[0] << " " << m_ProbeEyeTarget[1] << " " << m_ProbeEyeTarget[2] << endl;
        cout << "entry: " << m_ProbeEyeEntry[0] << " " << m_ProbeEyeEntry[1] << " " << m_ProbeEyeEntry[2] << endl;

        vtkSmartPointer<vtkPlane> plane = vtkSmartPointer<vtkPlane>::New();
        plane->SetOrigin(target[0], target[1], target[2]);
        plane->SetNormal(entry_far[0], entry_far[1], entry_far[2]);
        this->m_MainVolumeData.m_ProbeEyeImageResliceMapper->SetSlicePlane(plane);

        map<string, OverlayVolumeInfo>::iterator it;
        for (it=m_OverlayVolumeData.begin(); it!= m_OverlayVolumeData.end(); it++) {
            (*it).second.m_ProbeEyeImageResliceMapper->SetSlicePlane(plane);
        }

        this->m_ProbeEyeRenderer->GetActiveCamera()->SetPosition(entry_far[0], entry_far[1], entry_far[2]);
        this->m_ProbeEyeRenderer->GetActiveCamera()->SetFocalPoint(target[0], target[1], target[2]);
        this->m_ProbeEyeRenderer->GetActiveCamera()->OrthogonalizeViewUp();
        this->ProbeEyeUpdateCursor();

    }

    void BasicVolumeVisualizer2D::ProbeEyeRender() {
        m_ProbeEyeWidget.GetInteractor()->Render();
        m_ProbeEyeWidget.GetRenderWindow()->Render();
    }


    void BasicVolumeVisualizer2D::ProbeEyeGenerateCursor() {


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

        this->m_ProbeEyeCursorPolyData->SetPoints(points);
        points->Delete();
        this->m_ProbeEyeCursorPolyData->SetLines(cells);
        cells->Delete();
    }

    void BasicVolumeVisualizer2D::ProbeEyeUpdateCursor() {

        double p[3];
        double up[3];
        double origin[3];

        this->m_ProbeEyeRenderer->GetActiveCamera()->GetViewUp(up);
        vtkPlane *plane = this->m_MainVolumeData.m_ProbeEyeImageResliceMapper->GetSlicePlane();
        plane->GetOrigin(origin);


        Vector3D_f n(   m_ProbeEyeEntry[0]-m_ProbeEyeTarget[0],
                        m_ProbeEyeEntry[1]-m_ProbeEyeTarget[1],
                        m_ProbeEyeEntry[2]-m_ProbeEyeTarget[2]);
        Vector3D_f v1, v2;

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

        vtkPoints * cursorPts = this->m_ProbeEyeCursorPolyData->GetPoints();
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

        this->m_ProbeEyeCursorPolyData->Modified();
    }



    void BasicVolumeVisualizer2D::RenderAll() {
        ProbeEyeRender();
        TriplanarRender();
    }


    bool BasicVolumeVisualizer2D::RGBAOverlayVolumeExists(const std::string &label ) {
        return m_OverlayVolumeData.find(label) != m_OverlayVolumeData.end();
    }




    /**** PROTECTED AND PRIVATE FUNCTIONS ****/


    void BasicVolumeVisualizer2D::SetCrosshairToViewingCoord() {
        Point3D curWorld;
        m_MainVolumeData.m_Volume->TransformIndexToPhysicalPoint(this->m_ViewingCoord, curWorld);

        double worldPos[3];
        worldPos[0] = curWorld[0];
        worldPos[1] = curWorld[1];
        worldPos[2] = curWorld[2];
        for (int i=0; i<3; i++) {
            m_CrosshairHandleRep[i]->SetWorldPosition(worldPos);
        }

    }

}

