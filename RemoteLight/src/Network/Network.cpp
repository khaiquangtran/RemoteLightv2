#include "./Network.h"

namespace remoteLight
{
    Network *Network::instance = nullptr;

    Network::Network(std::shared_ptr<RemoteLight> rml) : mRML(rml)
    {
        mSSID = WIFI_SSID;
        mPassword = WIFI_PASSWORD;
        mFlagProvision = false;
        mFlagOnEvent = false;
        mIsFirebaseConnected = false;
        mConfig.api_key = API_KEY;
        mConfig.database_url = DATABASE_URL;
        mAuth.user.email = USER_EMAIL;
        mAuth.user.password = USER_PASSWORD;
        instance = this; // save pointer instance
        WiFi.onEvent(this->SysProvEvent);
        mAllTimeStr = "0/0/0 0:0:0";
        mTimeClient = new NTPClient(ntpUDP, NTP_SERVER);
        LOGI(" =========== Network =========== ");
    }

    void Network::handleSignal(const utils::SignalType signal, utils::Package *data)
    {
        switch (signal)
        {
        case utils::SignalType::NETWORK_CHECK_STATUS_WIFI:
        {
            connectWifi();
            break;
        }
        case utils::SignalType::NETWORK_CHECK_STATUS_FIREBASE:
        {
            signUp();
            break;
        }
        case utils::SignalType::NETWORK_CHECK_STATUS_NTP:
        {
            checkConnectNTP();
            break;
        }
        case utils::SignalType::NETWORK_GET_TIME_DATE_FROM_NTP:
        {
            getTimeDataFromNtp();
            break;
        }
        case utils::SignalType::NETWORK_START_PROVISION:
        {
            startProvision();
            break;
        }
        case utils::SignalType::NETWORK_SSID_PASSWORD_STORED:
        {
            getSSIDAndPasswordFromEEPROM(data);
            break;
        }
        case utils::SignalType::NETWORK_REMOVE_EVENT:
        {
            removeEvent();
            break;
        }
        case utils::SignalType::NETWORK_HANDLE_STATUS_FROM_FIREBASE:
        {
            handleStatusFromFIREBASE();
            break;
        }
        case utils::SignalType::NETWORK_CREATE_DEFAULT_DATA_IN_FIREBASE:
        {
            createDefaultDataInFirebase();
            break;
        }
        case utils::SignalType::NETWORK_SEND_LIGHT_STATUS_TO_FIREBASE:
        {
            uploadDataToFirebase(data);
            break;
        }
        case utils::SignalType::RTC_SEND_ALL_TIME_DATA:
        {
            parseAllTimeDataFromRTC(data);
            break;
        }
        default:
            break;
        }
    }

    void Network::connectWifi()
    {
        if (mSSID == "" || mPassword == "")
        {
            LOGE("SSID or Password is empty!");
            mRML->handleSignal(utils::SignalType::TASKS_CONNECT_WIFI_FAILED_SSID_PASSWORD_EMPTY);
        }
        else
        {
            WiFi.begin(mSSID, mPassword);
        }
    }

    void Network::signUp()
    {
        // LOGD("Connecting to Firebase...");
        // if (Firebase.signUp(&mConfig, &mAuth) == true)
        // {
        Firebase.begin(&mConfig, &mAuth);
        if (Firebase.ready())
        {
            mRML->handleSignal(utils::SignalType::TASKS_CONNECT_FIREBASE_SUCCESS);
            mIsFirebaseConnected = true;
            Firebase.refreshToken(&mConfig);
        }
        else
        {
            LOGE("REASON: %s", mFbdo.errorReason().c_str());
            mRML->handleSignal(utils::SignalType::TASKS_CONNECT_FIREBASE_FAILED);
            mIsFirebaseConnected = false;
        }
        // }
        // else
        // {
        //     LOGE("REASON: %s", mFbdo.errorReason().c_str());
        //     mRML->handleSignal(utils::SignalType::TASKS_CONNECT_FIREBASE_FAILED);
        //     mIsFirebaseConnected = false;
        // }
    }

    void Network::checkConnectNTP()
    {
        mTimeClient->begin();
        mTimeClient->setTimeOffset(GMT);
        if (mTimeClient->update())
        {
            mRML->handleSignal(utils::SignalType::TASKS_CONNECT_NTP_SUCCESS);
        }
        else
        {
            LOGE("NTP server connect failed");
            mRML->handleSignal(utils::SignalType::TASKS_CONNECT_NTP_FAILED);
        }
    }

    void Network::removeEvent()
    {
        LOGD("Removing WiFi event handlers");
        mFlagOnEvent = true;
        WiFi.removeEvent(this->SysProvEvent);
    }

    void Network::uploadDataToFirebase(const utils::Package *data)
    {
        if (mIsFirebaseConnected == false)
        {
            LOGE("Firebase is not connected.");
            return;
        }
        else if (data == nullptr)
        {
            LOGE("Data from RTC is null.");
            return;
        }
        else
        {
            const int32_t *parseData = data->getPackage();
            if (parseData == nullptr)
            {
                LOGE("parseData is null");
                return;
            }

            int32_t index = parseData[0];
            if (index < 0 || index >= static_cast<int32_t>(HEAD_PATHS.size()))
            {
                LOGE("Invalid index: %d", index);
                return;
            }
            LOGD("Index: %d", index);
            String headPath = HEAD_PATHS.at(index);

            for (size_t i = 0; i < DATA_PATHS.size(); ++i)
            {
                String dataPath = headPath + DATA_PATHS[i];
                int32_t value = parseData[i + 1];

                if (!Firebase.RTDB.setInt(&mFbdo, dataPath.c_str(), value))
                {
                    handleFirebaseError();
                    return;
                }
            }
            String flagPath = headPath + FLAG_PATH;
            if (!Firebase.RTDB.setInt(&mFbdo, flagPath.c_str(),
                                      static_cast<int32_t>(REQUEST_FB::UPLOAD_INFORM)))
            {
                handleFirebaseError();
                return;
            }
        }
    }

    void Network::getTimeDataFromNtp()
    {
        LOGI(".");
        // Format: hour minute second day date month year
        std::vector<int32_t> vecData(7, 0);

        mTimeClient->update();
        time_t epochTime = mTimeClient->getEpochTime();

        if (epochTime > 0)
        {
            struct tm *ptm = gmtime(&epochTime);
            vecData[0] = static_cast<int32_t>(ptm->tm_hour);
            vecData[1] = static_cast<int32_t>(ptm->tm_min);
            vecData[2] = static_cast<int32_t>(ptm->tm_sec);
            vecData[3] = static_cast<int32_t>(ptm->tm_wday + 1);
            vecData[4] = static_cast<int32_t>(ptm->tm_mday);
            vecData[5] = static_cast<int32_t>(ptm->tm_mon + 1);     // tm_mon is 0-based
            vecData[6] = static_cast<int32_t>(ptm->tm_year + 1900); // tm_year is years since 1900
            std::unique_ptr<utils::Package> package = std::make_unique<utils::Package>(vecData);
            mRML->handleSignal(utils::SignalType::NETWORK_SEND_TIME_DATE_FROM_NTP, package.get());
        }
        else
        {
            LOGE("NTP server get time failed");
            mRML->handleSignal(utils::SignalType::LIGHT_LED_RESET_BTN_ON);
            mRML->handleSignal(utils::SignalType::TASKS_SET_NTP_ERROR);
        }
    }

    void Network::startProvision()
    {
        if (mFlagOnEvent == true)
        {
            mFlagOnEvent = false;
            WiFi.onEvent(this->SysProvEvent);
        }
        WiFiProv.beginProvision(
            WIFI_PROV_SCHEME_SOFTAP,
            WIFI_PROV_SCHEME_HANDLER_NONE,
            WIFI_PROV_SECURITY_1,
            POP,
            SERVICE_NAME,
            SERVICE_KEY,
            uuid,
            true);
        WiFiProv.printQR(SERVICE_NAME, POP, "softap");
        mFlagProvision = true;
    }

    void Network::SysProvEvent(arduino_event_t *sys_event)
    {
        switch (sys_event->event_id)
        {
        case ARDUINO_EVENT_WIFI_STA_GOT_IP:
        {
            uint32_t raw_ip = sys_event->event_info.got_ip.ip_info.ip.addr;
            IPAddress ip(raw_ip);
            char ssid[33] = {0};
            memcpy(
                ssid,
                sys_event->event_info.prov_cred_recv.ssid,
                sizeof(ssid) - 1);
            LOGD("Connect to WiFi SSID: %s", ssid);
            LOGD("Connected IP address : %s", ip.toString().c_str());
            if (instance->mFlagProvision == false)
            {
                instance->mRML->handleSignal(utils::SignalType::TASKS_CONNECT_WIFI_SUCCESS);
            }
            break;
        }
        case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
        {
            LOGE("Disconnected. Connecting to the AP again... ");
            break;
        }
        case ARDUINO_EVENT_PROV_START:
        {
            LOGD("Provisioning started");
            LOGD("Give Credentials of your access point using smartphone app");
            std::string SERVICE_NAME_STR(instance->SERVICE_NAME);
            std::string popStr(instance->POP); // POP should be same as SERVICE_KEY
            std::string dataStr = SERVICE_NAME_STR + "%" + popStr;
            std::unique_ptr<utils::Package> packData = std::make_unique<utils::Package>(dataStr);
            instance->mRML->handleSignal(utils::SignalType::LCD_START_PROVISIONING, packData.get());
            break;
        }
        case ARDUINO_EVENT_PROV_CRED_RECV:
        {
            LOGD("Received Wi-Fi credentials");
            LOGD("SSID : %s", (const char *)sys_event->event_info.prov_cred_recv.ssid);
            LOGD("Password : %s", (const char *)sys_event->event_info.prov_cred_recv.password);
            instance->mSSID = String((const char *)sys_event->event_info.prov_cred_recv.ssid);
            instance->mPassword = String((const char *)sys_event->event_info.prov_cred_recv.password);
            break;
        }
        case ARDUINO_EVENT_PROV_CRED_FAIL:
        {
            instance->mRML->handleSignal(utils::SignalType::REMOTE_LIGHT_REMOVE_WIFI_PROVISIONING_FAILED);
            LOGE("Provisioning failed! Please reset to factory and retry provisioning");
            if (sys_event->event_info.prov_fail_reason == WIFI_PROV_STA_AUTH_ERROR)
            {
                LOGE("Wi-Fi AP password incorrect");
            }
            else
            {
                LOGE("Wi-Fi AP not found....Add API \" nvs_flash_erase() \" before beginProvision()");
            }
            break;
        }
        case ARDUINO_EVENT_PROV_CRED_SUCCESS:
        {
            LOGD("Provisioning Successful");
            String ssid_password = static_cast<String>(instance->mSSID.c_str()) + "%" + static_cast<String>(instance->mPassword.c_str());
            std::unique_ptr<utils::Package> packData = std::make_unique<utils::Package>(ssid_password.c_str());
            instance->mRML->handleSignal(utils::SignalType::EEPROM_NETWORK_SEND_SSID_PASSWORD, packData.get());
            instance->mRML->handleSignal(utils::SignalType::LCD_PROVISIONING_SUCCESS);
            break;
        }
        case ARDUINO_EVENT_PROV_END:
        {
            LOGD("Provisioning Ends");
            instance->mRML->handleSignal(utils::SignalType::REMOTE_LIGHT_REMOVE_WIFI_PROVISIONING_DONE);
            break;
        }
        default:
            break;
        }
    }

    void Network::getSSIDAndPasswordFromEEPROM(const utils::Package *data)
    {
        if (data == nullptr)
        {
            LOGE("Data from EEPROM is null.");
            return;
        }
        else
        {
            const int32_t size = data->getSize();
            const int32_t *value = data->getPackage();
            if (size <= 0)
            {
                LOGE("Data from EEPROM with length is invalid.");
                return;
            }
            else
            {
                String str = "";
                for (int32_t i = 0; i < size; i++)
                {
                    str += static_cast<char>(value[i]);
                }

                int32_t sep = str.indexOf('%');
                if (sep != -1)
                {
                    mSSID = str.substring(0, sep);
                    mPassword = str.substring(sep + 1);

                    LOGI("SSID: %s", mSSID.c_str());
                    LOGI("Password: %s", mPassword.c_str());
                }
            }
        }
    }

    void Network::handleStatusFromFIREBASE()
    {
        if (mIsFirebaseConnected != true)
        {
            LOGW("Firebase is not connected.");
            return;
        }
        else
        {
            Firebase.reconnectWiFi(true);

            if (Firebase.ready())
            {
                if ((Firebase.RTDB.setString(&mFbdo, DATE_PATH.c_str(), mAllTimeStr.c_str())) == false)
                {
                    handleFirebaseError();
                    return;
                }
                int index = 0;
                for (std::array<String, 4U>::const_iterator it = HEAD_PATHS.begin(); it != HEAD_PATHS.end(); ++it)
                {
                    String headPath = String(it->c_str());
                    if (Firebase.RTDB.getJSON(&mFbdo, headPath))
                    {
                        FirebaseJson &json = mFbdo.jsonObject();
                        FirebaseJsonData jsonData;
                        if (json.get(jsonData, FLAG_PATH.c_str()))
                        {
                            int32_t flag = jsonData.intValue;
                            if (flag == static_cast<int32_t>(REQUEST_FB::SET_INFORM))
                            {
                                LOGI("Get flag LIGHT_%d is SET_INFORM", index + 1);
                                std::vector<int32_t> vecData;
                                vecData.push_back(index);
                                for (std::array<String, 8U>::const_iterator dataIt = DATA_PATHS.begin(); dataIt != DATA_PATHS.end(); ++dataIt)
                                {
                                    if (json.get(jsonData, dataIt->c_str()))
                                    {
                                        int32_t value = jsonData.intValue;
                                        vecData.push_back(value);
                                        LOGD("Get %s : %d", dataIt->c_str(), value);
                                    }
                                }
                                std::unique_ptr<utils::Package> packData = std::make_unique<utils::Package>(vecData);
                                mRML->handleSignal(utils::SignalType::EEPROM_RTC_SEND_LIGHT_ON_OFF_DATA, packData.get());

                                // After getting data, set flag to RECEIVED_INFORM
                                String flagPath = headPath + FLAG_PATH.c_str();
                                if (Firebase.RTDB.setInt(&mFbdo, flagPath.c_str(), static_cast<int32_t>(REQUEST_FB::RECEIVED_INFORM)))
                                {
                                    LOGI("Set flag LIGHT_%d to RECEIVED_INFORM", index + 1);
                                }
                                else
                                {
                                    handleFirebaseError();
                                    return;
                                }
                            }
                            else
                            {
                                LOGW("There is no new DATA from Firebase for LIGHT_%d", index + 1);
                            }
                        }
                        else
                        {
                            handleFirebaseError();
                            return;
                        }
                    }
                    else
                    {
                        handleFirebaseError();
                        return;
                    }
                    index++;
                }
            }
        }
    }

    void Network::createDefaultDataInFirebase()
    {
        if (mIsFirebaseConnected != true)
        {
            LOGW("Firebase is not connected.");
            return;
        }
        else
        {
            for (std::array<String, 4U>::const_iterator it = HEAD_PATHS.begin(); it != HEAD_PATHS.end(); ++it)
            {
                String flagPath = String(it->c_str()) + FLAG_PATH;
                if (Firebase.RTDB.setInt(&mFbdo, flagPath.c_str(), static_cast<int32_t>(REQUEST_FB::NONE)))
                {
                    LOGI("Create default flag LIGHT_%d in Firebase", std::distance(HEAD_PATHS.begin(), it) + 1);
                }
                else
                {
                    LOGE("REASON: %s", mFbdo.errorReason().c_str());
                }

                for (std::array<String, 8U>::const_iterator dataIt = DATA_PATHS.begin(); dataIt != DATA_PATHS.end(); ++dataIt)
                {
                    String path = String(it->c_str()) + String(dataIt->c_str());
                    if (Firebase.RTDB.setInt(&mFbdo, path.c_str(), 0))
                    {
                        LOGI("Create default data %s in Firebase", path.c_str());
                    }
                    else
                    {
                        LOGE("REASON: %s", mFbdo.errorReason().c_str());
                    }
                }
            }
        }
    }

    void Network::handleFirebaseError()
    {
        LOGE("REASON: %s", mFbdo.errorReason().c_str());
        mRML->handleSignal(utils::SignalType::TASKS_SET_FIREBASE_ERROR);
        mRML->handleSignal(utils::SignalType::LIGHT_LED_RESET_BTN_ON);
    }

    void Network::parseAllTimeDataFromRTC(const utils::Package *data)
    {
        if (data == nullptr)
        {
            LOGE("Data from EEPROM is null.");
            return;
        }
        else
        {
            const int32_t size = data->getSize();
            const int32_t *value = data->getPackage();
            if (size <= 0)
            {
                LOGE("Data from EEPROM with length is invalid.");
                return;
            }
            else
            {
                const int32_t *dataPtr = data->getPackage();
                const int32_t size = data->getSize();
                if (size == 6)
                {
                    if (size == 6)
                    {
                        mAllTimeStr = String(dataPtr[0]) + "/" +
                                      String(dataPtr[1]) + "/" +
                                      String(dataPtr[2]) + " " +
                                      String(dataPtr[3]) + ":" +
                                      String(dataPtr[4]) + ":" +
                                      String(dataPtr[5]);
                    }
                }
            }
        }
    }
} // namespace remoteLight