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
    /** POSSIBLE MODELS
      * if adding new model modify:
      * in this file: 1.enum list / 2.ElectrodeTypeEnumToString / 3.ElectrodeTypeEnumToIndex / 4.ElectrodeTypeStringToEnum / 5.New
      * in SEEGplanningwidget.ui: 1.add to list
      */
    enum SEEG_ELECTRODE_MODEL_TYPE {
        MNI,
        DIXI10,
        DIXI15,
        DIXI18,
        STRIP,
        CONTACT,
        MM09A51,
        MM09A40,
        MM09A33,
        TEST
    };

    static string ElectrodeTypeEnumToString(SEEG_ELECTRODE_MODEL_TYPE type){
        switch(type){
        case MNI:
            return "MNI";
        case DIXI10:
            return "DIXI10";
        case DIXI15:
            return "DIXI15";
        case DIXI18:
            return "DIXI18";
        case STRIP:
            return "STRIP";
        case CONTACT:
            return "CONTACT"; // 1 DIXI contact
        case MM09A51:
            return "MM09A51";
        case MM09A40:
            return "MM09A40";
        case MM09A33:
            return "MM09A33";
        case TEST:
            return "TEST";
        default:
            return "-1";
        }
    }

    static SEEG_ELECTRODE_MODEL_TYPE ElectrodeTypeIndexToType(int index){ // RIZ: index MUST be the same as in the ui
        switch(index){
        case 0:
            return MNI;
        case 1:
            return DIXI10;
        case 2:
            return DIXI15;
        case 3:
            return DIXI18;
        case 4:
            return STRIP;
        case 5:
            return CONTACT; // 1 DIXI contact
        case 6:
            return MM09A51;
        case 7:
            return MM09A40;
        case 8:
            return MM09A33;
        case 9:
            return TEST;
        default:
            return MNI;
        }
    }

    int ElectrodeTypeEnumToIndex(SEEG_ELECTRODE_MODEL_TYPE type){
        //order in SEEGplanningwidget list MUST be the same!
        //RIZ20151227!! HACK to by pass error in IBIS with vtk6
        switch(type){
        case MNI:
            return 0;
        case DIXI10:
            return 1;
        case DIXI15:
            return 2;
        case DIXI18:
            return 3;
        case STRIP:
            return 4;
        case CONTACT:
            return 5;
        case MM09A51:
            return 6;
        case MM09A40:
            return 7;
        case MM09A33:
            return 8;
        case TEST:
            return 9;
        default:
            return -1;
        }
    }

    SEEG_ELECTRODE_MODEL_TYPE ElectrodeTypeStringToEnum(string typeStr){
        SEEG_ELECTRODE_MODEL_TYPE type;
        if(typeStr.compare(string("MNI"))==0){type=MNI;}
        else if (typeStr.compare(string("DIXI10"))==0) {type=DIXI10;}
        else if (typeStr.compare(string("DIXI15"))==0) {type=DIXI15;}
        else if (typeStr.compare(string("DIXI18"))==0) {type=DIXI18;}
        else if (typeStr.compare(string("STRIP"))==0) {type=STRIP;}
        else if (typeStr.compare(string("CONTACT"))==0) {type=CONTACT;}
        else if (typeStr.compare(string("MM09A51")) == 0) { type = MM09A51; }
        else if (typeStr.compare(string("MM09A40")) == 0) { type = MM09A40; }
        else if (typeStr.compare(string("MM09A33")) == 0) { type = MM09A33; }
        else if (typeStr.compare(string("TEST"))==0) {type=TEST;}
        // This line is giving an error on Win32
		//error C2679: binary '<<': no operator found which takes a right-hand operand of type 'overloaded-function' (or there is no acceptable conversion)
		// cout<<"In electrodemodel: "<<typeStr<<" - "<<type<<endl;

        return type;
     }

    virtual void Serialize(Serializer * ser);

private:
    std::string m_ElectrodeName;
    double m_TipContactHeight;
    double m_recordingRadius;
    double m_PegHeight; //RIZ -> find out REAL VALUES!!!
    double m_PegDiameter;
    SEEG_ELECTRODE_MODEL_TYPE m_ElectrodeTypeName; // keep electrode type to be able to save it's name
    double m_ContactDiameter;
    double m_ContactHeight;
    double m_ContactSpacing;
    int m_NumContacts;
    double m_TipOffset; // distance between tip and last contact

public:
    typedef mrilSmartPtr<SEEGElectrodeModel> Pointer;
    //typedef SEEGElectrodeModel * Pointer;

    static Pointer New(SEEG_ELECTRODE_MODEL_TYPE type) {
        std::string name = ElectrodeTypeEnumToString(type);
        switch(type) {
        case MNI:// diameter,  heightContact,  spacingInterContact,  numContacts,  tipOffset,  heightTip,  recRadious, heightPEG,  diameterPEG, type
            return Pointer(new SEEGElectrodeModel(name, 0.8, 0.5, 5, 9, 0, 1, 2.5, 2.5, 1.3, type)); // in diameter - use canula's diameter (0.8mm) instead of real diameter (0.254+0.0762) // RIZ2016 changed recording radius to 5mm
        case DIXI10:
            return Pointer(new SEEGElectrodeModel(name, 0.8, 2, 3.5, 10, 0, 2, 2.5, 2.5, 1.3, type));
        case DIXI15:
            return Pointer(new SEEGElectrodeModel(name, 0.8, 2, 3.5, 15, 0, 2, 2.5, 2.5, 1.3, type));
        case DIXI18:
            return Pointer(new SEEGElectrodeModel(name, 0.8, 2, 3.5, 18, 0, 2, 2.5, 2.5, 1.3, type));
        case STRIP:
            return Pointer(new SEEGElectrodeModel(name, 1, 2, 5, 8, -1, 2, 5, 0, 0, type)); // RIZ2016: FALTA - ADTECH??
        case CONTACT:
            return Pointer(new SEEGElectrodeModel(name, 0.8, 2, 3.5, 1, -1, 2, 2.5, 0, 0, type)); //CONTACT is 1 DIXI contact - because tip point is actually center -> move electrode hegihtContact/2 (tipOffset=-1)
        case MM09A51:
            return Pointer(new SEEGElectrodeModel(name, 0.8, 2, 6.1, 9, 0, 2, 2.5, 2.5, 1.3, type));
        case MM09A40:
            return Pointer(new SEEGElectrodeModel(name, 0.8, 2, 4.8, 9, 0, 2, 2.5, 2.5, 1.3, type));
        case MM09A33:
            return Pointer(new SEEGElectrodeModel(name, 0.8, 2, 3.9, 9, 0, 2, 2.5, 2.5, 1.3, type));
        case TEST:
        default:
            return Pointer(new SEEGElectrodeModel(name, 5, 1, 10, 3, 0.25, 0.5, 2.5, 20, 5, type));
        }
    }
    
    SEEGElectrodeModel();

protected:
    SEEGElectrodeModel(std::string name, double contactDiameter, double contactHeight, double spacing, int numContacts, double tipOffset, double tipHeight, double recRadious,double pegHeight, double pegDiameter, SEEG_ELECTRODE_MODEL_TYPE type){
        m_ElectrodeName = name;
        m_TipContactHeight = tipHeight;
        m_recordingRadius = recRadious; // CHECK VALUES - might depend based on electrode type!!!
        m_PegHeight = pegHeight;
        m_PegDiameter = pegDiameter;
        m_ElectrodeTypeName = type;
        m_ContactDiameter = contactDiameter;
        m_ContactHeight = contactHeight;
        m_ContactSpacing = spacing;
        m_NumContacts = numContacts;
        m_TipOffset = tipOffset;
    }

public:
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

    SEEG_ELECTRODE_MODEL_TYPE GetElectrodeType() {
        return m_ElectrodeTypeName;
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

};

ObjectSerializationHeaderMacro(SEEGElectrodeModel);

}

#endif // SEEGELECTRODE_H
