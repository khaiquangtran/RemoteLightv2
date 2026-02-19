#ifndef RTC_H
#define RTC_H

#include "./HardwareBase.h"
#include "./RTCDef.h"

namespace remoteLight {
    class RemoteLight;
}

namespace hardware
{
	// 				the time the light turns on - turns off
	using LightMapValue = std::pair<LightOfTime, LightOfTime>;

	class RTC : public HardwareBase
	{
	public:
		RTC(std::shared_ptr<remoteLight::RemoteLight> rml);
		virtual ~RTC() = default;
		RTC(const RTC &) = delete;
		RTC &operator=(const RTC &) = delete;

		/**
		 * @brief handle signal from other module
		 * @param signal The signal to be handled
		 * @param data Optional data associated with the signal
		 */
		void handleSignal(const utils::SignalType &signal, const utils::Package *data = nullptr);

		/**
		 * @brief Initialize RTC hardware
		 */
		void init();

	private:
		std::shared_ptr<remoteLight::RemoteLight> mRML;
		std::map<String, LightMapValue> mTimeOfLight;
		struct TimeDS1307 mAllTimeData;
		struct TimeDS1307 mAllTimeDataTemp;

		uint8_t mRTCAddr;
		TimeField mIndexOfAllTimeData;
		LightOfTimeField mIndexLight;
		uint8_t mIndexListLight;
		uint8_t mCountRetry;

		const String LISTLIGHT[4] = {"Light1", "Light2", "Light3", "Light4"};
		const uint8_t RTC_ADDRESS = 0x68;
		const uint8_t RETRY = 3U;
		const LightOfTime BUTTON_LED_ON_TIME{1U, 22U, 1U, 1U};
		const LightOfTime BUTTON_LED_OFF_TIME{1U, 6U, 1U, 2U};

		/**
		 * @brief Check if the RTC device is connected by scanning I2C addresses
		 * @return true if the device is connected, false otherwise
		 */
		bool checkAddress();

		/**
		 * @brief Convert BCD to Decimal
		 * @param val The BCD value to convert
		 * @return The converted Decimal value
		 */
		byte bcdToDec(byte val);

		/**
		 * @brief Convert Decimal to BCD
		 * @param val The Decimal value to convert
		 * @return The converted BCD value
		 */
		byte decToHex(byte val);

		/**
		 * @brief Read data from a specific register of the RTC
		 * @param reg The register address to read from
		 * @return The data read from the register
		 */
		bool writeData(uint8_t reg, uint8_t data);

		/**
		 * @brief Write data to a specific register of the RTC
		 * @param reg The register address to write to
		 * @return The data read from the register
		 */
		struct TimeDS1307 getTimeData();

		/**
		 * @brief Set time data to the RTC
		 * @param data The time data to set
		 * @return true if the operation was successful, false otherwise
		 */
		bool setTimeData(struct TimeDS1307 data);

		/**
		 * @brief Send all time data to LCD module
		 */
		void sendAllTimeDataToLCD();

		/**
		 * @brief Send all temp time data to LCD module
		 */
		void sendAllTimeTempDataToLCD();

		/**
		 * @brief Increase the value of the selected time data field
		 */
		void increaseValueOfTimeData();

		/**
		 * @brief Decrease the value of the selected time data field
		 */
		void decreaseValueOfTimeData();

		/**
		 * @brief Shift the index of the selected time data field
		 * @param adjust true to shift left, false to shift right
		 */
		void shiftIndexOfAllTimeData(bool adjust);

		/**
		 * @brief Increase the value of the selected menu mode
		 */
		void increaseValueOfMenuMode();

		/**
		 * @brief Decrease the value of the selected menu mode
		 */
		void decreaseValueOfMenuMode();

		/**
		 * @brief Send time configuration of lights to LCD module
		 */
		void sendTimeOfLightToLCD();

		/**
		 * @brief Check and configure time for lights based on current RTC time
		 */
		void checkConfigureTimeForLight();

		/**
		 * @brief Receive time and date data from NTP server
		 * @param data The data package containing time information
		 */
		void receiveTimeDateFromNTP(const utils::Package *data);

		/**
		 * @brief Parse time data received from EEPROM
		 * @param data The data package containing time data
		 */
		void getLightOnOffDataFromEEPROM(const utils::Package *data);

		/**
		 * @brief Increases or decreases a time value with specificed range
		 *
		 * This function increases or decreases the given value within the range [min, max]
		 * If inDec is true, the value is incremented. If the value exceeds max, it wraps around to min
		 * If inDec is false, the value is decremented. If the value goes below min, it wraps around to max
		 *
		 * @param value 	The current value to be incremented or decremented
		 * @param max		The maximum allowed value.
		 * @param min 		The minimum allowed value.
		 * @param incDec	If true, increment the value; if false, decrement the value
		 * @return			The updated value after increment/decrement and range check
		 */
		uint32_t incDecTimeData(uint32_t value, const uint32_t max, const uint32_t min, const bool incDec);

		/**
		 * @brief Hanldes the confimation action in the light setting configuration
		 */
		void handleOKIntoSettingConfig();

		/**
		 * @brief Calculate the number of minutes from the time of input
		 *
		 * @param time The input time for conversion
		 */
		uint16_t  convertTimeToMinute(const struct LightOfTime& time);

		/**
		 * @brief Calculate the number of minutes from the time of input
		 *
		 * @param time The input time for conversion
		 */
		uint16_t convertTimeToMinute(const struct TimeDS1307& time);

		/**
		 * @brief Check if button led is available
		 */
		void handleEnableButtonLED();

		 /**
		  * @brief Send the current time to the Network module
		  */
		void sendCurrentTimeToNetwork();

	};
} // namespace hardware

#endif // RTC_H