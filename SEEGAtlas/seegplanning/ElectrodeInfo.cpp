/**
 * @file ElectrodeInfo.cpp
 *
 * Implementation of the ElectrodeInfo class for atlas SEEG study
 *
 * @author  Rina Zelmann
 */

// header files to include
#include "ElectrodeInfo.h"

using namespace std;

namespace seeg {

/***
 * Constructors / Destructors
 */
ElectrodeInfo::ElectrodeInfo() {
    m_TargetPointWorld[0] = 0;m_TargetPointWorld[1] = 0;m_TargetPointWorld[2] = 0;
    m_EntryPointWorld[0] = 0;m_EntryPointWorld[1] = 0;m_EntryPointWorld[2] = 0;
    m_ElectrodeVectorWorld.x=0;
    m_ElectrodeVectorWorld.y=0;
    m_ElectrodeVectorWorld.z=0;
    m_ElectrodeName = "";
    m_Valid = false;
}


/***
 * @param entryPointWorld reference to the entry point's world coordinates
 * @param targetPointWorld reference to the target point's world coordinates
 */
ElectrodeInfo::ElectrodeInfo(const Point3D& entryPointWorld, const Point3D& targetPointWorld, const SEEGElectrodeModel::Pointer electrodeModel, std::string electrodeName) {
    m_TargetPointWorld = targetPointWorld;
    m_EntryPointWorld = entryPointWorld;

    Vector3D_lf v;
    v.x = m_EntryPointWorld[0] - m_TargetPointWorld[0];
    v.y = m_EntryPointWorld[1] - m_TargetPointWorld[1];
    v.z = m_EntryPointWorld[2] - m_TargetPointWorld[2];
    float d = norm(v);
    m_ElectrodeVectorWorld = v/d; // unit vector
    m_ElectrodeModel = SEEGElectrodeModel::New();
    m_ElectrodeModel->DeepCopy(electrodeModel);
    m_ElectrodeName = electrodeName;
    m_Valid = false;
}

ElectrodeInfo::~ElectrodeInfo() {
    // nothing to do
}

/***
 * Getters & Setters
 */
seeg::Point3D ElectrodeInfo::GetTargetPoint(){
    return m_TargetPointWorld;
}

seeg::Point3D ElectrodeInfo::GetEntryPoint(){
    return m_EntryPointWorld;
}

void ElectrodeInfo::SetTargetPoint(seeg::Point3D targetPoint){
     m_TargetPointWorld = targetPoint;
}

void ElectrodeInfo::SetEntryPoint(seeg::Point3D entryPoint){
     m_EntryPointWorld = entryPoint;
}

seeg::SEEGElectrodeModel::Pointer ElectrodeInfo::GetElectrodeModel()
{
    return m_ElectrodeModel;
}

void ElectrodeInfo::SetElectrodeModel(seeg::SEEGElectrodeModel::Pointer electrodeModel){
    m_ElectrodeModel->DeepCopy(electrodeModel);
}

seeg::Vector3D_lf ElectrodeInfo::GetElectrodeVectorWorld(){
    if (m_ElectrodeVectorWorld.x==0 && m_ElectrodeVectorWorld.y==0 && m_ElectrodeVectorWorld.z==0) { //Lazy computation
        seeg::Vector3D_lf v;
        v.x = m_EntryPointWorld[0] - m_TargetPointWorld[0];
        v.y = m_EntryPointWorld[1] - m_TargetPointWorld[1];
        v.z = m_EntryPointWorld[2] - m_TargetPointWorld[2];
        float d = norm(v);
        m_ElectrodeVectorWorld = v/d; // unit vector
    }
    return m_ElectrodeVectorWorld;
}

std::string ElectrodeInfo::GetElectrodeName(){
    return m_ElectrodeName;
}

void ElectrodeInfo::SetElectrodeName(std::string electrodeName){
    m_ElectrodeName = electrodeName;
}

int ElectrodeInfo::GetNumberOfContacts(){
    return m_Contacts.size();
}

vector<ContactInfo::Pointer> ElectrodeInfo::GetAllContacts(){
    return m_Contacts;
}

seeg::ContactInfo::Pointer ElectrodeInfo::GetOneContact(const int index){
    return m_Contacts[index];
}

void ElectrodeInfo::AddContactToElectrode(ContactInfo::Pointer contact){
    m_Contacts.push_back(contact);
}

void ElectrodeInfo::AddAllContactsToElectrode(vector<seeg::ContactInfo::Pointer> allContacts){
    m_Contacts = allContacts;
}

void ElectrodeInfo::AddAllContactsToElectrode(vector<seeg::Point3D> allCentralPoints){
    for (int index=0; index< allCentralPoints.size(); index++) {
        ostringstream sstream;
        sstream << (index+1);
        string contactName = this->GetElectrodeName() + sstream.str();
        ContactInfo::Pointer contact = ContactInfo::New(allCentralPoints[index], index, contactName);
        this->AddContactToElectrode(contact);
    }
}

void ElectrodeInfo::ReplaceContact(ContactInfo::Pointer contact, const int index){
    m_Contacts[index] = contact;
}

//Channel Info
int ElectrodeInfo::GetNumberOfChannels(){
    return m_Channels.size();
}

vector<ChannelInfo::Pointer> ElectrodeInfo::GetAllChannels(){
    return m_Channels;
}

seeg::ChannelInfo::Pointer ElectrodeInfo::GetOneChannel(const int index){
    return m_Channels[index];
}

void ElectrodeInfo::AddChannelToElectrode(ChannelInfo::Pointer channel){
    m_Channels.push_back(channel);
}

void ElectrodeInfo::AddAllChannelsToElectrode(){
    //All Contacts must exist! (call after calling AddAllContactsToElectrode)
    if (m_Contacts.size()<1){
        return;
    }
    for (int index=0; index< m_Contacts.size()-1; index++) {
        ostringstream sstreamIndex1, sstreamIndex2;
        sstreamIndex1 << (index+1);
        sstreamIndex2 << (index+2);
        string channelName = this->GetElectrodeName() + sstreamIndex1.str() +"-" + sstreamIndex2.str();
        ContactInfo::Pointer contact1 = m_Contacts[index];
        ContactInfo::Pointer contact2 = m_Contacts[index+1];
        ChannelInfo::Pointer channel = ChannelInfo::New(contact1, contact2, channelName);
        this->AddChannelToElectrode(channel);
    }
}

void ElectrodeInfo::AddAllContactsAndChannelsToElectrode(vector<seeg::Point3D> allCentralPoints){
    AddAllContactsToElectrode(allCentralPoints);
    AddAllChannelsToElectrode();
}

// Functions
void ElectrodeInfo::ExtrapolateTargetPoint(Point3D& targetIn, Point3D& entryPt, Point3D& targetOut, const float lenExtra) {

    Vector3D_lf pTP(targetIn[0], targetIn[1], targetIn[2]);
    Vector3D_lf pEP(entryPt[0], entryPt[1], entryPt[2]);

    float d = norm(pEP-pTP);

    // extrapolate by val mm
    float deltaX = lenExtra * (pEP.x - pTP.x) / d; //for entry points val =10.0mm
    float deltaY = lenExtra * (pEP.y - pTP.y) / d;
    float deltaZ = lenExtra * (pEP.z - pTP.z) / d;


    targetOut[0] = targetIn[0] - deltaX;
    targetOut[1] = targetIn[1] - deltaY;
    targetOut[2] = targetIn[2] - deltaZ;

}

void ElectrodeInfo::ExtrapolateEntryPoint(Point3D& targetPt, Point3D& entryIn, Point3D& entryOut, const float lenExtra) {

    Vector3D_lf pTP(targetPt[0], targetPt[1], targetPt[2]);
    Vector3D_lf pEP(entryIn[0], entryIn[1], entryIn[2]);

    float d = norm(pEP-pTP);

    // extrapolate by val mm
    float deltaX = lenExtra * (pEP.x - pTP.x) / d; //for entry points original val =10.0mm
    float deltaY = lenExtra * (pEP.y - pTP.y) / d;
    float deltaZ = lenExtra * (pEP.z - pTP.z) / d;


    entryOut[0] = entryIn[0] + deltaX;
    entryOut[1] = entryIn[1] + deltaY;
    entryOut[2] = entryIn[2] + deltaZ;
}

void ElectrodeInfo::ExtrapolateEntryPointToFixLength(Point3D& targetPt, Point3D& entryIn, Point3D& entryOut, const float lenFix) {

    Vector3D_lf pTP(targetPt[0], targetPt[1], targetPt[2]);
    Vector3D_lf pEP(entryIn[0], entryIn[1], entryIn[2]);

    float d = norm(pEP-pTP);
    float lenExtra = lenFix - d;
    ExtrapolateEntryPoint(targetPt, entryIn, entryOut, lenExtra);
}

/*** Save & Load functions ***/
bool ElectrodeInfo::LoadElectrodeDataFromFile (const string& filename, const char delimiter, std::vector<seeg::SEEGElectrodeModel::Pointer> modelList) {
    //only load this electrode's data
    ifstream file;
    string line("notempty");
    string token;
    string testName;

    bool status=false;
  //  ResetElectrode(); // do not reset electrode info -> just add contacts info

    file.open(filename.c_str());

    while(~file.eof() && line.size()>1) {
        getline(file, line);
        stringstream ss (line);

        token="";
        getline(ss, token, delimiter);

        if (token == "[fileType]") {

            getline(ss, token, delimiter);
            testName = token;

        }  else if(token == "[electrodeType]") { //electrode type line contains electrotype (e.g. MNI, DIXI15)
            getline(ss, token, delimiter);
            seeg::SEEGElectrodeModel::Pointer electrodeModel;

            bool found = false;
            for(int i = 0; (i < modelList.size()) && !found; ++i )
            {
                if( modelList[i]->GetElectrodeId().compare(token.c_str()) == 0 )
                {
                    electrodeModel = modelList[i];
                    found = true;
                }
                    
            }
            if( !found )
            {
                // electrode does not exist in config directory
                //TODO: display warning message to user
                return false;
            }
            
            this->SetElectrodeModel(electrodeModel);

        } else if (token == "[trajectory]") {
            Point3D entryPointWorld, targetPointWorld;
            string electrodeName;

            getline(ss, token, delimiter);
            electrodeName = token;
            getline(ss, token, delimiter);
            targetPointWorld[0] = atof(token.c_str());
            getline(ss, token, delimiter);
            targetPointWorld[1] = atof(token.c_str());
            getline(ss, token, delimiter);
            targetPointWorld[2] = atof(token.c_str());
            getline(ss, token, delimiter);
            entryPointWorld[0] = atof(token.c_str());
            getline(ss, token, delimiter);
            entryPointWorld[1] = atof(token.c_str());
            getline(ss, token, delimiter);
            entryPointWorld[2] = atof(token.c_str());
            getline(ss, token, delimiter);
            int isElectrodeValid = atoi(token.c_str()); //RIZ20151201: Should we use this to validate??

            // add info to electrode
            SetElectrodeName(electrodeName);
            SetEntryPoint(entryPointWorld);
            SetTargetPoint(targetPointWorld);
            m_Valid = isElectrodeValid;

         } else if (token == "[contact]") {
            //add info about each contact
            Point3D centralPointWorld;
            string contactName, location;
            int contactIndex;

            getline(ss, token, delimiter);
            contactName = token;
            getline(ss, token, delimiter);
            contactIndex = atof(token.c_str());
            getline(ss, token, delimiter);
            centralPointWorld[0] = atof(token.c_str());
            getline(ss, token, delimiter);
            centralPointWorld[1] = atof(token.c_str());
            getline(ss, token, delimiter);
            centralPointWorld[2] = atof(token.c_str());
            getline(ss, token, delimiter);
            location = token;
            getline(ss, token, delimiter);
            int locationValue =  atof(token.c_str());
            getline(ss, token, delimiter);
            float probability = atof(token.c_str());
            getline(ss, token, delimiter);
            int isLocationSet = atof(token.c_str());

            ContactInfo::Pointer contact = ContactInfo::New(centralPointWorld, contactIndex, contactName);
            if (isLocationSet > 0 || probability>0) {
                contact->SetContactLocation (location, locationValue, probability);
            }
            contact->SetIsLocationSet(isLocationSet);

            AddContactToElectrode (contact);
            status=true;
        } else if (token == "[channel]") {
           //add info about each channel
           Point3D centralPoint1, centralPoint2;
           string channelName, location;
           int channelIndex;

           getline(ss, token, delimiter);
           channelName = token;
           getline(ss, token, delimiter);
           channelIndex = atof(token.c_str());
           getline(ss, token, delimiter);
           centralPoint1[0] = atof(token.c_str());
           getline(ss, token, delimiter);
           centralPoint1[1] = atof(token.c_str());
           getline(ss, token, delimiter);
           centralPoint1[2] = atof(token.c_str());
           getline(ss, token, delimiter);
           centralPoint2[0] = atof(token.c_str());
           getline(ss, token, delimiter);
           centralPoint2[1] = atof(token.c_str());
           getline(ss, token, delimiter);
           centralPoint2[2] = atof(token.c_str());
           getline(ss, token, delimiter);
           location = token;
           getline(ss, token, delimiter);
           int locationValue = atof(token.c_str());
           getline(ss, token, delimiter);
           float probability = atof(token.c_str());
           getline(ss, token, delimiter);
           int isLocationSet = atof(token.c_str());

           ContactInfo::Pointer contact1 = ContactInfo::New(centralPoint1, channelIndex); //channel index is actually first contact's index
           ContactInfo::Pointer contact2 = ContactInfo::New(centralPoint2, channelIndex +1);
           ChannelInfo::Pointer channel = ChannelInfo::New(contact1, contact2, channelName);

           if (isLocationSet > 0 || probability > 0) {
               channel->SetChannelLocation (location, locationValue, probability);
           }
           channel->SetIsLocationSet(isLocationSet);

           AddChannelToElectrode (channel);
           status=true;
        } else if (token == "[order]") {
            cout <<"order:"<< token.c_str()<<endl;
          //  getline(ss, token, delimiter);
           // break;
        } else {
            cout << token.c_str() << " not found: Possible Parse error\n";
          break;
        }
    }

    file.close();
    if (m_Channels.size() < 1) { //if only contacts are in the file (old file) -> create channels from contact's info
        AddAllChannelsToElectrode();
    }
    if (status == true) {
        cout <<"file: "<< filename << " loaded" <<endl;
    }
    return status;
}


void ElectrodeInfo::SaveElectrodeDataToFile(const string& filename, const char delimiter){
    // Saves best trajectory
    cout<<"Saving All Electrodes information to "<<filename<< endl;
    seeg::SEEGElectrodeModel::Pointer electModel = m_ElectrodeModel;
    ofstream file;
    file.open(filename.c_str());
    file << "[fileType]"<< delimiter<< "Electrode" << endl;
    file << "[electrodeType]"<< delimiter<< electModel->GetElectrodeId() << endl;
    file << "[trajectory]"<< delimiter;
    file << m_ElectrodeName.c_str() << delimiter;
    file << m_TargetPointWorld[0] << delimiter;
    file << m_TargetPointWorld[1] << delimiter;
    file << m_TargetPointWorld[2] << delimiter;
    file << m_EntryPointWorld[0] << delimiter;
    file << m_EntryPointWorld[1] << delimiter;
    file << m_EntryPointWorld[2] << delimiter;
    file << m_Valid << delimiter;
    file << endl;

    //Coordinates per contact
    // vector<ContactInfo::Pointer>::iterator contactIt;
    for (int index=0; index< m_Contacts.size(); index++) {
        ContactInfo::Pointer contact = m_Contacts[index];
        string contactName = contact->GetContactName();
        Point3D point = contact->GetCentralPoint();

        file << "[contact]"<< delimiter;
        file << contactName.c_str() << delimiter;
        file << contact->GetContactIndex() << delimiter;
        file << point[0] << delimiter;
        file << point[1] << delimiter;
        file << point[2] << delimiter;
        file << contact->GetContactLocation()<< delimiter;
        file << contact->GetContactLocationValue()<< delimiter;
        file << contact->GetProbabilityOfLocation ()<< delimiter;
        file << contact->IsLocationSet()<< delimiter;

        //finish contact line and move to next contact in the electrode
        file << endl;
    }

    //Coordinates per bipolar channel
    for (int index=0; index< m_Channels.size(); index++) {
        ChannelInfo::Pointer channel = m_Channels[index];
        string channelName = channel->GetChannelName();
        ContactInfo::Pointer contact1= channel->GetContact1();
        Point3D point1 = contact1->GetCentralPoint();
        ContactInfo::Pointer contact2= channel->GetContact2();
        Point3D point2 = contact2->GetCentralPoint();

        file << "[channel]"<< delimiter;
        file << channelName.c_str() << delimiter;
        file << channel->GetChannelIndex() << delimiter;
        file << point1[0] << delimiter;
        file << point1[1] << delimiter;
        file << point1[2] << delimiter;
        file << point2[0] << delimiter;
        file << point2[1] << delimiter;
        file << point2[2] << delimiter;
        file << channel->GetChannelLocation()<< delimiter;
        file << channel->GetChannelLocationValue()<< delimiter;
        file << channel->GetProbabilityOfLocation ()<< delimiter;
        file << channel->IsLocationSet()<< delimiter;

        //finish channel line and move to next channel in the electrode
        file << endl;
    }
    file << "[order]"<< delimiter<<"ContactName"<< delimiter<<"indexContact"<< delimiter<<"X"<< delimiter<<"Y"<< delimiter<<"Z"<< delimiter<<"Location"<< delimiter<<"ProbaOfLocation"<< delimiter<<"IsLocSet"<< endl;
    file << "[order]"<< delimiter<<"ChannelName"<< delimiter<<"indexChannel"<< delimiter<<"X1"<< delimiter<<"Y1"<< delimiter<<"Z1"<< delimiter<<"X2"<< delimiter<<"Y2"<< delimiter<<"Z2"<< delimiter<<"Location"<< delimiter<<"ProbaOfLocation"<< delimiter<<"IsLocSet"<< endl;

    file.close();
}

void ElectrodeInfo::ResetElectrode(){
    m_TargetPointWorld[0] = 0;m_TargetPointWorld[1] = 0;m_TargetPointWorld[2] = 0;
    m_EntryPointWorld[0] = 0;m_EntryPointWorld[1] = 0;m_EntryPointWorld[2] = 0;
    m_ElectrodeVectorWorld.x=0;
    m_ElectrodeVectorWorld.y=0;
    m_ElectrodeVectorWorld.z=0;
    m_ElectrodeName = "";
    m_Valid = false;
}

}
