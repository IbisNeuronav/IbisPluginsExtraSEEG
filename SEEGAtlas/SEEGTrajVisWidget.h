#ifndef __TRAJ_VIS_WIDGET_H__
#define __TRAJ_VIS_WIDGET_H__

#include <QtWidgets/QWidget>
#include <vector>
#include "ProbeEyeView.h"
#include "TrajectoryView2D.h"
#include "VolumeTypes.h"
#include "GeneralTransform.h"
#include "itkRescaleIntensityImageFilter.h"
#include <vtkEventQtSlotConnect.h>


enum TRAJ_VIS_MODE {
    TRAJ_VIS_MODE_MULTI_MODAL = 0,
    TRAJ_VIS_MODE_CTA,
    TRAJ_VIS_MODE_GADO,
    TRAJ_VIS_MODE_T1,
    //TRAJ_VIS_MODE_SWI,
    //TRAJ_VIS_MODE_TOF,
    TRAJ_VIS_MODE_ALL_ANGIOS,
    TRAJ_VIS_MODE_CUSTOM
};


class SEEGTrajVisWidget : public QWidget {

    Q_OBJECT

public:
    SEEGTrajVisWidget(QWidget *parent = 0);
    ~SEEGTrajVisWidget();


    void Configure(TRAJ_VIS_MODE mode);

    virtual QSize sizeHint () const;

    void ShowTrajectory(double target[3], double entry[3]);
    void ShowTrajectory(seeg::Point3D target, seeg::Point3D entry);

    bool HasDataToDisplay();

    double GetPreferredAngioSlabThickness();
    void SetPreferredAngioSlabThickness(double thickness);

    void Clear();

private slots:
    void OnUserSliceSelect(vtkObject* caller);

private:

    typedef itk::RescaleIntensityImageFilter<seeg::FloatVolume,seeg::ByteVolume> RescaleFilterType;

 //   void AutoAdjustSWISliceThickness();

    void ConfigureT1ProbeView();
 //   void ConfigureSWIProbeView();
//    void ConfigureTOFProbeView();
    void ConfigureGadoProbeView();
    void ConfigureGadoAngioProbeView();
    void ConfigureCtaProbeView();
    void ConfigureCtaAngioProbeView();

    void ConfigureT1TrajView();
    void ConfigureGadoTrajView();
    void ConfigureCtaTrajView();


    seeg::ByteVolume::Pointer ConvertToByteVolume(seeg::FloatVolume::Pointer vol);

    RescaleFilterType::Pointer m_FloatToByteVolumeFilter;

    seeg::ProbeEyeView::Pointer m_T1ProbeView;
   // seeg::ProbeEyeView::Pointer m_SWIProbeView;
  //  seeg::ProbeEyeView::Pointer m_TOFProbeView;
    seeg::ProbeEyeView::Pointer m_GadoProbeView;
    seeg::ProbeEyeView::Pointer m_GadoAngioProbeView;

    seeg::ProbeEyeView::Pointer m_CtaProbeView;
    seeg::ProbeEyeView::Pointer m_CtaAngioProbeView;


    seeg::TrajectoryView2D::Pointer m_T1TrajView;
    seeg::TrajectoryView2D::Pointer m_GadoTrajView;
    seeg::TrajectoryView2D::Pointer m_CtaTrajView;


    seeg::GeneralTransform::Pointer m_T1ToRef;
    //seeg::GeneralTransform::Pointer m_SWIToRef;
    //seeg::GeneralTransform::Pointer m_TOFToRef;
    seeg::GeneralTransform::Pointer m_GadoToRef;
    seeg::GeneralTransform::Pointer m_CtaToRef;

    vtkSmartPointer<vtkEventQtSlotConnect> m_UserSliceSelectConnection;

    bool m_HasDataToDisplay;

    double m_PreferredAngioSlabThickness;

};

#endif
