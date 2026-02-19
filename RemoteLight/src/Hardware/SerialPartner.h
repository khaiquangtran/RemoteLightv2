#ifndef SERIAL_PARTNER_H
#define SERIAL_PARTNER_H

#include <Arduino.h>
#include "./HardwareBase.h"

namespace remoteLight {
    class RemoteLight;
}

namespace hardware
{
    class SerialPartner : public HardwareBase
    {
    public:
        SerialPartner(std::shared_ptr<remoteLight::RemoteLight> rml);
        virtual ~SerialPartner() = default;
        SerialPartner(const SerialPartner &) = delete;
        SerialPartner &operator=(const SerialPartner &) = delete;

        /**
         * @brief handle signal from other module
         * @param signal The signal to be handled
         */
        void handleSignal(const utils::SignalType &signal, const utils::Package *data = nullptr);

        /**
         * @brief Listen for serial input and handle commands
         */
        void listenning() override;

    private:
        std::shared_ptr<remoteLight::RemoteLight> mRML;
        const int32_t BAUD_RATE = 115200;

        /**
         * @brief Handle a received serial message
         * @param receiverData The received message string
         */
        void handleMessage(String receiverData);
    };
} // namespace hardware
#endif // SERIAL_PARTNER_H