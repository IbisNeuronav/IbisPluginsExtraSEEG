#include "ChannelInfo.h"

/**
 * @file ChannelInfo.cpp
 *
 * Implementation of the ChannelInfo class for atlas SEEG study
 *
 * ChannelInfo class defines a bipolar Channel with the 2 contacts as input
 *
 * @author  Rina Zelmann
 */


using namespace std;

namespace seeg {

/***
 * Constructors / Destructors
 */
    ChannelInfo::ChannelInfo() {

    }

    ChannelInfo::ChannelInfo(ContactInfo::Pointer contact1, ContactInfo::Pointer contact2, std::string channelName) {
        m_Contact1 = contact1;
        m_Contact2 = contact2;
        m_ChannelIndex = m_Contact1->GetContactIndex();  // It is actually the index of the first contact
        m_ChannelName = channelName;
        m_Location ="";
        m_LocationValue = 0;
        m_ProbaLocation= -1;
        m_IsLocationSet=false;
    }

    ChannelInfo::~ChannelInfo() {
        //nothing to do
    }
/*** Getters & Setters ***/

    seeg::ContactInfo::Pointer ChannelInfo::GetContact1(){
        return m_Contact1;
    }

    seeg::ContactInfo::Pointer ChannelInfo::GetContact2(){
        return m_Contact2;
    }

    std::string ChannelInfo::GetChannelName(){
        return m_ChannelName;
    }

    void ChannelInfo::SetChannelName(std::string name){
        m_ChannelName = name;
    }

    std::string ChannelInfo::GetChannelLocation(){
        return m_Location;
    }

    int ChannelInfo::GetChannelIndex(){
        // It is actually the index of the first contact
        return m_ChannelIndex;
    }

    void ChannelInfo::SetChannelIndex(const int index){
        // By default is the index of the first contact
         m_ChannelIndex = index;
    }

    float ChannelInfo::GetProbabilityOfLocation(){
        return m_ProbaLocation;
    }

    int ChannelInfo::GetChannelLocationValue(){
        return m_LocationValue;
    }

    void ChannelInfo::SetChannelLocation(string location){
        SetChannelLocation(location, 0, -1);
    }

    void ChannelInfo::SetChannelLocation(string location, const int locationValue, const float probability){
        m_Location = location;
        m_LocationValue = locationValue;
        m_IsLocationSet = true;
        m_ProbaLocation =probability;
    }

    bool ChannelInfo::IsLocationSet(){
        return m_IsLocationSet;
    }

    void ChannelInfo::SetIsLocationSet(bool isSet){
        m_IsLocationSet = isSet;
    }


}
