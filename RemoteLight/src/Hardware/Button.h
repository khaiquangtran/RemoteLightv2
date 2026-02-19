#ifndef BUTTON_H
#define BUTTON_H

#include "./HardwareBase.h"

namespace remoteLight {
    class RemoteLight;
}
namespace hardware
{
  using stateButton = std::pair<bool, bool>;
  constexpr uint32_t DEPAY = 50;
  class Button : public HardwareBase
  {
  public:
    Button(std::shared_ptr<remoteLight::RemoteLight> rml);
    virtual ~Button() = default;
    Button(const Button &) = delete;
    Button &operator=(const Button &) = delete;

    /**
     * @brief Listen for button presses and handle debouncing
     */
    void listenning() override;

    /**
     * @brief handle signal from other module
     * @param signal The signal to be handled
     * @param data Optional data associated with the signal
     */
    void handleSignal(const utils::SignalType &signal, const utils::Package *data = nullptr);

    /**
     * @brief Initialize button hardware
     */
    void init();

  private:
    struct ButtonInfo
    {
      unsigned long debounceTime = 0;
      bool prevRaw = HIGH;     // raw not debounce
      bool stableState = HIGH; // state after debounce

      unsigned long pressStart = 0;
      bool longPressed = false;

      utils::SignalType signal; // short-press signal
    };

    std::shared_ptr<remoteLight::RemoteLight> mRML;
    std::map<uint8_t, ButtonInfo> mListButton;

    /**
     * @brief Add a button to be monitored
     * @param pin The pin number where the button is connected
     * @param signal The signal to be sent when the button is pressed
     */
    void addButton(uint8_t pin, utils::SignalType signal);
  };
} // namespace hardware

#endif // BUTTON_H
