#include <Arduino.h>
#include "./SerialPartner.h"
#include "./../RemoteLight.h"

namespace hardware
{
    SerialPartner::SerialPartner(std::shared_ptr<remoteLight::RemoteLight> rml) : mRML(rml)
    {
        Serial.begin(BAUD_RATE);
        LOGI(" ================== SerialPartner ================== ");
    }

    void SerialPartner::listenning()
    {
        if (Serial.available())
        {

            String receiverData = Serial.readStringUntil('\n');
            receiverData.trim();

            LOGI("%s", receiverData.c_str());

            if (receiverData.equalsIgnoreCase("help") || receiverData.equalsIgnoreCase("h"))
            {
                LOGI("Available commands:");
                LOGI("help or h - Show this help message");
                LOGI("1 - Send BTN_PRESS_BTN_1_SIGNAL");
                LOGI("2 - Send BTN_PRESS_BTN_2_SIGNAL");
                LOGI("3 - Send BTN_PRESS_BTN_1_2_COMBO_SIGNAL");
                LOGI("4 - Clean SSID and Password stored in EEPROM");
                LOGI("=====================================");
            }
            else if (receiverData == "1")
            {
                mRML->handleSignal(utils::SignalType::BTN_PRESS_BTN_1_SIGNAL);
            }
            else if (receiverData == "2")
            {
                mRML->handleSignal(utils::SignalType::BTN_PRESS_BTN_2_SIGNAL);
            }
            else if (receiverData == "3")
            {
                mRML->handleSignal(utils::SignalType::BTN_PRESS_BTN_1_2_COMBO_SIGNAL);
            }
            else if (receiverData == "4")
            {
                mRML->handleSignal(utils::SignalType::EEPROM_CLEAR_SSID_PASSOWRD_DATA);
            }
        }
    }

    void SerialPartner::handleSignal(const utils::SignalType &signal, const utils::Package *data)
    {
        switch (signal)
        {
        default:
            break;
        }
    }
} // namespace hardware