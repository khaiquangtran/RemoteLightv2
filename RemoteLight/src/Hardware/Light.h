#ifndef LIGHT_H
#define LIGHT_H

#include "./HardwareBase.h"

namespace remoteLight {
    class RemoteLight;
}


namespace hardware
{
  class Light : public HardwareBase
  {
  public:
    Light(std::shared_ptr<remoteLight::RemoteLight> rml);
    virtual ~Light() = default;
    Light(const Light &) = delete;
    Light &operator=(const Light &) = delete;

    /**
     * @brief Initialize light hardware
     */
    void handleSignal(const utils::SignalType &signal, const utils::Package *data = nullptr);

  private:
    std::shared_ptr<remoteLight::RemoteLight> mRML;

    std::map<uint8_t, bool> mListLight;
    std::map<utils::SignalType, uint8_t> mSignalLightMap;

    bool mIsBtnLedEnabled;

    /**
     * @brief Control the state of a light
     * @param light The light number to control
     * @param state The desired state (0: off, 1: on, 3: toggle)
     */
    void controlLight(uint8_t light, uint8_t state = 3);

    /**
     * @brief Control the button LED state
     * @param signal The signal indicating the desired button LED state
     */
    void controlButtonLED(const utils::SignalType &signal);

    /**
     * @brief Parse light state data received from EEPROM
     * @param data The data package containing light state data
     */
    void parseLightStateDataFromEEPROM(const utils::Package *data);

    void handleEnableButtonLED(const utils::Package *data);
  };
}

#endif // LIGHT_H