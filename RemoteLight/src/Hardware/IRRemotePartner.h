#ifndef IR_REMOTE_PARTNER_H
#define IR_REMOTE_PARTNER_H

#include "./HardwareBase.h"

namespace remoteLight {
    class RemoteLight;
}

namespace hardware
{
  class IRRemotePartner : public HardwareBase
  {
  public:
    IRRemotePartner(std::shared_ptr<remoteLight::RemoteLight> rml);
    virtual ~IRRemotePartner() = default;
    IRRemotePartner(const IRRemotePartner &) = delete;
    IRRemotePartner &operator=(const IRRemotePartner &) = delete;

    /**
     * @brief Listen for IR signals and handle them
     */
    void listenning();

    /**
     * @brief handle signal from other module
     * @param signal The signal to be handled
     * @param data Optional data associated with the signal
     */
    void handleSignal(const utils::SignalType &signal, const utils::Package *data = nullptr);

  private:
    std::shared_ptr<remoteLight::RemoteLight> mRML;
    const uint_fast8_t pinIR = 23U;
    int32_t NUMBER_BUTTONS;

    std::vector<utils::SignalType> mButtonSignal;
    std::map<int32_t, utils::SignalType> mButtonSignalMap;
    std::map<int32_t, utils::SignalType> mButtonSignalMapTemp;
    uint8_t mFlagInstallButton;
    int32_t mValueButton;
    uint8_t mNumberButton;

    /**
     * @brief Parse IR button data received from EEPROM
     * @param data The data package containing IR button data
     */
    void parseDataFromEEPROM(const utils::Package *data);

    /**
     * @brief Handle the installation mode for IR buttons
     */
    void handleInstallButtonMode();
  };
} // namespace hardware

#endif // IR_REMOTE_PARTNER_H