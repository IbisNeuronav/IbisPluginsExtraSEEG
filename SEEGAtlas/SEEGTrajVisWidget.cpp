#include "SEEGTrajVisWidget.h"
#include "SEEGFileHelper.h"
#include <QGridLayout>

using namespace itk;
using namespace seeg;
using namespace std;



SEEGTrajVisWidget::SEEGTrajVisWidget(QWidget *parent) : QWidget(parent) {
    m_UserSliceSelectConnection = vtkSmartPointer<vtkEventQtSlotConnect>::New();
    m_PreferredAngioSlabThickness = 5.0;
    Clear();
    this->setWindowFlags(Qt::WindowStaysOnTopHint);
    this->setWindowTitle(QString::fromStdString("Probe's Eye View"));
}

SEEGTrajVisWidget::~SEEGTrajVisWidget() {
    // do nothing
}

bool SEEGTrajVisWidget::HasDataToDisplay() {
    return m_HasDataToDisplay;
}


QSize SEEGTrajVisWidget::sizeHint () const {
    return QSize(900,500);
}


double SEEGTrajVisWidget::GetPreferredAngioSlabThickness() {
    return this->m_PreferredAngioSlabThickness;
}

void SEEGTrajVisWidget::SetPreferredAngioSlabThickness(double thickness) {
    m_PreferredAngioSlabThickness = thickness;

    if (m_GadoAngioProbeView.get()) {
        m_GadoAngioProbeView->SetSlabThickness(m_PreferredAngioSlabThickness);
        m_GadoAngioProbeView->Render();
    }

    if (m_CtaAngioProbeView.get()) {
        m_CtaAngioProbeView->SetSlabThickness(m_PreferredAngioSlabThickness);
        m_CtaAngioProbeView->Render();
    }
}

void SEEGTrajVisWidget::Configure(TRAJ_VIS_MODE mode) {
    Clear();
    QGridLayout *layout = new QGridLayout(this);
    layout->setSpacing(2);

    switch(mode) {
    case TRAJ_VIS_MODE_MULTI_MODAL:
        //ANGIO on top  T1 on bottom
        this->ConfigureT1ProbeView();
        if (m_T1ProbeView.get())  {
            layout->addWidget(m_T1ProbeView->GetProbeEyeWidget(),1,0);
        }
        this->ConfigureT1TrajView();
        if (m_T1TrajView.get()) {
            layout->addWidget(m_T1TrajView->GetTrajectoryViewWidget(),1,1);
        }
        this->ConfigureGadoAngioProbeView();
        if (m_GadoAngioProbeView.get()) {
            layout->addWidget(m_GadoAngioProbeView->GetProbeEyeWidget(),0,0);
        }
        this->ConfigureCtaAngioProbeView();
        if (m_CtaAngioProbeView.get()) {
            layout->addWidget(m_CtaAngioProbeView->GetProbeEyeWidget(),0,1);
        }
        break;

    case TRAJ_VIS_MODE_CTA:
        this->ConfigureCtaProbeView();
        if (m_CtaProbeView.get()) {
            layout->addWidget(m_CtaProbeView->GetProbeEyeWidget(),0,0);
        }
        this->ConfigureCtaAngioProbeView();
        if (m_CtaAngioProbeView.get()) {
            layout->addWidget(m_CtaAngioProbeView->GetProbeEyeWidget(),0,1);
        }
        this->ConfigureCtaTrajView();
        if (m_CtaTrajView.get()) {
            layout->addWidget(m_CtaTrajView->GetTrajectoryViewWidget(),1,1);
        }
       break;

    case TRAJ_VIS_MODE_GADO:
        this->ConfigureGadoProbeView();
        if (m_GadoProbeView.get()) {
            layout->addWidget(m_GadoProbeView->GetProbeEyeWidget(),0,0);
        }
        this->ConfigureGadoAngioProbeView();
        if (m_GadoAngioProbeView.get()) {
            layout->addWidget(m_GadoAngioProbeView->GetProbeEyeWidget(),0,1);
        }
        this->ConfigureGadoTrajView();
        if (m_GadoTrajView.get()) {
            layout->addWidget(m_GadoTrajView->GetTrajectoryViewWidget(),1,1);
        }
       break;

    case TRAJ_VIS_MODE_T1:
        this->ConfigureT1ProbeView();
        if (m_T1ProbeView.get())  {
            layout->addWidget(m_T1ProbeView->GetProbeEyeWidget(),0,0);
        }
        this->ConfigureT1TrajView();
        if (m_T1TrajView.get()) {
            layout->addWidget(m_T1TrajView->GetTrajectoryViewWidget(),0,1);
        }
        break;

    case TRAJ_VIS_MODE_ALL_ANGIOS:
        this->ConfigureGadoAngioProbeView();
        if (m_GadoAngioProbeView.get()) {
            layout->addWidget(m_GadoAngioProbeView->GetProbeEyeWidget(),0,0);
        }
        this->ConfigureCtaAngioProbeView();
        if (m_CtaAngioProbeView.get()) {
            layout->addWidget(m_CtaAngioProbeView->GetProbeEyeWidget(),0,1);
        }
        break;

    case TRAJ_VIS_MODE_CUSTOM:
        this->ConfigureT1ProbeView();
        if (m_T1ProbeView.get())  {
            layout->addWidget(m_T1ProbeView->GetProbeEyeWidget(),1,0);
        }
        this->ConfigureGadoAngioProbeView();
        if (m_GadoAngioProbeView.get()) {
            layout->addWidget(m_GadoAngioProbeView->GetProbeEyeWidget(),0,0);
        }
        this->ConfigureCtaAngioProbeView();
        if (m_CtaAngioProbeView.get()) {
            layout->addWidget(m_CtaAngioProbeView->GetProbeEyeWidget(),0,1);
        }
        break;
    }

    this->setLayout(layout);
    this->show();
}

void SEEGTrajVisWidget::OnUserSliceSelect(vtkObject* caller) {
    double distToTarget = 90;

    if (m_T1ProbeView.get()) {
        if (caller == this->m_T1ProbeView->GetProbeEyeWidget()->GetInteractor()) {
            distToTarget = m_T1ProbeView->GetCurrentDistanceToTarget();
        }
    }

    if (m_GadoProbeView.get()) {
        if (caller == this->m_GadoProbeView->GetProbeEyeWidget()->GetInteractor()) {
            distToTarget = m_GadoProbeView->GetCurrentDistanceToTarget();
        }
    }


    if (m_GadoAngioProbeView.get()) {
        if (caller == this->m_GadoAngioProbeView->GetProbeEyeWidget()->GetInteractor()) {
            distToTarget = m_GadoAngioProbeView->GetCurrentDistanceToTarget();
        }
    }

    if (m_CtaProbeView.get()) {
        if (caller == this->m_CtaProbeView->GetProbeEyeWidget()->GetInteractor()) {
            distToTarget = m_CtaProbeView->GetCurrentDistanceToTarget();
        }
    }


    if (m_CtaAngioProbeView.get()) {
        if (caller == this->m_CtaAngioProbeView->GetProbeEyeWidget()->GetInteractor()) {
            distToTarget = m_CtaAngioProbeView->GetCurrentDistanceToTarget();
        }
    }

    if (m_T1ProbeView.get() && caller != m_T1ProbeView->GetProbeEyeWidget()->GetInteractor()) {
        m_T1ProbeView->GoToSlice(distToTarget);
        m_T1ProbeView->Render();
    }


    if (m_GadoProbeView.get() && caller != m_GadoProbeView->GetProbeEyeWidget()->GetInteractor()) {
        m_GadoProbeView->GoToSlice(distToTarget);
        m_GadoProbeView->Render();
    }

    if (m_GadoAngioProbeView.get() && caller != m_GadoAngioProbeView->GetProbeEyeWidget()->GetInteractor()) {
        m_GadoAngioProbeView->GoToSlice(distToTarget);
        m_GadoAngioProbeView->Render();
    }

    if (m_CtaProbeView.get() && caller != m_CtaProbeView->GetProbeEyeWidget()->GetInteractor()) {
        m_CtaProbeView->GoToSlice(distToTarget);
        m_CtaProbeView->Render();
    }

    if (m_CtaAngioProbeView.get() && caller != m_CtaAngioProbeView->GetProbeEyeWidget()->GetInteractor()) {
        m_CtaAngioProbeView->GoToSlice(distToTarget);
        m_CtaAngioProbeView->Render();
    }
}


void SEEGTrajVisWidget::ShowTrajectory(double target[3], double entry[3]) {
    Point3D e,t;
    for (int i=0; i<3; i++) {
        e[i]=entry[i];
        t[i] = target[i];
    }
    ShowTrajectory(t,e);
}

void SEEGTrajVisWidget::ShowTrajectory(Point3D target, Point3D entry) {
    GeneralTransform::Pointer volToRef;
    Point3D target_xfm;
    Point3D entry_xfm;


    /**** T1 dataset ****/

    if (m_T1ToRef.get()) {
        m_T1ToRef->TransformPointInv(target, target_xfm);
        m_T1ToRef->TransformPointInv(entry, entry_xfm);
    } else {
        target_xfm = target;
        entry_xfm = entry;
    }

    if (m_T1ProbeView.get()) {
        m_T1ProbeView->ChangeOrientation(target_xfm, entry_xfm);
        m_T1ProbeView->Render();
    }

    if (m_T1TrajView.get()) {
        m_T1TrajView->ChangeOrientation(target_xfm, entry_xfm);
        m_T1TrajView->Render();
    }


    /**** GADO dataset ****/
    if (m_GadoToRef.get()) {
        m_GadoToRef->TransformPointInv(target, target_xfm);
        m_GadoToRef->TransformPointInv(entry, entry_xfm);
    } else {
        target_xfm = target;
        entry_xfm = entry;
    }

    if (m_GadoProbeView.get()) {
        m_GadoProbeView->ChangeOrientation(target_xfm, entry_xfm);
        m_GadoProbeView->Render();
    }

    if (m_GadoAngioProbeView.get()) {
        m_GadoAngioProbeView->ChangeOrientation(target_xfm, entry_xfm);
        m_GadoAngioProbeView->Render();
    }

    if (m_GadoTrajView.get()) {
        m_GadoTrajView->ChangeOrientation(target_xfm, entry_xfm);
        m_GadoTrajView->Render();
    }

    /**** CTA dataset ****/
    if (m_CtaToRef.get()) {
        m_CtaToRef->TransformPointInv(target, target_xfm);
        m_CtaToRef->TransformPointInv(entry, entry_xfm);
    } else {
        target_xfm = target;
        entry_xfm = entry;
    }

    if (m_CtaProbeView.get()) {
        m_CtaProbeView->ChangeOrientation(target_xfm, entry_xfm);
        m_CtaProbeView->Render();
    }

    if (m_CtaAngioProbeView.get()) {
        m_CtaAngioProbeView->ChangeOrientation(target_xfm, entry_xfm);
        m_CtaAngioProbeView->Render();
    }

    if (m_CtaTrajView.get()) {
        m_CtaTrajView->ChangeOrientation(target_xfm, entry_xfm);
        m_CtaTrajView->Render();
    }


}


void SEEGTrajVisWidget::ConfigureT1ProbeView() {
    FloatVolume::Pointer t1Vol = seeg::OpenFloatVolume(VOL_GROUP_T1, VOL_T1_PRE);
    if (t1Vol.IsNotNull()) {
        m_HasDataToDisplay = true;
        m_T1ProbeView = ProbeEyeView::New(ConvertToByteVolume(t1Vol));
        m_T1ProbeView->SetResliceMode(RESLICE_MODE_MEAN);
        m_T1ProbeView->SetSlabThickness(1.0);

        vtkImageProperty * prop = m_T1ProbeView->GetImageProperty();
        prop->SetColorWindow(100);
        prop->SetColorLevel(55);


        m_UserSliceSelectConnection->Connect(   m_T1ProbeView->GetProbeEyeWidget()->GetInteractor(),
                                                vtkCommand::MouseWheelBackwardEvent,
                                                this,
                                                SLOT(OnUserSliceSelect(vtkObject*)));

        m_UserSliceSelectConnection->Connect(   m_T1ProbeView->GetProbeEyeWidget()->GetInteractor(),
                                                vtkCommand::MouseWheelForwardEvent,
                                                this,
                                                SLOT(OnUserSliceSelect(vtkObject*)));

    }
}



void SEEGTrajVisWidget::ConfigureGadoProbeView() {
    FloatVolume::Pointer gadoVol = seeg::OpenFloatVolume(VOL_GROUP_GADO, VOL_GADO_RAW);
    if (gadoVol.IsNotNull()) {
        m_HasDataToDisplay = true;
        m_GadoProbeView = ProbeEyeView::New(ConvertToByteVolume(gadoVol));
        m_GadoProbeView->SetResliceMode(RESLICE_MODE_MEAN);
        m_GadoProbeView->SetSlabThickness(1.0);

        vtkImageProperty * prop = m_GadoProbeView->GetImageProperty();
        prop->SetColorWindow(50);
        prop->SetColorLevel(43);


        m_UserSliceSelectConnection->Connect(   m_GadoProbeView->GetProbeEyeWidget()->GetInteractor(),
                                                vtkCommand::MouseWheelBackwardEvent,
                                                this,
                                                SLOT(OnUserSliceSelect(vtkObject*)));

        m_UserSliceSelectConnection->Connect(   m_GadoProbeView->GetProbeEyeWidget()->GetInteractor(),
                                                vtkCommand::MouseWheelForwardEvent,
                                                this,
                                                SLOT(OnUserSliceSelect(vtkObject*)));

    }
}

void SEEGTrajVisWidget::ConfigureGadoAngioProbeView() {
    FloatVolume::Pointer gadoVol = seeg::OpenFloatVolume(VOL_GROUP_GADO, VOL_GADO_RAW);
    if (gadoVol.IsNotNull()) {
        m_HasDataToDisplay = true;
        m_GadoAngioProbeView = ProbeEyeView::New(ConvertToByteVolume(gadoVol));
        m_GadoAngioProbeView->SetResliceMode(RESLICE_MODE_MAX);
        m_GadoAngioProbeView->SetSlabThickness(this->m_PreferredAngioSlabThickness);

        vtkImageProperty * prop = m_GadoAngioProbeView->GetImageProperty();
        prop->SetColorWindow(50);
        prop->SetColorLevel(43);


        m_UserSliceSelectConnection->Connect(   m_GadoAngioProbeView->GetProbeEyeWidget()->GetInteractor(),
                                                vtkCommand::MouseWheelBackwardEvent,
                                                this,
                                                SLOT(OnUserSliceSelect(vtkObject*)));

        m_UserSliceSelectConnection->Connect(   m_GadoAngioProbeView->GetProbeEyeWidget()->GetInteractor(),
                                                vtkCommand::MouseWheelForwardEvent,
                                                this,
                                                SLOT(OnUserSliceSelect(vtkObject*)));
    }
}

void SEEGTrajVisWidget::ConfigureCtaProbeView() {
    FloatVolume::Pointer ctaVol = seeg::OpenFloatVolume(VOL_GROUP_CTA, VOL_CTA_RAW_NATIVE);
    if (ctaVol.IsNotNull()) {
        m_HasDataToDisplay = true;
        m_CtaProbeView = ProbeEyeView::New(ConvertToByteVolume(ctaVol));
        m_CtaProbeView->SetResliceMode(RESLICE_MODE_MEAN);
        m_CtaProbeView->SetSlabThickness(1.0);

        vtkImageProperty * prop = m_CtaProbeView->GetImageProperty();
        prop->SetColorWindow(50);
        prop->SetColorLevel(43);


        m_UserSliceSelectConnection->Connect(   m_CtaProbeView->GetProbeEyeWidget()->GetInteractor(),
                                                vtkCommand::MouseWheelBackwardEvent,
                                                this,
                                                SLOT(OnUserSliceSelect(vtkObject*)));

        m_UserSliceSelectConnection->Connect(   m_CtaProbeView->GetProbeEyeWidget()->GetInteractor(),
                                                vtkCommand::MouseWheelForwardEvent,
                                                this,
                                                SLOT(OnUserSliceSelect(vtkObject*)));

    }
}

void SEEGTrajVisWidget::ConfigureCtaAngioProbeView() {
    FloatVolume::Pointer ctaVol = seeg::OpenFloatVolume(VOL_GROUP_CTA, VOL_CTA_RAW_NATIVE);
    if (ctaVol.IsNotNull()) {
        m_HasDataToDisplay = true;
        m_CtaAngioProbeView = ProbeEyeView::New(ConvertToByteVolume(ctaVol));
        m_CtaAngioProbeView->SetResliceMode(RESLICE_MODE_MAX);
        m_CtaAngioProbeView->SetSlabThickness(this->m_PreferredAngioSlabThickness);

        vtkImageProperty * prop = m_CtaAngioProbeView->GetImageProperty();
        prop->SetColorWindow(50);
        prop->SetColorLevel(43);


        m_UserSliceSelectConnection->Connect(   m_CtaAngioProbeView->GetProbeEyeWidget()->GetInteractor(),
                                                vtkCommand::MouseWheelBackwardEvent,
                                                this,
                                                SLOT(OnUserSliceSelect(vtkObject*)));

        m_UserSliceSelectConnection->Connect(   m_CtaAngioProbeView->GetProbeEyeWidget()->GetInteractor(),
                                                vtkCommand::MouseWheelForwardEvent,
                                                this,
                                                SLOT(OnUserSliceSelect(vtkObject*)));
    }
}

void SEEGTrajVisWidget::ConfigureT1TrajView() {
    FloatVolume::Pointer t1Vol = seeg::OpenFloatVolume(VOL_GROUP_T1, VOL_T1_PRE);
    if (t1Vol.IsNotNull()) {
        m_HasDataToDisplay = true;
        m_T1TrajView = TrajectoryView2D::New(ConvertToByteVolume(t1Vol));

        vtkImageProperty * prop = m_T1TrajView->GetImageProperty();
        prop->SetColorWindow(100);
        prop->SetColorLevel(55);
    }
}

void SEEGTrajVisWidget::ConfigureGadoTrajView() {
    FloatVolume::Pointer gadoVol = seeg::OpenFloatVolume(VOL_GROUP_GADO, VOL_GADO_RAW);
    if (gadoVol.IsNotNull()) {
        m_HasDataToDisplay = true;
        m_GadoTrajView = TrajectoryView2D::New(ConvertToByteVolume(gadoVol));

        vtkImageProperty * prop = m_GadoTrajView->GetImageProperty();
        prop->SetColorWindow(50);
        prop->SetColorLevel(43);
    }
}

void SEEGTrajVisWidget::ConfigureCtaTrajView() {
    FloatVolume::Pointer ctaVol = seeg::OpenFloatVolume(VOL_GROUP_CTA, VOL_CTA_RAW_NATIVE);
    if (ctaVol.IsNotNull()) {
        m_HasDataToDisplay = true;
        m_CtaTrajView = TrajectoryView2D::New(ConvertToByteVolume(ctaVol));

        vtkImageProperty * prop = m_CtaTrajView->GetImageProperty();
        prop->SetColorWindow(50);
        prop->SetColorLevel(43);
    }
}

ByteVolume::Pointer SEEGTrajVisWidget::ConvertToByteVolume(FloatVolume::Pointer vol) {
    m_FloatToByteVolumeFilter = RescaleFilterType::New();
    m_FloatToByteVolumeFilter->SetOutputMinimum(0);
    m_FloatToByteVolumeFilter->SetOutputMaximum(255);
    m_FloatToByteVolumeFilter->SetInput(vol);
    m_FloatToByteVolumeFilter->Update();
    ByteVolume::Pointer out = m_FloatToByteVolumeFilter->GetOutput();
    out->DisconnectPipeline();
    return out;
}


void SEEGTrajVisWidget::Clear() {

    m_UserSliceSelectConnection->Disconnect();

    QLayout *layout = this->layout();
    if (layout) delete layout;

    m_T1ProbeView = ProbeEyeView::Pointer();
    m_GadoProbeView = ProbeEyeView::Pointer();
    m_T1TrajView = TrajectoryView2D::Pointer();
    m_GadoTrajView = TrajectoryView2D::Pointer();

    m_HasDataToDisplay = false;

    m_T1ToRef = OpenTransform(VOL_GROUP_T1);
    m_GadoToRef = OpenTransform(VOL_GROUP_GADO);
    m_CtaToRef = OpenTransform(VOL_GROUP_CTA);
}

