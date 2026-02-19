#ifndef LCD16x2_H
#define LCD16x2_H

#include <LiquidCrystal_I2C.h>
#include "./HardwareBase.h"

namespace remoteLight {
    class RemoteLight;
}

namespace hardware
{
  class LCD16x2 : public HardwareBase
  {
  public:
    LCD16x2(std::shared_ptr<remoteLight::RemoteLight> rml);
    ~LCD16x2();
    LCD16x2(const LCD16x2 &) = delete;
    LCD16x2 &operator=(const LCD16x2 &) = delete;

    /**
     * @brief handle signal from other module
     * @param signal The signal to be handled
     * @param data Optional data associated with the signal
     */
    void handleSignal(const utils::SignalType &signal, const utils::Package *data = nullptr);

    /**
     * @brief Initialize LCD16x2 hardware
     */
    void init();

  private:
    std::shared_ptr<remoteLight::RemoteLight> mRML;
    LiquidCrystal_I2C *mLCD;
    char *DAY[8];
    struct TimeDS1307 mReceiverTime;
    uint8_t mRetry;
    std::map<uint8_t, String> mButtonStringMap;
    bool mFlagLCDLightOn;

    const uint8_t RETRY = 3U;
    const uint8_t LCD_ADDR = 0x27;

    /**
     * @brief Check if the LCD device is connected by scanning I2C addresses
     * @return true if the device is connected, false otherwise
     */
    bool checkAddress();

    /**
     * @brief Display time received from RTC module
     * @param data The data package containing time information
     */
    void displayTimeFromDS1307(const utils::Package *data);

    /**
     * @brief Display setting RTC time start mode
     */
    void displaySettingRTCTimeStart();

    /**
     * @brief Display setting RTC time end mode
     */
    void displaySettingRTCTimeEnd();

    /**
     * @brief Display menu mode for light configuration
     * @param light The light number to configure
     */
    void displayMenuMode(const uint8_t light);

    /**
     * @brief Display the selected menu mode based on input data
     * @param data The data containing the selected menu information
     */
    void displaySelectedLightTimeConfig(const utils::Package *data);

    /**
     * @brief Display start connecting to WiFi
     */
    void displayStartConnectWifi();

    /**
     * @brief Display connecting to WiFi
    */
    void displayConnecting();

    /**
     * @brief Display WiFi connected successfully
     */
    void displayConnectWifiSuccess();

    /**
     * @brief Display WiFi connection failed
     */
    void displayConnectWifiFailed();

    /**
     * @brief Display start connecting to Firebase
     */
    void displayStartConnectFirebase();

    /**
     * @brief Display Firebase connected successfully
     */
    void displayConnectFirebaseSuccess();

    /**
     * @brief Display Firebase connection failed
     */
    void displayConnectFirebaseFailed();

    /**
     * @brief Display start connecting to NTP
     */
    void displayStartConnectNTP();

    /**
     * @brief Display NTP connected successfully
     */
    void displayConnectNTPSuccess();

    /**
     * @brief Display NTP connection failed
     */
    void displayConnectNTPFailed();

    /**
     * @brief Display instructions for installing an IR button
     * @param data The data package containing installation information
     */
    void displayInstallButton(const utils::Package *data);

    /**
     * @brief Display the start of provisioning process
     * @param data The data package containing provisioning information
     */
    void displayStartProvisioning(const utils::Package *data);

    /**
     * @brief Display provisioning failed message
     */
    void displayProvisioningFailed();

    /**
     * @brief Display provisioning success message
     */
    void displayProvisioningSuccess();

    /**
     * @brief Display installation completion message for an IR button
     * @param data The data package containing installation information
     */
    void displayInstallButtonDone(const utils::Package *data);

    /**
     * @brief Display installation mode completion message for IR buttons
     */
    void displayInstallButtonModeDone();

    /**
     * @brief Display the start of IR button installation mode
     */
    void displayInstallButtonModeStart();

    /**
     * @brief Display on LCD is "Button 1"
     */
    void displayInstallButton1();

    void print2Digit(const int32_t& value);
  };
} // namespace hardware

#endif // LCD16x2_H