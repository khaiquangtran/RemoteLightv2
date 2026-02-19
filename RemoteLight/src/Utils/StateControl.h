#ifndef STATECONTROL_H
#define STATECONTROL_H

#include "Define.h"

namespace utils
{
    struct StateConfig
    {
        TIMER_SIGNAL signal;
        uint16_t duration;
        INSERT_LEVEL level;
    };

    class StateControl {
    public:
        explicit StateControl(CONTROL_MODE mode)
            : mMode(mode), mState(STATE::IDLE) {}

        /**
         * @brief Get the current control mode
         * @return The control mode
         */
        CONTROL_MODE getControlMode()  const { return mMode; }

        /**
         * @brief Get the current state
         * @return The state
         */
        STATE getState()        const { return mState; }

        /**
         * @brief Set the current state
         * @param s The new state
         */
        void setState(STATE s) { mState = s; }

        /**
         * @brief Get the configuration for the current control mode
         * @return The state configuration
         */
        StateConfig getConfig() const
        {
            switch (mMode)
            {
            case utils::CONTROL_MODE::CONNECT_WIFI:                        return {TIMER_SIGNAL::CONNECT_WIFI_SIGNAL,                       DELAY_3S, INSERT_LEVEL::LEVEL1};
            case utils::CONTROL_MODE::CONNECT_WIFI_DONE:                   return {TIMER_SIGNAL::CONNECT_WIFI_DONE,                         DELAY_3S, INSERT_LEVEL::LEVEL2};
            case utils::CONTROL_MODE::CONNECT_WIFI_FAILED:                 return {TIMER_SIGNAL::CONNECT_WIFI_FAILED_GOTO_NEXT_MODE,        DELAY_3S, INSERT_LEVEL::LEVEL2};
            case utils::CONTROL_MODE::CONNECT_FIREBASE:                    return {TIMER_SIGNAL::CONNECT_FIREBASE_SIGNAL,                   DELAY_3S, INSERT_LEVEL::LEVEL3};
            case utils::CONTROL_MODE::CONNECT_FIREBASE_DONE:               return {TIMER_SIGNAL::CONNECT_FIREBASE_DONE,                     DELAY_3S, INSERT_LEVEL::LEVEL4};
            case utils::CONTROL_MODE::CONNECT_FIREBASE_GET_TOKEN:          return {TIMER_SIGNAL::CONNECT_FIREBASE_GET_TOKEN,                DELAY_2S, INSERT_LEVEL::LEVEL4};
            case utils::CONTROL_MODE::CONNECT_FIREBASE_FAILED:             return {TIMER_SIGNAL::CONNECT_FIREBASE_FAILED_GOTO_NEXT_CONNECT, DELAY_3S, INSERT_LEVEL::LEVEL4};
            case utils::CONTROL_MODE::CONNECT_NTP:                         return {TIMER_SIGNAL::CONNECT_NTP_SIGNAL,                        DELAY_3S, INSERT_LEVEL::LEVEL5};
            case utils::CONTROL_MODE::CONNECT_NTP_DONE:                    return {TIMER_SIGNAL::CONNECT_NTP_DONE,                          DELAY_3S, INSERT_LEVEL::LEVEL6};
            case utils::CONTROL_MODE::CONNECT_NTP_FAILED:                  return {TIMER_SIGNAL::CONNECT_NTP_FAILED_GOTO_NEXT_CONNECT,      DELAY_3S, INSERT_LEVEL::LEVEL6};
            case utils::CONTROL_MODE::DISPLAY_ALL:                         return {TIMER_SIGNAL::DISPLAY_ALL_TIME_SIGNAL,                   DELAY_1S, INSERT_LEVEL::LEVEL8};
            case utils::CONTROL_MODE::SETTING_BUTTON_IR_REMOTE_START:      return {TIMER_SIGNAL::SETTING_BUTTON_IR_REMOTE_START,            DELAY_2S, INSERT_LEVEL::LEVEL7};
            case utils::CONTROL_MODE::SETTING_BUTTON_IR_REMOTE:            return {TIMER_SIGNAL::SETTING_BUTTON_IR_REMOTE,                  DELAY_1S, INSERT_LEVEL::LEVEL6};
            case utils::CONTROL_MODE::SETTING_BUTTON_IR_REMOTE_DONE:       return {TIMER_SIGNAL::SETTING_BUTTON_IR_REMOTE_DONE,             DELAY_2S, INSERT_LEVEL::LEVEL5};
            case utils::CONTROL_MODE::SETTING_RTC_TIME_START:              return {TIMER_SIGNAL::SETTING_RTC_TIME_START,                    DELAY_1S, INSERT_LEVEL::LEVEL7};
            case utils::CONTROL_MODE::SETTING_RTC_TIME:                    return {TIMER_SIGNAL::SETTING_RTC_TIME,                          DELAY_1S, INSERT_LEVEL::LEVEL6};
            case utils::CONTROL_MODE::SETTING_RTC_TIME_DONE:               return {TIMER_SIGNAL::SETTING_RTC_TIME_DONE,                     DELAY_1S, INSERT_LEVEL::LEVEL5};
            case utils::CONTROL_MODE::SETTING_LIGHT_TIME_CONFIG:           return {TIMER_SIGNAL::SETTING_LIGHT_TIME_CONFIG,                 DELAY_1S, INSERT_LEVEL::LEVEL6};
            case utils::CONTROL_MODE::SETTING_LIGHT_TIME_CONFIG_DONE:      return {TIMER_SIGNAL::SETTING_LIGHT_TIME_CONFIG_DONE,            DELAY_3S, INSERT_LEVEL::LEVEL5};
            case utils::CONTROL_MODE::WIFI_PROVISIONING_START:             return {TIMER_SIGNAL::WIFI_PROVISIONING_START,                   DELAY_1S, INSERT_LEVEL::LEVEL7};
            case utils::CONTROL_MODE::WIFI_PROVISIONING_FAILED:            return {TIMER_SIGNAL::WIFI_PROVISIONING_FAILED,                  DELAY_3S, INSERT_LEVEL::LEVEL6};
            default:                                                       return {TIMER_SIGNAL::TIMER_SIGNAL_MAX,                          DELAY_3S, INSERT_LEVEL::LEVELMAX};
            }
        }

    private:
        CONTROL_MODE mMode;
        STATE mState;
    };
} // namespace utils

#endif // STATECONTROL_H