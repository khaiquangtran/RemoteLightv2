#include "./Button.h"
#include "./../RemoteLight.h"

namespace hardware
{
  Button::Button(std::shared_ptr<remoteLight::RemoteLight> rml) : mRML(rml)
  {
    LOGI("================== Button ==================");
  }

  void Button::init()
  {
#if NOT_CONNECT_DEVICE
    LOGW("Skip pinMode setup due to NOT_CONNECT_DEVICE is defined");
    return;
#endif
    addButton(BNT_1, utils::BTN_PRESS_BTN_1_SIGNAL);
    addButton(BNT_2, utils::BTN_PRESS_BTN_2_SIGNAL);
  }

  void Button::handleSignal(const utils::SignalType &signal, const utils::Package *data)
  {
    LOGI("Signal handled for button %d", signal);
    switch (signal)
    {
    case utils::BTN_PRESS_BTN_1_SIGNAL:
    case utils::BTN_PRESS_BTN_2_SIGNAL:
    {
      mRML->handleSignal(signal);
      break;
    }
    default:
    {
      LOGW("Button is not supported yet.");
      break;
    }
    }
  }

  void Button::addButton(uint8_t pin, utils::SignalType signal)
  {
    mListButton[pin] = ButtonInfo();
    mListButton[pin].signal = signal;
    pinMode(pin, INPUT_PULLUP);
  }

  void Button::listenning()
  {
    static unsigned long comboStartTime = 0;
    static bool comboActive = false;
    static bool comboHandled = false;

    bool btn1Pressed = (digitalRead(BNT_1) == LOW);
    bool btn2Pressed = (digitalRead(BNT_2) == LOW);

    // ==============================
    //        CHECK COMBO
    // ==============================
    if (btn1Pressed && btn2Pressed)
    {
      if (!comboActive)
      {
        comboActive = true;
        comboStartTime = millis();
        comboHandled = false;
        LOGD("Combo BTN1+BTN2 started");
      }

      if (!comboHandled && (millis() - comboStartTime >= DELAY_3S))
      {
        comboHandled = true;
        LOGD("BTN1 + BTN2 held for 3s -> trigger combo!");
        mRML->handleSignal(utils::SignalType::BTN_PRESS_BTN_1_2_COMBO_SIGNAL);
      }

      return; // Single button presses are not processed during a combo event.
    }
    else
    {
      comboActive = false;
      comboStartTime = 0;
      comboHandled = false;
    }

    // ==============================
    //      PROCESS NORMAL BUTTONS
    // ==============================
    for (auto &it : mListButton)
    {
      uint8_t pin = it.first;
      auto &info = it.second;

      bool raw = digitalRead(pin);

      // ---------- DEBOUNCE ----------
      if (raw != info.prevRaw)
        info.debounceTime = millis();

      if (millis() - info.debounceTime > DEPAY)
      {
        // STATE CHANGED
        if (raw != info.stableState)
        {
          info.stableState = raw;

          if (raw == LOW)
          {
            // BUTTON DOWN
            info.pressStart = millis();
            info.longPressed = false;

            LOGD("Button %d pressed", pin);
          }
          else
          {
            // BUTTON UP
            if (!info.longPressed)
            {
              LOGD("Button %d short press", pin);
              mRML->handleSignal(info.signal);
            }

            LOGD("Button %d released", pin);
          }
        }
      }

      // ---------- LONG PRESS 3s ----------
      if (info.stableState == LOW && !info.longPressed)
      {
        if (millis() - info.pressStart >= DELAY_3S)
        {
          info.longPressed = true;
          LOGD("Button %d long press 3s", pin);

          mRML->handleSignal(utils::SignalType::BTN_LONG_PRESS_SIGNAL);
        }
      }

      info.prevRaw = raw;
    }
  }
}