#ifndef NETWORK_H
#define NETWORK_H

#include <Arduino.h>
#include "./Config.h"
#include "./../RemoteLight.h"
#include "../Utils/SignalType.h"
#include <Firebase_ESP_Client.h>
// #include "addons/TokenHelper.h"
// #include "addons/RTDBHelper.h"
#include <NTPClient.h>
#include <array>
#include <map>
#include "WiFiProv.h"
#include "WiFi.h"
namespace remoteLight
{
    class RemoteLight;
    class Network
    {
    public:
        Network(std::shared_ptr<RemoteLight> rml);
        ~Network() { delete mTimeClient; };
        Network(const Network &) = delete;
        Network &operator=(const Network &) = delete;

        /**
         * @brief handle signal from other module
         * @param signal The signal to be handled
         * @param data Optional data associated with the signal
         */
        void handleSignal(const utils::SignalType signal, utils::Package *data = nullptr);

    private:
        enum REQUEST_FB : int32_t
        {
            NONE = 0,
            SET_INFORM,
            RECEIVED_INFORM,
            UPLOAD_INFORM,
        };

        std::shared_ptr<RemoteLight> mRML;
        String mSSID;
        String mPassword;
        FirebaseData mFbdo;
        FirebaseAuth mAuth;
        FirebaseConfig mConfig;
        WiFiUDP ntpUDP;
        NTPClient *mTimeClient;
        bool mFlagProvision;

        const std::array<String, 4U> HEAD_PATHS = {
            "light1/",
            "light2/",
            "light3/",
            "light4/",
        };
        const String FLAG_PATH = "flag";
        const std::array<String, 8U> DATA_PATHS = {
            "swOn",
            "hourOn",
            "minuteOn",
            "secondOn",
            "swOff",
            "hourOff",
            "minuteOff",
            "secondOff",
        };

        const String DATE_PATH = "date";

        const uint32_t GMT = 25200; // GMT +7
        const static int8_t MAX_RETRY_PROVISION = 3;
        const char *POP = "light1234";            // Proof of possession - otherwise called a PIN - string provided by the device, entered by the user in the phone app
        const char *SERVICE_NAME = "ESP32_LIGHT"; // Name of your device (the Espressif apps expects by default device name starting with "Prov_")
        const char *SERVICE_KEY = NULL;           // Password used for SofAP method (NULL = no password needed)
        uint8_t uuid[16] = {0xb4, 0xdf, 0x5a, 0x1c, 0x3f, 0x6b, 0xf4, 0xbf,
                            0xea, 0x4a, 0x82, 0x03, 0x04, 0x90, 0x1a, 0x02};

        static Network *instance;
        bool mFlagOnEvent;
        bool mIsFirebaseConnected;

        String mAllTimeStr;

        /**
         * @brief Connect to WiFi network
         */
        void connectWifi();

        /**
         * @brief Sign up to Firebase
         */
        void signUp();

        /**
         * @brief Connect to NTP server
         */
        void checkConnectNTP();

        /**
         * @brief Get time and date data from NTP server
         */
        void getTimeDataFromNtp();

        /**
         * @brief Start provisioning process for WiFi credentials
         */
        void startProvision();

        /**
         * @brief Static event handler for system provisioning events
         * @param sys_event The system event data
         */
        static void SysProvEvent(arduino_event_t *sys_event);

        /**
         * @brief Get SSID and Password from EEPROM
         * @param data The data package containing SSID and Password
         */
        void getSSIDAndPasswordFromEEPROM(const utils::Package *data);

        /**
         * @brief Upload data to Firebase
         * @param data The data package containing data to upload
         */
        void uploadDataToFirebase(const utils::Package *data);

        /**
         * @brief Remove WiFi event handlers
         */
        void removeEvent();

        /**
         * @brief Handle status from Firebase
         */
        void handleStatusFromFIREBASE();

        /**
         * @brief Create default data in Firebase
         */
        void createDefaultDataInFirebase();

        /**
         * @brief handles Firebase error
         */
        void handleFirebaseError();

         /**
         * @brief parse All time data from RTC module
         * @param data The data package containing SSID and Password
         */
        void parseAllTimeDataFromRTC(const utils::Package *data);
    };
}   // namespace remoteLight
#endif // NETWORK_H