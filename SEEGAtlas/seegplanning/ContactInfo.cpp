/**
 * @file ContactInfo.cpp
 *
 * Implementation of the ContactInfo class for atlas SEEG study
 *
 * @author  Rina Zelmann
 */

#include "ContactInfo.h"

using namespace std;

namespace seeg {

/***
 * Constructors / Destructors
 */
ContactInfo::ContactInfo() {
}

/***
 * @param centralPointWorld reference to the contact central point's world coordinates
 * @param index reference to the contact's index within the electrode (usually 1 is the deepest)
 * @param contactName reference to the channel Name (Ususally is Electrode name + contact idnex)
 */
ContactInfo::ContactInfo(const Point3D& centralPointWorld, const int index, std::string contactName) {
    m_CentralPointWorld = centralPointWorld;
    m_ContactIndex = index;
    m_ContactName = contactName;
    m_Location="";
    m_LocationValue=0;
    m_ProbaLocation=-1;
    m_IsLocationSet=false;
}

ContactInfo::~ContactInfo() {
    //nothing to do
}

/*** Getters & Setters ***/

seeg::Point3D ContactInfo::GetCentralPoint(){
    return m_CentralPointWorld;
}

void ContactInfo::SetCentralPoint(seeg::Point3D  point){
    m_CentralPointWorld = point;
}

std::string ContactInfo::GetContactName(){
    return m_ContactName;
}

void ContactInfo::SetContactName(std::string name){
    m_ContactName = name;
}

std::string ContactInfo::GetContactLocation(){
    return m_Location;
}

float ContactInfo::GetProbabilityOfLocation(){
    return m_ProbaLocation;
}

int ContactInfo::GetContactLocationValue(){
    return m_LocationValue;
}

void ContactInfo::SetContactLocation(std::string location){
    SetContactLocation(location, 0, -1);
}

void ContactInfo::SetContactLocation(std::string location, const int locationValue, const float probability){
    m_Location = location;
    m_LocationValue = locationValue;
    m_IsLocationSet = true;
    m_ProbaLocation =probability;
}

int ContactInfo::GetContactIndex(){
    return m_ContactIndex;
}

void ContactInfo::SetContactIndex(const int index){
    m_ContactIndex = index;
}

bool ContactInfo::IsLocationSet(){
    return m_IsLocationSet;
}

void ContactInfo::SetIsLocationSet(const bool isSet){
    m_IsLocationSet = isSet;
}

}
