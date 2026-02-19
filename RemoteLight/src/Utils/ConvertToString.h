#ifndef CONVERT_TO_STRING_H
#define CONVERT_TO_STRING_H

#include "SignalType.h"
#include "Define.h"

namespace utils
{
    inline String SIGNALTOSTRING(const SignalType& signal)
    {
        switch (signal) {
            case SignalType::IR_BTN_UP_SIGNAL:                  return "IR_BTN_UP_SIGNAL";
            case SignalType::IR_BTN_DOWN_SIGNAL:                return "IR_BTN_DOWN_SIGNAL";
            case SignalType::IR_BTN_LEFT_SIGNAL:                return "IR_BTN_LEFT_SIGNAL";
            case SignalType::IR_BTN_RIGHT_SIGNAL:               return "IR_BTN_RIGHT_SIGNAL";
            case SignalType::IR_BTN_OK_SIGNAL:                  return "IR_BTN_OK_SIGNAL";
            case SignalType::IR_BTN_BACK_SIGNAL:                return "IR_BTN_BACK_SIGNAL";
            default:                                            return "UNKNOWN_SIGNAL";
        }
    }

    inline String CONTROLMODETOSTRING(const CONTROL_MODE& mode)
    {
        switch (mode) {
            case utils::CONTROL_MODE::CONNECT_WIFI:                    return "CONNECT_WIFI";
            case utils::CONTROL_MODE::CONNECT_WIFI_DONE:               return "CONNECT_WIFI_DONE";
            case utils::CONTROL_MODE::CONNECT_WIFI_FAILED:             return "CONNECT_WIFI_FAILED";
            case utils::CONTROL_MODE::CONNECT_FIREBASE:                return "CONNECT_FIREBASE";
            case utils::CONTROL_MODE::CONNECT_FIREBASE_DONE:           return "CONNECT_FIREBASE_DONE";
            case utils::CONTROL_MODE::CONNECT_FIREBASE_FAILED:         return "CONNECT_FIREBASE_FAILED";
            case utils::CONTROL_MODE::CONNECT_FIREBASE_GET_TOKEN:      return "CONNECT_FIREBASE_GET_TOKEN";
            case utils::CONTROL_MODE::CONNECT_NTP:                     return "CONNECT_NTP";
            case utils::CONTROL_MODE::CONNECT_NTP_DONE:                return "CONNECT_NTP_DONE";
            case utils::CONTROL_MODE::CONNECT_NTP_FAILED:              return "CONNECT_NTP_FAILED";
            case utils::CONTROL_MODE::DISPLAY_ALL:                     return "DISPLAY_ALL";
            case utils::CONTROL_MODE::SETTING_BUTTON_IR_REMOTE_START:  return "SETTING_BUTTON_IR_REMOTE_START";
            case utils::CONTROL_MODE::SETTING_BUTTON_IR_REMOTE:        return "SETTING_BUTTON_IR_REMOTE";
            case utils::CONTROL_MODE::SETTING_BUTTON_IR_REMOTE_DONE:   return "SETTING_BUTTON_IR_REMOTE_DONE";
            case utils::CONTROL_MODE::SETTING_RTC_TIME_START:          return "SETTING_RTC_TIME_START";
            case utils::CONTROL_MODE::SETTING_RTC_TIME:                return "SETTING_RTC_TIME";
            case utils::CONTROL_MODE::SETTING_RTC_TIME_DONE:           return "SETTING_RTC_TIME_DONE";
            case utils::CONTROL_MODE::SETTING_LIGHT_TIME_CONFIG:       return "SETTING_LIGHT_TIME_CONFIG";
            case utils::CONTROL_MODE::WIFI_PROVISIONING_START:         return "WIFI_PROVISIONING_START";
            case utils::CONTROL_MODE::WIFI_PROVISIONING_FAILED:        return "WIFI_PROVISIONING_FAILED";
            default:                                                   return "UNKNOWN_CONTROL_MODE";
        }
    }

    inline String STATETOSTRING(const STATE& state)
    {
        switch (state) {
            case STATE::IDLE:           return "IDLE";
            case STATE::RUNNING:        return "RUNNING";
            case STATE::PAUSED:         return "PAUSED";
            case STATE::WAITING:        return "WAITING";
            case STATE::DELAY:          return "DELAY";
            case STATE::DONE:           return "DONE";
            default:                    return "UNKNOWN_STATE";
        }
    }

    inline String INSERTLEVELTOSTRING(const INSERT_LEVEL& level)
    {
        switch (level) {
            case INSERT_LEVEL::LEVEL0:      return "LEVEL0";
            case INSERT_LEVEL::LEVEL1:      return "LEVEL1";
            case INSERT_LEVEL::LEVEL2:      return "LEVEL2";
            case INSERT_LEVEL::LEVEL3:      return "LEVEL3";
            case INSERT_LEVEL::LEVEL4:      return "LEVEL4";
            case INSERT_LEVEL::LEVEL5:      return "LEVEL5";
            case INSERT_LEVEL::LEVEL6:      return "LEVEL6";
            case INSERT_LEVEL::LEVEL7:      return "LEVEL7";
            case INSERT_LEVEL::LEVEL8:      return "LEVEL8";
            case INSERT_LEVEL::LEVEL9:      return "LEVEL9";
            case INSERT_LEVEL::LEVEL10:     return "LEVEL10";
            case INSERT_LEVEL::LEVEL11:     return "LEVEL11";
            default:                        return "UNKNOWN_INSERT_LEVEL";
        }
    }
} // namespace utils

#endif // CONVERT_TO_STRING_H