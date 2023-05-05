#ifndef SEEGELECTRODE_H
#define SEEGELECTRODE_H

#include "VolumeTypes.h"
#include "BasicTypes.h"
#include "FileUtils.h"
#include "MathUtils.h"
#include "serializer.h"

namespace seeg {

// Class SEEGElectrodeModel is similar to calss ElectrodeModel of the DBS planning software but with the addition of contact specific functions
// It used to inherite from ElectrodeModel, it was changed to be selfcontained.

class SEEGElectrodeModel
{
public:
    
    virtual void Serialize(Serializer * ser);

private:
    std::string m_ElectrodeId;
    std::string m_ElectrodeName;
    double m_TipContactHeight;
    double m_recordingRadius;
    double m_PegHeight; //RIZ -> find out REAL VALUES!!!
    double m_PegDiameter;
    double m_ContactDiameter;
    double m_ContactHeight;
    double m_ContactSpacing;
    int m_NumContacts;
    double m_TipOffset; // distance between tip and last contact

public:
    typedef mrilSmartPtr<SEEGElectrodeModel> Pointer;
    //typedef SEEGElectrodeModel * Pointer;

    SEEGElectrodeModel();
    static SEEGElectrodeModel::Pointer New();

protected:
    SEEGElectrodeModel(std::string id, std::string name, double contactDiameter, double contactHeight, double spacing, int numContacts, double tipOffset, double tipHeight, double recRadious,double pegHeight, double pegDiameter){
        m_ElectrodeId = id;
        m_ElectrodeName = name;
        m_TipContactHeight = tipHeight;
        m_recordingRadius = recRadious; // CHECK VALUES - might depend based on electrode type!!!
        m_PegHeight = pegHeight;
        m_PegDiameter = pegDiameter;
        m_ContactDiameter = contactDiameter;
        m_ContactHeight = contactHeight;
        m_ContactSpacing = spacing;
        m_NumContacts = numContacts;
        m_TipOffset = tipOffset;
    }

public:
    void SetElectrodeId(std::string id) {
        m_ElectrodeId = id;
    }

    std::string GetElectrodeId() {
        return m_ElectrodeId;
    }

    void SetElectrodeName(std::string name) {
        m_ElectrodeName = name;
    }

    std::string GetElectrodeName() {
        return m_ElectrodeName;
    }

    void SetRecordingRadius(double recordingRadius) {
        m_recordingRadius = recordingRadius;
    }

    double GetRecordingRadius() {
        return m_recordingRadius;
    }

    double GetTipContactHeight() {
        return m_TipContactHeight;
    }

    double GetInterContactDistance() {
        return m_ContactSpacing;
    }

    double GetTipOffset() {
        return m_TipOffset;
    }

    double GetPegHeight() {
        return m_PegHeight;
    }

    double GetPegDiameter() {
        return m_PegDiameter;
    }

    double GetContactDiameter() {
        return m_ContactDiameter;
    }

    double GetContactHeight() {
        return m_ContactHeight;
    }

 /* RIZ20151120 Unused
    double GetContactSpacing() {
        m_ContactSpacing;
    }
*/
    int GetNumContacts() {
        return m_NumContacts;
    }

    double GetElectrodeHeight();

    Vector3D_lf GetUnitVector(Point3D electrodeTip, Point3D entryPoint);

    //Computational functions
    Point3D SEEGCalcContactPosition(unsigned int contact_index, Point3D electrodeTip, Point3D entryPoint, bool onlyInsideBrain); // overlay to consider first contact different

    void CalcAllContactPositions(Point3D electrodeTip, Point3D entryPoint, vector<Point3D> & allContacts, bool onlyInsideBrain); // computes position of all contacts

    void CalcAllLinePositions(float spacing, Point3D electrodeTip, Point3D entryPoint, vector<Point3D> &allPointsVec); // computes the position of each point in the electrode's central line

    void CalcContactStartEnds(unsigned int contact_index, Point3D electrodeTip, Point3D entryPoint, Point3D &contactPoint1, Point3D &contactPoint2); //returns start and end position of a contact (usefull to display)

    Point3D CalcPegPointPosition(Point3D electrodeTip, Point3D entryPoint);

    float CalcDistFromTipToContactCenter(unsigned int contact_index);

    Point3D getElectrodeTipFromContactPosition(unsigned int contact_index, Point3D contactCenter, Point3D entryPoint);

    Point3D CalcChannelCenterPosition(unsigned int contact_index, Point3D electrodeTip, Point3D entryPoint, bool onlyInsideBrain); // overlay to consider first contact different

    float CalcDistFromTipToChannelCenter(unsigned int contact_index);

    void DeepCopy(const SEEGElectrodeModel::Pointer model);

};

ObjectSerializationHeaderMacro(SEEGElectrodeModel);

}

#endif // SEEGELECTRODE_H
