#include "IRRemotePartner.h"
#include "EEPROMPartner.h"
#include <IRremote.hpp>
#include "./../RemoteLight.h"

namespace hardware
{
  IRRemotePartner::IRRemotePartner(std::shared_ptr<remoteLight::RemoteLight> rml) : mRML(rml), mFlagInstallButton(0U), mValueButton(0), mNumberButton(0)
  {
#if NOT_CONNECT_DEVICE
    LOGW("IrReceiver.begin skipped. NOT_CONNECT_DEVICE is defined");
#else
    IrReceiver.begin(pinIR, ENABLE_LED_FEEDBACK);
#endif
    mButtonSignal = {
        utils::SignalType::IR_BTN_1_SIGNAL,
        utils::SignalType::IR_BTN_2_SIGNAL,
        utils::SignalType::IR_BTN_3_SIGNAL,
        utils::SignalType::IR_BTN_4_SIGNAL,
        utils::SignalType::IR_BTN_5_SIGNAL,
        utils::SignalType::IR_BTN_UP_SIGNAL,
        utils::SignalType::IR_BTN_DOWN_SIGNAL,
        utils::SignalType::IR_BTN_RIGHT_SIGNAL,
        utils::SignalType::IR_BTN_LEFT_SIGNAL,
        utils::SignalType::IR_BTN_OK_SIGNAL,
        utils::SignalType::IR_BTN_MENU_SIGNAL,
        utils::SignalType::IR_BTN_APP_SIGNAL,
        utils::SignalType::IR_BTN_BACK_SIGNAL,
    };

    NUMBER_BUTTONS = mButtonSignal.size();

    LOGI(" =========== IRRemotePartner =========== ");
  }

  void IRRemotePartner::handleSignal(const utils::SignalType &signal, const utils::Package *data)
  {
    switch (signal)
    {
    case utils::SignalType::IR_INSTALL_BUTTON:
    {
      mFlagInstallButton = 1;
      mNumberButton = 1;
      break;
    }
    case utils::SignalType::IR_INSTALL_BUTTON_DONE:
    {
      handleInstallButtonMode();
      break;
    }
    case (utils::SignalType::IR_ERRPROM_SEND_DATA):
    {
      parseDataFromEEPROM(data);
      break;
    }

    default:
      break;
    }
  }

  void IRRemotePartner::listenning()
  {
    if (IrReceiver.decode())
    {
      IrReceiver.resume();
      if (IrReceiver.decodedIRData.flags & IRDATA_FLAGS_IS_REPEAT)
      {
        return;
      }
      else
      {
        int32_t valueIR = static_cast<int32_t>(IrReceiver.decodedIRData.decodedRawData);
        if (valueIR == 0U)
        {
          return;
        }
        else
        {
          LOGD("What IR heard was %x", valueIR);
          if (mFlagInstallButton == 0)
          {
            if (mButtonSignalMap.find(valueIR) != mButtonSignalMap.end())
            {
              mRML->handleSignal(mButtonSignalMap[valueIR]);
            }
            else
            {
              LOGW("Data not found!");
            }
          }
          else if (mFlagInstallButton == 1)
          {
            if (mNumberButton > NUMBER_BUTTONS)
            {
              mFlagInstallButton = 0;
              mNumberButton = 0;
              mRML->handleSignal(utils::SignalType::IR_INSTALL_BUTTON_COMPLETE);
              mButtonSignalMap.clear();
              mButtonSignalMap = std::move(mButtonSignalMapTemp);
              return;
            }
            mValueButton = valueIR;
            uint8_t numberButton = mNumberButton - 1;
            std::vector<int32_t> vecData = {numberButton, mValueButton};
            std::unique_ptr<utils::Package> packData = std::make_unique<utils::Package>(vecData);
            mRML->handleSignal(utils::SignalType::LCD_DISPLAY_IRBUTTON_INSTALL, packData.get());
          }
          else
          {
            /*Do nothing*/
          }
        }
      }
    }
  }

  void IRRemotePartner::parseDataFromEEPROM(const utils::Package *data)
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
      if (size != NUMBER_BUTTONS)
      {
        LOGE("Data from EEPROM with length is not %.", NUMBER_BUTTONS);
        return;
      }
      else
      {
        for (int32_t i = 0; i < size; i++)
        {
          mButtonSignalMap[value[i]] = mButtonSignal[i];
          LOGD("data: %x, order: %d", value[i], i);
        }
      }
    }
  }

  void IRRemotePartner::handleInstallButtonMode()
  {
    if (mNumberButton <= NUMBER_BUTTONS)
    {
      LOGD("Button %d : %x", mNumberButton, mValueButton);
      // After the button installation is complete, display on LCD the done message
      mButtonSignalMapTemp[mValueButton] = mButtonSignal[mNumberButton - 1];
      std::vector<int32_t> vecData = {mNumberButton};
      std::unique_ptr<utils::Package> packData = std::make_unique<utils::Package>(vecData);
      mRML->handleSignal(utils::SignalType::LCD_LIGHT_IRBUTTON_INSTALL_DONE, packData.get());

      // After the button installation is complete, store the data in EEPROM
      uint8_t numberButton = mNumberButton - 1;
      std::vector<int32_t> vecData2 = {numberButton, mValueButton};
      std::unique_ptr<utils::Package> packData2 = std::make_unique<utils::Package>(vecData2);
      mRML->handleSignal(utils::SignalType::EEPROM_STORE_IRBUTTON_INSTALL, packData2.get());
      mNumberButton++;
    }
  }
} // namespace hardware