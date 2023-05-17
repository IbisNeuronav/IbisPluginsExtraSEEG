#include <QString>
#include <QObject>


#include "SEEGPointRepresentation.h"

namespace seeg {

SEEGPointRepresentation::SEEGPointRepresentation(SceneObject * parent)
{
    m_points = PointsObject::New();
    if( parent )
    {
        m_points->setParent(parent);
        // TODO: set parent color
        //m_points->SetEnabledColor(parent->GetC)
    }
    m_points->SetCanChangeParent(false);
    m_points->SetCanAppendChildren(false);
    m_points->SetCanEditTransformManually(false);
    m_points->SetPickabilityLocked(true);
    m_points->SetNameChangeable(false);
    m_points->ShowLabels(false);
    m_points->Set3DRadius(0);
    m_points->SetHidden(true);
}

SEEGPointRepresentation::~SEEGPointRepresentation()
{
    m_points->Delete();
}

void SEEGPointRepresentation::SetName(std::string name)
{
    m_points->SetName(QString(name.c_str()));
}

std::string SEEGPointRepresentation::GetName()
{
    return m_points->GetName().toStdString();
}

int SEEGPointRepresentation::InsertNextPoint(const double pos[3])
{
    return this->InsertNextPoint(pos[0], pos[1], pos[2]);
}

int SEEGPointRepresentation::InsertNextPoint(double x, double y, double z)
{
    if( !m_points ) return 0;
    int index = m_points->GetNumberOfPoints();
    QString pointname;
    pointname.sprintf("%s_%02d", m_points->GetName().toStdString().c_str(), index);
    double pos[3] = {x, y, z};
    m_points->AddPoint(pointname, pos);
    return 1;
}

int SEEGPointRepresentation::RemovePoint(int index)
{
    if( (index < 0) || (index >= m_points->GetNumberOfPoints()) ) return 0;
    m_points->RemovePoint(index);
    return 1;
}

int SEEGPointRepresentation::RemoveLastPoint()
{   
    return this->RemovePoint(m_points->GetNumberOfPoints()-1);
}

double * SEEGPointRepresentation::GetPointPosition(int index)
{
    return m_points->GetPointCoordinates(index);
}

int SEEGPointRepresentation::SetPointPosition(int index, double pos[3])
{
    if( (index < 0) || (index >= m_points->GetNumberOfPoints()) ) return 0;
    m_points->SetPointCoordinates(index, pos);
    return 1;
}

int SEEGPointRepresentation::SetPointPosition(int index, double x, double y, double z)
{
    double pos[3] = {x, y, z};
    return this->SetPointPosition(index, pos);
}

void SEEGPointRepresentation::ShowPoints()
{
    m_points->SetHidden(false);
}

void SEEGPointRepresentation::HidePoints()
{
    m_points->SetHidden(true);
}

void SEEGPointRepresentation::SelectPoint(int index)
{
    if( index == -1 )
    {
        m_points->UnselectAllPoints();
        return;
    }

    m_points->SetSelectedPoint(index);
}

int SEEGPointRepresentation::GetSelectedPoint()
{
    return m_points->GetSelectedPointIndex();
}

void SEEGPointRepresentation::SetColor(double color[3])
{
    Q_ASSERT(m_points);
    m_points->SetEnabledColor(color);
}

double * SEEGPointRepresentation::GetColor()
{
    Q_ASSERT(m_points);
    return m_points->GetEnabledColor();
}

void SEEGPointRepresentation::SetPointsRadius(double radius)
{
    m_points->Set2DRadius(radius);
}

int SEEGPointRepresentation::GetNumberOfPoints()
{
    return m_points->GetNumberOfPoints();
}

void SEEGPointRepresentation::Delete()
{
    m_points->Delete();
}

} // end of namespace seeg
