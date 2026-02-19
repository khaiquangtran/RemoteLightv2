#include "./Tasks.h"

namespace remoteLight
{
    Tasks::Tasks(std::shared_ptr<RemoteLight> rml,
                 std::shared_ptr<hardware::HardwareBase> lcd,
                 std::shared_ptr<hardware::HardwareBase> rtc,
                 std::shared_ptr<hardware::HardwareBase> ir,
                 std::shared_ptr<hardware::HardwareBase> light,
                 std::shared_ptr<Network> net) : mRML(rml), mLCD(lcd), mRTC(rtc), mIR(ir), mLight(light), mNET(net)
    {
        mCounterConnect = 0U;
        mCounterDisplayAllTime = 0U;
        mModeHandle = MODE_HANDLE::NONE;
        mFlagError = 0U;
    }

    void Tasks::handleControlMode(const utils::CONTROL_MODE mode)
    {
        switch (mode)
        {
        case (utils::CONTROL_MODE::CONNECT_WIFI):
        {
            connectWifiMode();
            break;
        }
        case (utils::CONTROL_MODE::CONNECT_WIFI_DONE):
        {
            mCounterConnect = 0;
            LOGI("WIFI connection SUCCESS!");
            mLCD->handleSignal(utils::SignalType::LCD_CONNECT_WIFI_SUCCESS);
            break;
        }
        case (utils::CONTROL_MODE::CONNECT_WIFI_FAILED):
        {
            LOGW("WIFI connection FAILED!");
            mCounterConnect = 0;
            setErrorMode(ERROR_FLAG::WIFI_ERROR);
            mLight->handleSignal(utils::SignalType::LIGHT_LED_RESET_BTN_ON);
            mLCD->handleSignal(utils::SignalType::LCD_CONNECT_WIFI_FAILED);
            break;
        }
        case (utils::CONTROL_MODE::DISPLAY_ALL):
        {
            displayAllTime();
            break;
        }
        case (utils::CONTROL_MODE::CONNECT_FIREBASE):
        {
            connectFirebaseMode();
            break;
        }

        case (utils::CONTROL_MODE::CONNECT_FIREBASE_DONE):
        {
            mCounterConnect = 0;
            LOGI("FIREBASE connection SUCCESS!");
            mLCD->handleSignal(utils::SignalType::LCD_CONNECT_FIREBASE_SUCCESS);
            break;
        }
        case (utils::CONTROL_MODE::CONNECT_FIREBASE_GET_TOKEN):
        {
            mRTC->handleSignal(utils::SignalType::RTC_HANDLE_ALL_TIME_DATA);
            mNET->handleSignal(utils::SignalType::NETWORK_HANDLE_STATUS_FROM_FIREBASE);
            mRML->handleSignal(utils::SignalType::EEPROM_SEND_LIGHT_TIME_ON_OFF_DATA_TO_RTC);
            break;
        }
        case (utils::CONTROL_MODE::CONNECT_FIREBASE_FAILED):
        {
            mCounterConnect = 0;
            LOGW("FIREBASE connection FAILED!");
            setErrorMode(ERROR_FLAG::FIREBASE_ERROR);
            mLCD->handleSignal(utils::SignalType::LCD_CONNECT_FIREBASE_FAILED);
            mLight->handleSignal(utils::SignalType::LIGHT_LED_RESET_BTN_ON);
            mRML->handleSignal(utils::SignalType::EEPROM_SEND_LIGHT_TIME_ON_OFF_DATA_TO_RTC);
            break;
        }
        case (utils::CONTROL_MODE::CONNECT_NTP):
        {
            connectNTPMode();
            break;
        }
        case (utils::CONTROL_MODE::CONNECT_NTP_DONE):
        {
            mCounterConnect = 0;
            mCounterDisplayAllTime = 0;
            LOGI("NTP connection SUCCESS!");
            mLCD->handleSignal(utils::SignalType::LCD_CONNECT_NTP_SUCCESS);
            mNET->handleSignal(utils::SignalType::NETWORK_GET_TIME_DATE_FROM_NTP);
            break;
        }
        case (utils::CONTROL_MODE::CONNECT_NTP_FAILED):
        {
            LOGW("NTP connection FAILED!");
            mLight->handleSignal(utils::SignalType::LIGHT_LED_RESET_BTN_ON);
            mLCD->handleSignal(utils::SignalType::LCD_CONNECT_NTP_FAILED);
            break;
        }
        case (utils::CONTROL_MODE::SETTING_BUTTON_IR_REMOTE_START):
        {
            installIRButtonModeStart();
            break;
        }
        case (utils::CONTROL_MODE::SETTING_BUTTON_IR_REMOTE):
        {
            installIRButton();
            break;
        }
        case (utils::CONTROL_MODE::SETTING_BUTTON_IR_REMOTE_DONE):
        {
            if(mModeHandle == MODE_HANDLE::INSTALL_IR_BUTTON)
            {
                mModeHandle = MODE_HANDLE::NONE;
            }
            mLCD->handleSignal(utils::SignalType::LCD_INSTALL_BUTTON_DONE);
            break;
        }
        case (utils::CONTROL_MODE::SETTING_RTC_TIME_START):
        {
            mLCD->handleSignal(utils::SignalType::LCD_CLEAR_SCREEN);
            mLCD->handleSignal(utils::SignalType::LCD_TURN_ON_LIGHT);
            mLCD->handleSignal(utils::SignalType::LCD_SETTING_RTC_TIME_START);
            break;
        }
        case (utils::CONTROL_MODE::SETTING_RTC_TIME):
        {
            mRTC->handleSignal(utils::SignalType::RTC_SETTING_RTC_TIME_START);
            break;
        }
        case (utils::CONTROL_MODE::SETTING_RTC_TIME_UP):
        {
            mRTC->handleSignal(utils::SignalType::RTC_SETTING_RTC_INCREASE_VALUE);
            break;
        }
        case (utils::CONTROL_MODE::SETTING_RTC_TIME_DOWN):
        {
            mRTC->handleSignal(utils::SignalType::RTC_SETTING_RTC_DECREASE_VALUE);
            break;
        }
        case (utils::CONTROL_MODE::SETTING_RTC_TIME_LEFT):
        {
            mRTC->handleSignal(utils::SignalType::RTC_SETTING_RTC_SHIFT_LEFT_VALUE);
            break;
        }
        case (utils::CONTROL_MODE::SETTING_RTC_TIME_RIGHT):
        {
            mRTC->handleSignal(utils::SignalType::RTC_SETTING_RTC_SHIFT_RIGHT_VALUE);
            break;
        }
        case (utils::CONTROL_MODE::SETTING_RTC_TIME_OK):
        {
            mRTC->handleSignal(utils::SignalType::RTC_SETTING_RTC_TIME_OK);
            mLCD->handleSignal(utils::SignalType::LCD_CLEAR_SCREEN);
            mLCD->handleSignal(utils::SignalType::LCD_SETTING_RTC_TIME_END);
            mRML->handleSignal(utils::SignalType::REMOTE_LIGHT_SETTING_RTC_TIME_DONE);
            break;
        }
        case (utils::CONTROL_MODE::SETTING_RTC_TIME_DONE):
        {
            mLCD->handleSignal(utils::SignalType::LCD_SETTING_RTC_TIME_END);
            break;
        }
        case (utils::CONTROL_MODE::SETTING_LIGHT_TIME_CONFIG):
        {
            if (mModeHandle == MODE_HANDLE::NONE)
            {
                LOGI("Start MENU mode!");
                mModeHandle = MODE_HANDLE::SETTING_TIME_CONFIG;
                mLCD->handleSignal(utils::SignalType::LCD_CLEAR_SCREEN);
                mLCD->handleSignal(utils::SignalType::LCD_TURN_ON_LIGHT);
                mLCD->handleSignal(utils::SignalType::IR_BTN_MENU_SIGNAL);
            }
            break;
        }
        case (utils::CONTROL_MODE::SETTING_LIGHT_TIME_CONFIG_LEFT):
        {
            if (mModeHandle == MODE_HANDLE::SETTING_TIME_CONFIG)
            {
                mRTC->handleSignal(utils::SignalType::RTC_MOVE_LEFT_SETTING_CONFIG);
            }
            else if (mModeHandle == MODE_HANDLE::INTO_SETTING_TIME_CONFIG)
            {
                mRTC->handleSignal(utils::SignalType::RTC_MOVE_LEFT_INTO_SETTING_CONFIG);
            }
            break;
        }
        case (utils::CONTROL_MODE::SETTING_LIGHT_TIME_CONFIG_RIGHT):
        {
            if (mModeHandle == MODE_HANDLE::SETTING_TIME_CONFIG)
            {
                mRTC->handleSignal(utils::SignalType::RTC_MOVE_RIGHT_SETTING_CONFIG);
            }
            else if (mModeHandle == MODE_HANDLE::INTO_SETTING_TIME_CONFIG)
            {
                mRTC->handleSignal(utils::SignalType::RTC_MOVE_RIGHT_INTO_SETTING_CONFIG);
            }
            break;
        }
        case (utils::CONTROL_MODE::SETTING_LIGHT_TIME_CONFIG_UP):
        {
            if (mModeHandle == MODE_HANDLE::INTO_SETTING_TIME_CONFIG)
            {
                mRTC->handleSignal(utils::SignalType::RTC_INCREASE_VALUE_SETTING_LIGHT_CONFIG);
            }
            break;
        }
        case (utils::CONTROL_MODE::SETTING_LIGHT_TIME_CONFIG_DOWN):
        {
            if (mModeHandle == MODE_HANDLE::INTO_SETTING_TIME_CONFIG)
            {
                mRTC->handleSignal(utils::SignalType::RTC_DECREASE_VALUE_SETTING_LIGHT_CONFIG);
            }
            break;
        }
        case (utils::CONTROL_MODE::SETTING_LIGHT_TIME_CONFIG_OK):
        {
            if (mModeHandle == MODE_HANDLE::SETTING_TIME_CONFIG)
            {
                mModeHandle = MODE_HANDLE::INTO_SETTING_TIME_CONFIG;
                mLCD->handleSignal(utils::SignalType::LCD_CLEAR_SCREEN);
                mRTC->handleSignal(utils::SignalType::RTC_OK_SETTING_CONFIG);
            }
            else if (mModeHandle == MODE_HANDLE::INTO_SETTING_TIME_CONFIG)
            {
                mModeHandle = MODE_HANDLE::SETTING_TIME_CONFIG;
                mLCD->handleSignal(utils::SignalType::LCD_CLEAR_SCREEN);
                mRTC->handleSignal(utils::SignalType::RTC_OK_INTO_SETTING_CONFIG);
            }
            break;
        }
        case (utils::CONTROL_MODE::SETTING_LIGHT_TIME_CONFIG_BACK):
        {
            if (mModeHandle == MODE_HANDLE::SETTING_TIME_CONFIG)
            {
                mModeHandle = MODE_HANDLE::NONE;
                mLCD->handleSignal(utils::SignalType::LCD_CLEAR_SCREEN);
                mRML->handleSignal(utils::SignalType::REMOTE_LIGHT_SETTING_LIGHT_TIME_CONFIG_DONE);
            }
            break;
        }
        case (utils::CONTROL_MODE::WIFI_PROVISIONING_START):
        {
            mLCD->handleSignal(utils::SignalType::LCD_TURN_ON_LIGHT);
            mNET->handleSignal(utils::SignalType::NETWORK_START_PROVISION);
            break;
        }
        case (utils::CONTROL_MODE::WIFI_PROVISIONING_FAILED):
        {
            mLCD->handleSignal(utils::SignalType::LCD_PROVISIONING_FAILED);
            break;
        }
        case (utils::CONTROL_MODE::SET_WIFI_ERROR):
        {
            setErrorMode(ERROR_FLAG::WIFI_ERROR);
            break;
        }
        case (utils::CONTROL_MODE::SET_FIREBASE_ERROR):
        {
            setErrorMode(ERROR_FLAG::FIREBASE_ERROR);
            break;
        }
        case (utils::CONTROL_MODE::SET_NTP_ERROR):
        {
            setErrorMode(ERROR_FLAG::NTP_ERROR);
            break;
        }
        case (utils::CONTROL_MODE::CHECK_NTP_TIME_UPDATE):
        {
            handleNTPTimeUpdate();
            break;
        }
        case (utils::CONTROL_MODE::HANDLE_ERROR):
        {
            hanldeError();
            break;
        }
        case (utils::CONTROL_MODE::CONNEC_WIFI_FAILED_SSID_PASSWORD_EMPTY):
        {
            mCounterConnect = REPEATS_5;
            break;
        }
        default:
            break;
        }
    }

    void Tasks::connectWifiMode()
    {
        if (mCounterConnect == 0)
        {
            mLCD->handleSignal(utils::SignalType::LCD_DISPLAY_START_CONNECT_WIFI);
        }
        if (mCounterConnect < REPEATS_5)
        {
            mCounterConnect++;
            LOGI("WIFI connection %d times", mCounterConnect);
            mLCD->handleSignal(utils::SignalType::LCD_DISPLAY_CONNECT_WIFI);
            mNET->handleSignal(utils::SignalType::NETWORK_CHECK_STATUS_WIFI);
        }
        else
        {
            LOGW("WIFI connection FAILED!");
            mCounterConnect = 0;
            setErrorMode(ERROR_FLAG::WIFI_ERROR);
            mLCD->handleSignal(utils::SignalType::LCD_CONNECT_WIFI_FAILED);
            mLight->handleSignal(utils::SignalType::LIGHT_LED_RESET_BTN_ON);
            mNET->handleSignal(utils::SignalType::NETWORK_REMOVE_EVENT);
            mRML->handleSignal(utils::SignalType::REMOTE_LIGHT_CONNECT_WIFI_TIMEOUT);
        }
    }

    void Tasks::connectNTPMode()
    {
        if (mCounterConnect == 0)
        {
            mLCD->handleSignal(utils::SignalType::LCD_CLEAR_SCREEN);
            mLCD->handleSignal(utils::SignalType::LCD_DISPLAY_START_CONNECT_NTP);
        }
        if (mCounterConnect < REPEATS_10)
        {
            LOGI("NTP connection %d times", mCounterConnect);
            mCounterConnect++;
            mLCD->handleSignal(utils::SignalType::LCD_DISPLAY_CONNECT_WIFI);
            mNET->handleSignal(utils::SignalType::NETWORK_CHECK_STATUS_NTP);
        }
        else
        {
            LOGW("NTP connection FAILED!");
            mCounterConnect = 0;
            setErrorMode(ERROR_FLAG::NTP_ERROR);
            mLCD->handleSignal(utils::SignalType::LCD_CONNECT_NTP_FAILED);
            mLight->handleSignal(utils::SignalType::LIGHT_LED_RESET_BTN_ON);
            mRML->handleSignal(utils::SignalType::REMOTE_LIGHT_CONNECT_NTP_TIMEOUT);
        }
    }

    void Tasks::connectFirebaseMode()
    {
        if (mCounterConnect == 0)
        {
            mLCD->handleSignal(utils::SignalType::LCD_CLEAR_SCREEN);
            mLCD->handleSignal(utils::SignalType::LCD_DISPLAY_START_CONNECT_FIREBASE);
        }
        if (mCounterConnect < REPEATS_10)
        {
            LOGI("FIREBASE connection %d times", mCounterConnect);
            mCounterConnect++;
            mLCD->handleSignal(utils::SignalType::LCD_DISPLAY_CONNECT_WIFI);
            mNET->handleSignal(utils::SignalType::NETWORK_CHECK_STATUS_FIREBASE);
        }
        else
        {
            LOGW("Firebase connection FAILED!");
            mCounterConnect = 0;
            setErrorMode(ERROR_FLAG::FIREBASE_ERROR);
            mLCD->handleSignal(utils::SignalType::LCD_CONNECT_FIREBASE_FAILED);
            mLight->handleSignal(utils::SignalType::LIGHT_LED_RESET_BTN_ON);
            mRML->handleSignal(utils::SignalType::REMOTE_LIGHT_CONNECT_FIREBASE_TIMEOUT);
        }
    }

    void Tasks::displayAllTime()
    {
        LOGD("mCounterDisplayAllTime: %d", mCounterDisplayAllTime);
        if (mCounterDisplayAllTime < REPEATS_30)
        {
            mCounterDisplayAllTime++;
            mRTC->handleSignal(utils::SignalType::RTC_DISPLAY_ALL_TIME);
        }
        else if (mCounterDisplayAllTime == REPEATS_30)
        {
            mCounterDisplayAllTime = 0;
            mLCD->handleSignal(utils::SignalType::LCD_CLEAR_TURN_OFF_SCREEN);
            mRML->handleSignal(utils::SignalType::REMOTE_LIGHT_DISPLAY_ALL_TIME_MODE_FINISH);
        }
    }

    void Tasks::installIRButton()
    {
        LOGD("Install IR button mode.");
        if (mModeHandle == MODE_HANDLE::NONE)
        {
            mModeHandle = MODE_HANDLE::INSTALL_IR_BUTTON;
            mIR->handleSignal(utils::SignalType::IR_INSTALL_BUTTON);
            mLCD->handleSignal(utils::SignalType::LCD_INSTALL_BUTTON1);
        }
        else if (mModeHandle == MODE_HANDLE::INSTALL_IR_BUTTON)
        {
            LOGI("Install button Done. Move next button");
            mIR->handleSignal(utils::SignalType::IR_INSTALL_BUTTON_DONE);
        }
        else
        {
            // Do nothing
        }
    }

    void Tasks::installIRButtonModeStart()
    {
        mLCD->handleSignal(utils::SignalType::LCD_CLEAR_SCREEN);
        mLCD->handleSignal(utils::SignalType::LCD_TURN_ON_LIGHT);
        mLCD->handleSignal(utils::SignalType::LCD_INSTALL_BUTTON_START);
    }

    void Tasks::setErrorMode(const ERROR_FLAG& error)
    {
        switch (error)
        {
        case ERROR_FLAG::WIFI_ERROR:
            mFlagError |= 0x01;
            break;
        case ERROR_FLAG::FIREBASE_ERROR:
            mFlagError |= 0x02;
            break;
        case ERROR_FLAG::NTP_ERROR:
            mFlagError |= 0x04;
            break;
        default:
            LOGW("Unknown error flag: %d", static_cast<uint8_t>(error));
        }
    }

    void Tasks::handleNTPTimeUpdate()
    {
        if( (mFlagError & 0x05) == 0x00)
        {
            LOGD("Handle NTP time update check.");
            mRML->handleSignal(utils::SignalType::NETWORK_GET_TIME_DATE_FROM_NTP);
        }
        else
        {
            LOGW("Cannot handle NTP time update due to error flag: %02X", mFlagError);
        }
    }

    void Tasks::hanldeError()
    {
        if (mFlagError != 0x00)
        {
            ESP.restart();
        }
    }
} // namespace remoteLight