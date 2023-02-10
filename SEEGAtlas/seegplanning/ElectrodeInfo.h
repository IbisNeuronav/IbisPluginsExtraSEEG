#ifndef __ELECTRODE_INFO__
#define __ELECTRODE_INFO__

/**
 * @file ElectrodeInfo
 *
 * Defines the class ElectrodeInfo which includes information
 * about a specific electrode (similar to EntryPointInfo but for electrode instead)
 *
 * @author Rina Zelmann
 */


// Header files to include
#include "MathUtils.h"
#include <map>
#include <string>
#include "BasicTypes.h"
#include "VolumeTypes.h"
#include "SEEGElectrodeModel.h"
#include "ContactInfo.h"
#include "ChannelInfo.h"

using namespace std;


namespace seeg {


    /**
     * Class that contains all the information about one electrode
     */
    class ElectrodeInfo  {
    public:

        /** Defines a smart pointer type for the ElectrodeInfo class */
        typedef mrilSmartPtr<ElectrodeInfo> Pointer;

         /** World coordinates of the target point*/
        seeg::Point3D m_TargetPointWorld;

        /** World coordinates of the entry point*/
        seeg::Point3D m_EntryPointWorld;

        /** World coordinates of electrode vector */
        seeg::Vector3D_lf m_ElectrodeVectorWorld;

        /** electrode model type**/
        seeg::SEEGElectrodeModel::Pointer m_ElectrodeModel;

        /** Electrode's Name**/
        std::string m_ElectrodeName;

        /** Vector of Contacts class**/
        vector<seeg::ContactInfo::Pointer> m_Contacts;

        /** Vector of Channels class**/
        vector<seeg::ChannelInfo::Pointer> m_Channels; // a channel is defined by 2 consecutive contacts

        /** is electrode valid?**/
        bool m_Valid;

        /***
         * Creates a new instance of the ElectrodeInfo class
         *
         * @param entryPointWorld reference to the entry point's world coordinates
         * @param targetWorld reference to the target point's world coordinates
         * @param targetWorld reference to the electrodeType point's world coordinates
         * @return a smart pointer to the newly created ElectrodeInfo instance
         */
        static Pointer New(const seeg::Point3D& entryPointWorld, const seeg::Point3D& targetWorld, const seeg::SEEGElectrodeModel::Pointer electrodeModel, std::string electrodeName) {
            return Pointer(new ElectrodeInfo(entryPointWorld, targetWorld, electrodeModel, electrodeName));
        }

        static Pointer New(const seeg::Point3D& entryPointWorld, const seeg::Point3D& targetWorld, const seeg::SEEGElectrodeModel::Pointer electrodeModel) {
            return Pointer(new ElectrodeInfo(entryPointWorld, targetWorld, electrodeModel, ""));
        }

        static Pointer New(const seeg::Point3D& entryPointWorld, const seeg::Point3D& targetWorld) {
            return Pointer(new ElectrodeInfo(entryPointWorld, targetWorld, SEEGElectrodeModel::New(), ""));
        }

        static Pointer New() {
            return Pointer(new ElectrodeInfo());
        }

        virtual ~ElectrodeInfo();

        /*** Getters & Setters ***/

        seeg::Point3D GetTargetPoint();

        seeg::Point3D GetEntryPoint();

        void SetTargetPoint(seeg::Point3D targetPoint);

        void SetEntryPoint(seeg::Point3D entryPoint);

        seeg::SEEGElectrodeModel::Pointer GetElectrodeModel();

        void SetElectrodeModel(seeg::SEEGElectrodeModel::Pointer electrodeModel);

        seeg::Vector3D_lf GetElectrodeVectorWorld();

        std::string GetElectrodeName();

        void SetElectrodeName(std::string electrodeName);

        int GetNumberOfContacts();

        vector<seeg::ContactInfo::Pointer> GetAllContacts();

        ContactInfo::Pointer GetOneContact(const int index);

        void AddContactToElectrode(seeg::ContactInfo::Pointer contact);

        void AddAllContactsToElectrode(vector<ContactInfo::Pointer> allContacts);

        void AddAllContactsToElectrode(vector<seeg::Point3D> allCentralPoints);

        void ReplaceContact(seeg::ContactInfo::Pointer contact, const int index);

        int GetNumberOfChannels();

        vector<ChannelInfo::Pointer> GetAllChannels();

        ChannelInfo::Pointer GetOneChannel(const int index);

        void AddChannelToElectrode(ChannelInfo::Pointer channel);

        void AddAllChannelsToElectrode();

        void AddAllContactsAndChannelsToElectrode(vector<Point3D> allCentralPoints);


        /**
         * Extrapolate Entry by lenExtra specified (in mm)
         */
        void ExtrapolateEntryPoint(Point3D& target, Point3D& entryIn, Point3D& entryOut, const float lenExtra);

        /*** Load & Save ***/
        bool LoadElectrodeDataFromFile (const string& filename, const char delimiter, std::vector<seeg::SEEGElectrodeModel::Pointer> modelList);

        void SaveElectrodeDataToFile(const string& filename, const char delimiter);


    protected:
        /*** Constructors / Destructors ***/
        ElectrodeInfo();

        ElectrodeInfo(const Point3D& entryPointWorld, const Point3D& targetPointWorld, const SEEGElectrodeModel::Pointer electrodeModel, std::string electrodeName);

    private:

        /**
         * Extrapolate Target by lenExtra specified (in mm)
         */
        void ExtrapolateTargetPoint(Point3D& targetIn, Point3D& entryPt, Point3D& targetOut, const float lenExtra);

        /**
         * Extrapolate Entry to achievbe as full length the length specified  by LenFix(in mm)
         */
        void ExtrapolateEntryPointToFixLength(Point3D& target, Point3D& entryIn, Point3D& entryOut, const float lenFix);

        //Clean everything regarding this electrode (remove contact vector etc)
        void ResetElectrode();
    };

}

#endif
