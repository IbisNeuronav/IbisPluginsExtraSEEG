#ifndef CHANNELINFO_H
#define CHANNELINFO_H

#include "ContactInfo.h"
#include <string>
#include "BasicTypes.h"
#include "VolumeTypes.h"

using namespace std;

namespace seeg {

class ChannelInfo {
    public:
        /** Defines a smart pointer type for the ContactInfo class */
        typedef mrilSmartPtr<ChannelInfo> Pointer;

        /** a bipolar channel is defined by its constituent contacts */
        seeg::ContactInfo::Pointer m_Contact1;
        seeg::ContactInfo::Pointer m_Contact2;

        /** Channel's Name**/
        std::string m_ChannelName;

        /** Channel Index (it is actually the index of the first contact)**/
        int m_ChannelIndex;

        /** Channel's Anatomical Location**/
        std::string m_Location;
        int m_LocationValue;
        bool m_IsLocationSet;
        double m_ProbaLocation; //Probability (0-1) that the contact is in that location


        static Pointer New(ContactInfo::Pointer contact1, ContactInfo::Pointer contact2, std::string channelName) {
            return Pointer(new ChannelInfo(contact1, contact2, channelName));
        }

        static Pointer New(ContactInfo::Pointer contact1, ContactInfo::Pointer contact2) {
            return Pointer(new ChannelInfo(contact1, contact2, ""));
        }

        static Pointer New() {
            return Pointer(new ChannelInfo());
        }

        virtual ~ChannelInfo();

        /*** Getters & Setters ***/

        seeg::ContactInfo::Pointer GetContact1();

        seeg::ContactInfo::Pointer GetContact2();

        std::string GetChannelName();

        void SetChannelName(std::string name);

        std::string GetChannelLocation();

        float GetProbabilityOfLocation();

        int GetChannelLocationValue();

        void SetChannelLocation(string location);

        void SetChannelLocation(string location, const int locationValue, const float probability);

        int GetChannelIndex();        

        void SetChannelIndex(const int index);

        bool IsLocationSet();

        void SetIsLocationSet(bool isSet);

    protected:
        /*** Constructors / Destructors ***/
        ChannelInfo();

        ChannelInfo(ContactInfo::Pointer contact1, ContactInfo::Pointer contact2, std::string channelName);

    };
}


#endif // CHANNELINFO_H
