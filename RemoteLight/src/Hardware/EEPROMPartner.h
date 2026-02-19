#ifndef EEPROM_PARTNER_H
#define EEPROM_PARTNER_H

#include <Preferences.h>

#include "./HardwareBase.h"
#include "IRRemotePartner.h"

namespace remoteLight {
    class RemoteLight;
}

namespace hardware
{
    class EEPROMPartner : public HardwareBase
    {
    public:
        EEPROMPartner(std::shared_ptr<remoteLight::RemoteLight> rml);
        EEPROMPartner(const EEPROMPartner &) = delete;
        EEPROMPartner &operator=(const EEPROMPartner &) = delete;
        virtual ~EEPROMPartner() = default;

        /**
         * @brief handle signal from other module
         * @param signal The signal to be handled
         * @param data Optional data associated with the signal
         */
        void handleSignal(const utils::SignalType &signal, const utils::Package *data = nullptr);

        /**
         * @brief Initialize EEPROM hardware
         */
        void init();

    private:
        std::shared_ptr<remoteLight::RemoteLight> mRML;

        Preferences mDataPreferences;
        std::map<int32_t, std::pair<String, int32_t>> mMapOfDataOfIR;
        std::map<int32_t, std::pair<String, int32_t>> mMapOfLightOnOffTime;
        std::map<uint8_t, String> mMapOfStateLight;
        int32_t NUMBER_BUTTONS;

        String mSsid;
        String mPassword;

        const String LIGHT_TIME_DATA = "light_time_data";
        const String IR_DATA = "ir_data";
        const String SSID_PASSWORD_DATA = "ssid_password";
        const String SSID = "ssid";
        const String PASSWORD = "password";
        const String BUTTON_LED_DATA = "button_led";
        const String STATE_LIGHT_DATA = "state_light";
        /**
         * @brief Send light on/off data to RTC
         */
        void sendLightOnOffDataToRTC();

        /**
         * @brief Store SSID and password received from Network module
         * @param data The data package containing SSID and password
         */
        void storedSsidPassFromNetwork(const utils::Package *data);

        /**
         * @brief Store light on/off data received from RTC
         * @param data The data package containing light on/off data
         */
        void storeLightOnOffDataFromRTC(const utils::Package *data);

        /**
         * @brief Store button LED enable state
         * @param data The data package containing button LED state
         */
        void storeLightIsEnableButtonLED(const utils::Package *data);

        /**
         * @brief Send button LED enable state to Light module
         */
        void sendLightIsEnableButtonLED();

        /**
         * @brief Store light state data
         * @param data The data package containing light state data
         */
        void storeLightStateData(const utils::Package *data);

        /**
         * @brief Send light state data to Light module
         */
        void sendLightStateDataToLight();

        /**
         * @brief Send data from IR Remote to IR module
         */
        void sendDataFromIRRemoteToIRModule();

        /**
         * @brief Store IR button install data
         * @param data The data package containing IR button install data
         */
        void storeIRButtonInstallData(const utils::Package *data);

        /**
         * @brief Send SSID and Password to Network module
         */
        void sendSSIDAndPasswordToNetwork();

        /**
         * @brief Clear SSID and Password stored in EEPROM
         */
        void clearSSIDPasswordData();
    };
}

#endif // EEPROM_PARTNER_H