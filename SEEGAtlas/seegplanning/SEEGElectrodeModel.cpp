#include "SEEGElectrodeModel.h"

namespace seeg {

    ObjectSerializationMacro(seeg::SEEGElectrodeModel);

    SEEGElectrodeModel::SEEGElectrodeModel() 
    {
        double contactDiameter = 0.8;
        double contactHeight = 0.5;
        double spacing = 5;
        int numContacts = 9;
        double tipOffset = 0;
        double tipHeight = 1;
        double recRadious = 2.5;
        double pegHeight = 2.5;
        double pegDiameter = 1.3;

        // default MNI
        m_ElectrodeId = "MNI";
        m_ElectrodeName = "MNI electrodes (0.8mm^2)";
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

    SEEGElectrodeModel::Pointer SEEGElectrodeModel::New()
    {
        return SEEGElectrodeModel::Pointer(new SEEGElectrodeModel());
    }


    Point3D SEEGElectrodeModel::SEEGCalcContactPosition(unsigned int contact_index, Point3D electrodeTip, Point3D entryPoint, bool onlyInsideBrain) {
        //overlays CalcContactPosition from ElectrodeModel to consider first contact different size

        Vector3D_lf pt1 (electrodeTip[0], electrodeTip[1], electrodeTip[2]);
        Vector3D_lf pt2 (entryPoint[0], entryPoint[1], entryPoint[2]);
        Point3D ptOut;

        double d = norm(pt2-pt1);
        Vector3D_lf unit_vect = (pt2-pt1)/d;

        double distanceFromTip =CalcDistFromTipToContactCenter(contact_index);

        if (distanceFromTip<= d || !onlyInsideBrain) { //distance is smaller than from entry-point, therefore contact is inside brain.
            ptOut[0] = electrodeTip[0] + distanceFromTip*unit_vect.x;
            ptOut[1] = electrodeTip[1] + distanceFromTip*unit_vect.y;
            ptOut[2] = electrodeTip[2] + distanceFromTip*unit_vect.z;
        }
        else {
            ptOut[0] = 0.0; //NULL;
            ptOut[1] = 0.0; //NULL;
            ptOut[2] = 0.0; //NULL;
        }
        return ptOut;
    }

    void SEEGElectrodeModel::CalcAllContactPositions(Point3D electrodeTip, Point3D entryPoint, vector<Point3D> & allContactPoints, bool onlyInsideBrain) {
        // get all contact's positions
        for (int iContact=0; iContact<m_NumContacts; iContact++) {
            Point3D p = SEEGCalcContactPosition(iContact, electrodeTip, entryPoint, onlyInsideBrain);
            if (p[0]) {
                allContactPoints.push_back( p );
            }
        }
    }
    void SEEGElectrodeModel::CalcContactStartEnds(unsigned int contact_index, Point3D electrodeTip, Point3D entryPoint, Point3D &contactPoint1, Point3D &contactPoint2) {

        Vector3D_lf pt1 (electrodeTip[0], electrodeTip[1], electrodeTip[2]);
        Vector3D_lf pt2 (entryPoint[0], entryPoint[1], entryPoint[2]);

        double d = norm(pt2-pt1);
        Vector3D_lf unit_vect = (pt2-pt1)/d;

        double distanceFromTip1, distanceFromTip2;

        if (contact_index==0){ // first contact may have different size
            distanceFromTip1 = m_TipOffset;
            distanceFromTip2 = distanceFromTip1 + m_TipContactHeight;
        }
        else {
            distanceFromTip1 = m_TipOffset + m_TipContactHeight/2 + contact_index * m_ContactSpacing - m_ContactHeight/2; // m_TipOffset + contact_index * m_ContactSpacing + m_ContactHeight/2; -- this other option does not consider size of tip electrode
            distanceFromTip2 = distanceFromTip1 +  m_ContactHeight; // m_TipOffset + contact_index * m_ContactSpacing + m_ContactHeight/2; -- this other option does not consider size of tip electrode
        }

        contactPoint1[0] = electrodeTip[0] + distanceFromTip1*unit_vect.x;
        contactPoint1[1] = electrodeTip[1] + distanceFromTip1*unit_vect.y;
        contactPoint1[2] = electrodeTip[2] + distanceFromTip1*unit_vect.z;
        contactPoint2[0] = electrodeTip[0] + distanceFromTip2*unit_vect.x;
        contactPoint2[1] = electrodeTip[1] + distanceFromTip2*unit_vect.y;
        contactPoint2[2] = electrodeTip[2] + distanceFromTip2*unit_vect.z;
    }


    void SEEGElectrodeModel::CalcAllLinePositions(float spacing, Point3D electrodeTip, Point3D entryPoint, vector<Point3D> &allPointsVec) {
        //calculates the position of each point in the electrode central line considering spacing as the step size

        Vector3D_lf pt1 (electrodeTip[0], electrodeTip[1], electrodeTip[2]);
        Vector3D_lf pt2 (entryPoint[0], entryPoint[1], entryPoint[2]);
        Point3D ptOut;

        double d = norm(pt2-pt1);
        Vector3D_lf unit_vect = (pt2-pt1)/d;

        for (int i=0; i<=round(d/spacing); i++){
            ptOut[0] = pt1.x + i * spacing * unit_vect.x;
            ptOut[1] = pt1.y + i * spacing * unit_vect.y;
            ptOut[2] = pt1.z + i * spacing * unit_vect.z;
            allPointsVec.push_back(ptOut);
        }

    }

    Point3D SEEGElectrodeModel::CalcPegPointPosition(Point3D electrodeTip, Point3D entryPoint){
        //Peg point is defined as entryPoint + pegDistance (in the direction specified by target-entry points)
        Point3D pegPoint;
        float electrodeHeight = CalcLineLength(electrodeTip, entryPoint);
        float pegHeight = GetPegHeight();
        Resize3DLineSegmentSingleSide(electrodeTip, entryPoint, electrodeHeight + pegHeight, pegPoint);
        return pegPoint;
    }



    double SEEGElectrodeModel::GetElectrodeHeight(){
        double electrodeLength = m_TipOffset + m_TipContactHeight/2 + (m_NumContacts-1) * m_ContactSpacing + m_ContactHeight/2; //since spacing includes contact length ->remove 1 contact if including tip
        //double electrodeLength = m_TipOffset + m_TipContactHeight + m_NumContacts * (m_ContactSpacing + m_ContactHeight); //Assuming that spacing is from edges of electrode
        return electrodeLength;

    }

    Vector3D_lf SEEGElectrodeModel::GetUnitVector(Point3D electrodeTip, Point3D entryPoint){
        Vector3D_lf pt1 (electrodeTip[0], electrodeTip[1], electrodeTip[2]);
        Vector3D_lf pt2 (entryPoint[0], entryPoint[1], entryPoint[2]);

        double d = norm(pt2-pt1);
        Vector3D_lf unitVector = (pt2-pt1)/d;
        return unitVector;
    }

    float SEEGElectrodeModel::CalcDistFromTipToContactCenter(unsigned int contact_index) {
        // compute the distance from the tip to the center of the specified contact
        return m_TipOffset + m_TipContactHeight/2 + contact_index * m_ContactSpacing;  // assuming contact spacing is from center to center of contacts
        /*
        float distanceFromTip;
        if (contact_index==0){ // first contact may have different size
            distanceFromTip = m_TipOffset + m_TipContactHeight/2;
        }
        else {
           // distanceFromTip = m_TipOffset + m_TipContactHeight + contact_index * m_ContactSpacing + (contact_index-1) * m_ContactHeight + m_ContactHeight/2; // assuming contact spacing is from edges of the contact:
             distanceFromTip = m_TipOffset + m_TipContactHeight/2 + contact_index * m_ContactSpacing;  // assuming contact spacing is from center to center of contacts
             // m_TipOffset + contact_index * m_ContactSpacing + m_ContactHeight/2; -- this other option does not consider size of tip electrode
        }
        return distanceFromTip;*/
    }

    Point3D SEEGElectrodeModel::getElectrodeTipFromContactPosition(unsigned int contact_index, Point3D contactCenter, Point3D entryPoint) {
        //starting from the center of a contact, get back the tip of the electrode
        float distFromTip = CalcDistFromTipToContactCenter(contact_index);
        Vector3D_lf uVector = GetUnitVector(contactCenter, entryPoint);

        Point3D electrodeTip;
        electrodeTip[0] = contactCenter[0] - distFromTip * uVector.x;
        electrodeTip[1] = contactCenter[1] - distFromTip * uVector.y;
        electrodeTip[2] = contactCenter[2] - distFromTip * uVector.z;

        return electrodeTip;
    }

    Point3D SEEGElectrodeModel::CalcChannelCenterPosition(unsigned int contact_index, Point3D electrodeTip, Point3D entryPoint, bool onlyInsideBrain) {
        //overlays CalcContactPosition from ElectrodeModel to consider first contact different size

        Vector3D_lf pt1 (electrodeTip[0], electrodeTip[1], electrodeTip[2]);
        Vector3D_lf pt2 (entryPoint[0], entryPoint[1], entryPoint[2]);
        Point3D ptOut;

        double d = norm(pt2-pt1);
        Vector3D_lf unit_vect = (pt2-pt1)/d;

        double distanceFromTipToChannelCenter =CalcDistFromTipToChannelCenter(contact_index);

        if (distanceFromTipToChannelCenter<= d || !onlyInsideBrain) { //distance is smaller than from entry-point, therefore contact is inside brain.
            ptOut[0] = electrodeTip[0] + distanceFromTipToChannelCenter*unit_vect.x;
            ptOut[1] = electrodeTip[1] + distanceFromTipToChannelCenter*unit_vect.y;
            ptOut[2] = electrodeTip[2] + distanceFromTipToChannelCenter*unit_vect.z;
        }
        else {
            ptOut[0] = 0.0; //NULL;
            ptOut[1] = 0.0; //NULL;
            ptOut[2] = 0.0; //NULL;
        }
        return ptOut;
    }
    float SEEGElectrodeModel::CalcDistFromTipToChannelCenter(unsigned int contact_index) {
        // compute the distance from the tip to the center of the specified contact
        return m_TipOffset + m_TipContactHeight/2 + contact_index * m_ContactSpacing + m_ContactSpacing/2;  // assuming contact spacing is from center to center of contacts

    }

    void SEEGElectrodeModel::DeepCopy(const SEEGElectrodeModel::Pointer model)
    {
        if( this == model.get() ) return;

        this->m_ElectrodeId = model->m_ElectrodeId;
        this->m_ElectrodeName = model->m_ElectrodeName;
        this->m_TipContactHeight = model->m_TipContactHeight;
        this->m_recordingRadius = model->m_recordingRadius;
        this->m_PegHeight = model->m_PegHeight;
        this->m_PegDiameter = model->m_PegDiameter;
        this->m_ContactDiameter = model->m_ContactDiameter;
        this->m_ContactHeight = model->m_ContactHeight;
        this->m_ContactSpacing = model->m_ContactSpacing;
        this->m_NumContacts = model->m_NumContacts;
        this->m_TipOffset = model->m_TipOffset;

    }

    void SEEGElectrodeModel::Serialize(Serializer * ser)
    {
        ::Serialize(ser, "ElectrodeId", m_ElectrodeId);
        ::Serialize(ser, "ElectrodeName", m_ElectrodeName);
        ::Serialize(ser, "TipContactHeight", m_TipContactHeight);
        ::Serialize(ser, "RecordingRadius", m_recordingRadius);
        ::Serialize(ser, "PegHeight", m_PegHeight);
        ::Serialize(ser, "PegDiameter", m_PegDiameter);
        ::Serialize(ser, "ContactDiameter", m_ContactDiameter);
        ::Serialize(ser, "ContactHeight", m_ContactHeight);
        ::Serialize(ser, "ContactSpacing", m_ContactSpacing);
        ::Serialize(ser, "NumContacts", m_NumContacts);
        ::Serialize(ser, "TipOffset", m_TipOffset);
    }
}
