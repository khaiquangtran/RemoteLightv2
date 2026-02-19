#include "./EEPROMPartner.h"
#include "./../RemoteLight.h"

namespace hardware
{

    EEPROMPartner::EEPROMPartner(std::shared_ptr<remoteLight::RemoteLight> rml) : mRML(rml)
    {
        mMapOfDataOfIR = {
            {0, std::make_pair("BTN_1", 0)},
            {1, std::make_pair("BTN_2", 0)},
            {2, std::make_pair("BTN_3", 0)},
            {3, std::make_pair("BTN_4", 0)},
            {4, std::make_pair("BTN_5", 0)},
            {5, std::make_pair("BTN_UP", 0)},
            {6, std::make_pair("BTN_DOWN", 0)},
            {7, std::make_pair("BTN_RIGHT", 0)},
            {8, std::make_pair("BTN_LEFT", 0)},
            {9, std::make_pair("BTN_OK", 0)},
            {10, std::make_pair("BTN_MENU", 0)},
            {11, std::make_pair("BTN_APP", 0)},
            {12, std::make_pair("BTN_BACK", 0)},
        };

        mMapOfLightOnOffTime = {
            {0, std::make_pair("L1_SW_ON", 0)},
            {1, std::make_pair("L1_HR_ON", 0)},
            {2, std::make_pair("L1_MIN_ON", 0)},
            {3, std::make_pair("L1_SEC_ON", 0)},
            {4, std::make_pair("L1_SW_OFF", 0)},
            {5, std::make_pair("L1_HR_OFF", 0)},
            {6, std::make_pair("L1_MIN_OFF", 0)},
            {7, std::make_pair("L1_SEC_OFF", 0)},
            /*--------------------------------------------*/
            {8, std::make_pair("L2_SW_ON", 0)},
            {9, std::make_pair("L2_HR_ON", 0)},
            {10, std::make_pair("L2_MIN_ON", 0)},
            {11, std::make_pair("L2_SEC_ON", 0)},
            {12, std::make_pair("L2_SW_OFF", 0)},
            {13, std::make_pair("L2_HR_OFF", 0)},
            {14, std::make_pair("L2_MIN_OFF", 0)},
            {15, std::make_pair("L2_SEC_OFF", 0)},
            /*--------------------------------------------*/
            {16, std::make_pair("L3_SW_ON", 0)},
            {17, std::make_pair("L3_HR_ON", 0)},
            {18, std::make_pair("L3_MIN_ON", 0)},
            {19, std::make_pair("L3_SEC_ON", 0)},
            {20, std::make_pair("L3_SW_OFF", 0)},
            {21, std::make_pair("L3_HR_OFF", 0)},
            {22, std::make_pair("L3_MIN_OFF", 0)},
            {23, std::make_pair("L3_SEC_OFF", 0)},
            /*--------------------------------------------*/
            {24, std::make_pair("L4_SW_ON", 0)},
            {25, std::make_pair("L4_HR_ON", 0)},
            {26, std::make_pair("L4_MIN_ON", 0)},
            {27, std::make_pair("L4_SEC_ON", 0)},
            {28, std::make_pair("L4_SW_OFF", 0)},
            {29, std::make_pair("L4_HR_OFF", 0)},
            {30, std::make_pair("L4_MIN_OFF", 0)},
            {31, std::make_pair("L4_SEC_OFF", 0)},
        };

        mMapOfStateLight = {
            {LIGHT_1, "LIGHT_1"},
            {LIGHT_2, "LIGHT_2"},
            {LIGHT_3, "LIGHT_3"},
            {LIGHT_4, "LIGHT_4"},
        };

        NUMBER_BUTTONS = mMapOfDataOfIR.size();
        LOGI(" =========== EEPROMPartner =========== ");
    }

    void EEPROMPartner::init()
    {
        // Load data of IR remote buttons
        mDataPreferences.begin(IR_DATA.c_str(), false);
        std::map<int32_t, std::pair<String, int32_t>>::iterator it;
        for (it = mMapOfDataOfIR.begin(); it != mMapOfDataOfIR.end(); it++)
        {
            it->second.second = mDataPreferences.getInt(it->second.first.c_str(), it->second.second);
        }
        mDataPreferences.end();

        for (int8_t i = 0; i < mMapOfDataOfIR.size(); i++)
        {
            LOGD("Button: %s, data: %x", mMapOfDataOfIR.at(i).first.c_str(), mMapOfDataOfIR.at(i).second);
        }

        // Load light on/off time data
        mDataPreferences.begin(LIGHT_TIME_DATA.c_str(), false);
        for (it = mMapOfLightOnOffTime.begin(); it != mMapOfLightOnOffTime.end(); it++)
        {
            it->second.second = mDataPreferences.getInt(it->second.first.c_str(), it->second.second);
        }
        mDataPreferences.end();

        for (int8_t i = 0; i < mMapOfLightOnOffTime.size(); i++)
        {
            LOGD("%s : %d", mMapOfLightOnOffTime.at(i).first.c_str(), mMapOfLightOnOffTime.at(i).second);
        }

        // Load SSID and Password
        mDataPreferences.begin(SSID_PASSWORD_DATA.c_str(), false);
        // If there is no stored SSID and Password, create empty values
        if (!mDataPreferences.isKey(SSID.c_str()))
        {
            mDataPreferences.putString(SSID.c_str(), "");
            mDataPreferences.putString(PASSWORD.c_str(), "");
            mSsid = "";
            mPassword = "";
            LOGW("No stored SSID and Password");
        }
        else
        {
            mSsid = mDataPreferences.getString(SSID.c_str(), "");
            mPassword = mDataPreferences.getString(PASSWORD.c_str(), "");
            LOGI("Storing SSID: %s", mSsid.c_str());
            LOGI("Storing Password: %s", mPassword.c_str());
        }
        mDataPreferences.end();
    }

    void EEPROMPartner::handleSignal(const utils::SignalType &signal, const utils::Package *data)
    {
        switch (signal)
        {
        case (utils::SignalType::EEPROM_IR_REMOTE_GET_DATA):
        {
            /**
             * @brief Send data from IR Remote stored in EEPROM to IR Remote module
             */
            sendDataFromIRRemoteToIRModule();
            break;
        }
        case (utils::SignalType::EEPROM_STORE_IRBUTTON_INSTALL):
        {
            /**
             * @brief Save the final button configuration from the IR remote module.
            */
            storeIRButtonInstallData(data);
            break;
        }
        case utils::SignalType::EEPROM_IS_STORED_SSID_PASSWORD:
        {
            /**
             * @brief Send SSID and Password stored in EEPROM to Network module
             */
            sendSSIDAndPasswordToNetwork();
            break;
        }
        case utils::SignalType::EEPROM_SEND_LIGHT_TIME_ON_OFF_DATA_TO_RTC:
        {
            /**
             * @brief Send light on/off time data stored in EEPROM to RTC module
             */
            sendLightOnOffDataToRTC();
            break;
        }
        case utils::SignalType::EEPROM_NETWORK_SEND_SSID_PASSWORD:
        {
            /**
             * @brief Store SSID and Password received from Network module
             */
            storedSsidPassFromNetwork(data);
            break;
        }
        case utils::SignalType::EEPROM_CLEAR_SSID_PASSOWRD_DATA:
        {
            /**
             * @brief Clear SSID and Password stored in EEPROM
             */
            clearSSIDPasswordData();
            break;
        }
        case utils::SignalType::EEPROM_RTC_SEND_LIGHT_ON_OFF_DATA:
        {
            /**
             * @brief Store light on/off data received from RTC module
             */
            storeLightOnOffDataFromRTC(data);
            break;
        }
        case utils::SignalType::EEPROM_STORE_LIGHT_IS_ENABLE_BUTTON_LED:
        {
            /**
             * @brief Store button LED enable state
             */
            storeLightIsEnableButtonLED(data);
            break;
        }
        case utils::SignalType::EEPROM_SEND_LIGHT_IS_ENABLE_BUTTON_LED:
        {
            /**
             * @brief Send button LED enable state to Light module
             */
            sendLightIsEnableButtonLED();
            break;
        }
        case utils::SignalType::EEPROM_STORE_STATE_LIGHT:
        {
            /**
             * @brief Store light state data
             */
            storeLightStateData(data);
            break;
        }
        case utils::SignalType::EEPROM_SEND_STATE_LIGHT_TO_LIGHT:
        {
            /**
             * @brief Send light state data to Light module
             */
            sendLightStateDataToLight();
            break;
        }
        default:
            break;
        }
    }

    void EEPROMPartner::storeLightOnOffDataFromRTC(const utils::Package *data)
    {
        if (data == nullptr)
        {
            LOGE("Data from RTC is null.");
            return;
        }
        else
        {
            const int32_t size = data->getSize();
            const int32_t EXPECT_SIZE = 9U;
            if (size != EXPECT_SIZE)
            {
                LOGE("Data from RTC with length is %d.", size);
                return;
            }
            else
            {
                mDataPreferences.begin(LIGHT_TIME_DATA.c_str(), false);
                const int32_t *value = data->getPackage();

                // Number of data elements for each light block (e.g., 1 block = 8 time points)
                const int32_t LIGHT_BLOCK = 8U;
                // value[0] contains the index of the light to be configured (which light number)
                const int32_t indexLight = value[0];
                // Calculate the starting index in the map corresponding to this light
                // -1 because the map index starts from 0
                const int32_t baseIndex = indexLight * LIGHT_BLOCK - 1U;

                for (int32_t i = 1U; i < EXPECT_SIZE; i++)
                {
                    mMapOfLightOnOffTime.at(i + baseIndex).second = value[i];
                    mDataPreferences.putInt(mMapOfLightOnOffTime.at(i + baseIndex).first.c_str(), mMapOfLightOnOffTime.at(i + baseIndex).second);
                    LOGD("%s : %d", mMapOfLightOnOffTime.at(i + baseIndex).first.c_str(), mMapOfLightOnOffTime.at(i + baseIndex).second);
                    delay(10);
                }
                mDataPreferences.end();
            }
        }
    }

    void EEPROMPartner::storeLightIsEnableButtonLED(const utils::Package *data)
    {
        if (data == nullptr)
        {
            LOGE("Data from RTC is null.");
            return;
        }
        else
        {
            const int32_t size = data->getSize();
            const int32_t EXPECT_SIZE = 1U;
            if (size != EXPECT_SIZE)
            {
                LOGE("Data from RTC with length is %d.", size);
                return;
            }
            else
            {
                mDataPreferences.begin(BUTTON_LED_DATA.c_str(), false);
                const int32_t *value = data->getPackage();
                mDataPreferences.putInt(BUTTON_LED_DATA.c_str(), value[0]);
                delay(10);
                mDataPreferences.end();
            }
        }
    }

    void EEPROMPartner::sendLightOnOffDataToRTC()
    {
        // Pack the data in mMapOfLightOnOffTime to send to the RTC
        std::vector<int32_t> vecData;
        for (size_t i = 0; i < mMapOfLightOnOffTime.size(); i++)
        {
            vecData.push_back(static_cast<int32_t>(mMapOfLightOnOffTime.at(i).second));
        }
        std::unique_ptr<utils::Package> packData = std::make_unique<utils::Package>(vecData);
        mRML->handleSignal(utils::SignalType::RTC_GET_LIGHT_ON_OFF_DATA, packData.get());
    }

    void EEPROMPartner::storedSsidPassFromNetwork(const utils::Package *data)
    {
        if (data == nullptr)
        {
            LOGE("Data from Network is null.");
            return;
        }
        else
        {
            const int32_t size = data->getSize();
            const int32_t *value = data->getPackage();
            if (size <= 0)
            {
                LOGE("Length is invalid");
                return;
            }
            String str = "";
            for (int32_t i = 0; i < size; i++)
            {
                str += static_cast<char>(value[i]);
            }

            const int32_t sep = str.indexOf('%');
            if (sep != -1)
            {
                mSsid = str.substring(0, sep);
                mPassword = str.substring(sep + 1);

                LOGI("Storing SSID: %s", mSsid.c_str());
                LOGI("Storing Password: %s", mPassword.c_str());

                mDataPreferences.begin(SSID_PASSWORD_DATA.c_str(), false);
                mDataPreferences.putString(SSID.c_str(), mSsid.c_str());
                delay(10);
                mDataPreferences.putString(PASSWORD.c_str(), mPassword.c_str());
                delay(10);
                mDataPreferences.end();
            }
        }
    }

    void EEPROMPartner::sendLightIsEnableButtonLED()
    {
        mDataPreferences.begin(BUTTON_LED_DATA.c_str(), true);
        int32_t buttonLedState = mDataPreferences.getInt(BUTTON_LED_DATA.c_str(), 0);
        mDataPreferences.end();

        LOGD("Button LED enabled : %d", buttonLedState);
        std::vector<int32_t> vecData{buttonLedState};
        std::unique_ptr<utils::Package> packData = std::make_unique<utils::Package>(vecData);
        mRML->handleSignal(utils::SignalType::LIGHT_IS_ENABLE_BUTTON_LED, packData.get());
    }

    void EEPROMPartner::storeLightStateData(const utils::Package *data)
    {
        if (data == nullptr)
        {
            LOGE("Data from RTC is null.");
            return;
        }
        else
        {
            const int32_t size = data->getSize();
            const int32_t EXPECT_SIZE = 2U;
            if (size != EXPECT_SIZE)
            {
                LOGE("Data from RTC with length is %d.", size);
                return;
            }
            else
            {
                // Store light state data
                mDataPreferences.begin(STATE_LIGHT_DATA.c_str(), false);
                const int32_t *value = data->getPackage();

                // value[0] : light index
                // value[1] : light state
                uint8_t lightIndex = static_cast<uint8_t>(value[0]);
                String key = mMapOfStateLight[lightIndex];
                mDataPreferences.putInt(key.c_str(), value[1]);
                delay(10);
                mDataPreferences.end();
            }
        }
    }

    void EEPROMPartner::sendLightStateDataToLight()
    {
        std::vector<int32_t> vecData;
        mDataPreferences.begin(STATE_LIGHT_DATA.c_str(), true);
        for (const auto &pair : mMapOfStateLight)
        {
            String key = pair.second;
            const int32_t lightState = mDataPreferences.getInt(key.c_str(), 0);
            LOGD("Light %s state : %d", key.c_str(), lightState);

            // Prepare data to send to Light module
            // First element: light index
            // Second element: light state
            vecData.push_back(static_cast<int32_t>(pair.first));
            vecData.push_back(lightState);
        }
        mDataPreferences.end();

        std::unique_ptr<utils::Package> packData = std::make_unique<utils::Package>(vecData);
        mRML->handleSignal(utils::SignalType::LIGHT_STATE_FROM_EEPROM, packData.get());
    }

    void EEPROMPartner::sendDataFromIRRemoteToIRModule()
    {
        // Pack the data in mMapOfDataOfIR to send to the IR Remote module
        std::vector<int32_t> vecData(mMapOfDataOfIR.size());
        for (size_t i = 0; i < mMapOfDataOfIR.size(); i++)
        {
            vecData[i] = mMapOfDataOfIR.at(i).second;
        }
        std::unique_ptr<utils::Package> packData = std::make_unique<utils::Package>(vecData);
        mRML->handleSignal(utils::SignalType::IR_ERRPROM_SEND_DATA, packData.get());
    }

    void EEPROMPartner::storeIRButtonInstallData(const utils::Package *data)
    {
        if (data->getPackage() == nullptr)
        {
            LOGE("Data is NULL");
        }
        else
        {
            const int32_t *value = data->getPackage();
            const int32_t size = data->getSize();
            if (size != 2)
            {
                LOGE("Length is invalid");
            }
            else
            {
                const int32_t numberButton = static_cast<int32_t>(value[0]);
                const int32_t dataButton = static_cast<int32_t>(value[1]);
                if (mMapOfDataOfIR.find(numberButton) == mMapOfDataOfIR.end())
                {
                    LOGE("Button number %d is invalid", numberButton);
                }
                else
                {
                    // Store the button data
                    mDataPreferences.begin(IR_DATA.c_str(), false);
                    // Update in the map and store in Preferences
                    mMapOfDataOfIR.at(numberButton).second = dataButton;
                    mDataPreferences.putInt(mMapOfDataOfIR.at(numberButton).first.c_str(), dataButton);
                    delay(10);
                    LOGD("Stored button %s with data: %x", mMapOfDataOfIR.at(numberButton).first.c_str(), dataButton);
                    mDataPreferences.end();
                }
            }
        }
    }

    void EEPROMPartner::sendSSIDAndPasswordToNetwork()
    {
        if (mSsid == "" || mPassword == "")
        {
            LOGW("SSID or Password is empty");
        }
        else
        {
            String ssid_password = mSsid + String('%') + mPassword;
            std::unique_ptr<utils::Package> packData = std::make_unique<utils::Package>(ssid_password.c_str());
            mRML->handleSignal(utils::SignalType::NETWORK_SSID_PASSWORD_STORED, packData.get());
        }
    }

    void EEPROMPartner::clearSSIDPasswordData()
    {
        mDataPreferences.begin(SSID_PASSWORD_DATA.c_str(), false);
        mDataPreferences.putString(SSID.c_str(), "");
        delay(10);
        mDataPreferences.putString(PASSWORD.c_str(), "");
        delay(10);
        mDataPreferences.end();
        mSsid = "";
        mPassword = "";
        LOGI("Cleared SSID and Password stored in EEPROM");
    }
} // namespace hardware