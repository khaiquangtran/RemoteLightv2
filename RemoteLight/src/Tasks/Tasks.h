#ifndef TASKS_H
#define TASKS_H

#include <mutex>
#include <memory>
#include "./../Utils/Logging.h"
#include "./../Utils/Package.h"
#include "./../Utils/SignalType.h"
#include "./../Utils/Define.h"
#include "./../Utils/StateControl.h"
#include "./../RemoteLight.h"
#include "./../Network/Network.h"
#include "./../Hardware/HardwareBase.h"

class RemoteLight;
class Network;
namespace hardware {
    class HardwareBase;
}
namespace remoteLight
{
    constexpr uint8_t REPEATS_5 = 5U;
    constexpr uint8_t REPEATS_10 = 10U;
    constexpr uint8_t REPEATS_30 = 30U;
    class Tasks
    {
    public:
        Tasks(std::shared_ptr<RemoteLight> rml,
                std::shared_ptr<hardware::HardwareBase> lcd,
                std::shared_ptr<hardware::HardwareBase> rtc,
                std::shared_ptr<hardware::HardwareBase> ir,
                std::shared_ptr<hardware::HardwareBase> light,
                std::shared_ptr<Network> net);
        ~Tasks() {};
        Tasks(const Tasks &) = delete;
        Tasks &operator=(const Tasks &) = delete;

        /**
         * @brief handle control mode
         * @param mode The control mode to be handled
         */
        void handleControlMode(const utils::CONTROL_MODE mode);

    private:
        enum class MODE_HANDLE : uint8_t
        {
            NONE = 0,
            INTO_SETTING_RTC,
            INTO_SETTING_TIME_CONFIG,
            SETTING_TIME_CONFIG,
            INSTALL_IR_BUTTON,
        };

        enum class ERROR_FLAG : uint8_t
        {
            NONE = 0,
            WIFI_ERROR,
            FIREBASE_ERROR,
            NTP_ERROR,
        };

        std::shared_ptr<RemoteLight> mRML;
        std::shared_ptr<hardware::HardwareBase> mLCD;
        std::shared_ptr<hardware::HardwareBase> mRTC;
        std::shared_ptr<hardware::HardwareBase> mIR;
        std::shared_ptr<hardware::HardwareBase> mLight;
        std::shared_ptr<Network> mNET;

        MODE_HANDLE mModeHandle;
        uint8_t mCounterConnect;
        uint8_t mCounterDisplayAllTime;
        uint8_t mFlagError;

        /**
         * @brief process connect wifi mode
         */
        void connectWifiMode();

        /**
         * @brief process connect firebase mode
         */
        void connectFirebaseMode();

        /**
         * @brief process display all time mode
         */
        void connectNTPMode();

        /**
         * @brief process display all time mode
         */
        void displayAllTime();

        /**
         * @brief process install IR button mode
         */
        void installIRButton();

        /**
         * @brief process install IR button mode start
         */
        void installIRButtonModeStart();

        /**
         * @brief handle NTP time update
         */
        void handleNTPTimeUpdate();

        /**
         * @brief handle error
         */
        void hanldeError();

        /**
         * @brief set error mode
         * @param error The error flag to be set
         */
        void setErrorMode(const ERROR_FLAG& error);
    };
} // namespace remoteLight
#endif // TASKS_H