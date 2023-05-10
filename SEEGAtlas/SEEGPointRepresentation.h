#ifndef __SEEGPointRepresentation_h_
#define __SEEGPointRepresentation_h_

#include <pointsobject.h>
#include <sceneobject.h>

#include "BasicTypes.h"

class SceneObject;

namespace seeg {

class SEEGPointRepresentation 
{
public:
    // smart pointer
    typedef mrilSmartPtr<SEEGPointRepresentation> Pointer;

public:

    static Pointer New() { return Pointer(new SEEGPointRepresentation()); }
    static Pointer New(SceneObject * parent) { return Pointer(new SEEGPointRepresentation(parent)); }

    SEEGPointRepresentation(SceneObject * parent=nullptr);
    ~SEEGPointRepresentation();

    PointsObject * GetPointsObject() { return m_points; }

    void SetName(std::string name);
    std::string GetName();

    int InsertNextPoint(const double pos[3]);
    int InsertNextPoint(double x, double y, double z);

    int RemovePoint(int index);
    int RemoveLastPoint();

    double * GetPointPosition(int index);
    int SetPointPosition(int index, double pos[3]);
    int SetPointPosition(int index, double x, double y, double z);

    void ShowPoints();
    void HidePoints();

    void SelectPoint(int index);

    void SetColor(double color[3]);
    double * GetColor();

    void SetPointsRadius(double radius);

    int GetNumberOfPoints();

    void Delete();

private:

    PointsObject * m_points;

};


} // end of namespace seeg

#endif // __SEEGPointRepresentation_h_
