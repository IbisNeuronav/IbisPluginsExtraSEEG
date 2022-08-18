#ifndef CONTACTINFO_H
#define CONTACTINFO_H

/**
 * @file ContactInfo
 *
 * Defines the class ContactInfo which includes information
 * about a specific contact (coordinates, name, location, accessors)
 *
 * @author Rina Zelmann
 */

#include <string>
#include "BasicTypes.h"
#include "VolumeTypes.h"

using namespace std;


namespace seeg {

    /**
     * Class that contains all the information about one contact of the electrode
     */
    class ContactInfo
    {
    public:
        /** Defines a smart pointer type for the ContactInfo class */
        typedef mrilSmartPtr<ContactInfo> Pointer;

         /** World coordinates of the central point of the contact*/
        seeg::Point3D m_CentralPointWorld;

        /** Contact's Index within an electrode**/
        int m_ContactIndex;

        /** Contact's Name**/
        std::string m_ContactName;

        /** Contact's Anatomical Location**/
        std::string m_Location;
        int m_LocationValue;
        bool m_IsLocationSet;
        double m_ProbaLocation; //Probability (0-1) that the contact is in that location

        /***
         * Creates a new instance of the ElectrodeInfo class
         *
         * @param centralPointWorld reference to the contact central point's world coordinates
         * @return a smart pointer to the newly created ContactInfo instance
         */

        static Pointer New(const seeg::Point3D& centralPointWorld, const int index, std::string contactName) {
            return Pointer(new ContactInfo(centralPointWorld, index, contactName));
        }

        static Pointer New(const seeg::Point3D& centralPointWorld, const int index) {
            return Pointer(new ContactInfo(centralPointWorld, index, ""));
        }

        static Pointer New() {
            return Pointer(new ContactInfo());
        }

        virtual ~ContactInfo();

        /*** Getters & Setters ***/

        seeg::Point3D GetCentralPoint();

        void SetCentralPoint(seeg::Point3D  point);

        std::string GetContactName();

        void SetContactName(std::string name);

        std::string GetContactLocation();

        float GetProbabilityOfLocation();

        int GetContactLocationValue();

        void SetContactLocation(std::string location);

        void SetContactLocation(std::string location, const int locationValue, const float probability);

        int GetContactIndex();

        void SetContactIndex(const int index);

        bool IsLocationSet();

        void SetIsLocationSet(bool isSet);

    protected:
        /*** Constructors / Destructors ***/
        ContactInfo();

        ContactInfo(const Point3D& centralPointWorld, const int index, std::string contactName);

    };
}
#endif // CONTACTINFO_H
