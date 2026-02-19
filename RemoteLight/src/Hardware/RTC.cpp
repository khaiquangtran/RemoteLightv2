#include "RTC.h"
#include "./../RemoteLight.h"

namespace hardware
{
	RTC::RTC(std::shared_ptr<remoteLight::RemoteLight> rml) : mRML(rml), mRTCAddr(0U), mIndexOfAllTimeData(TimeField::Second), mIndexLight(LightOfTimeField::swOn),
												 mIndexListLight(0U), mCountRetry(0U)
	{
		mTimeOfLight["Light1"] = std::make_pair(LightOfTime{0U, 0U, 0U, 0U}, LightOfTime{0U, 0U, 0U, 0U});
		mTimeOfLight["Light2"] = std::make_pair(LightOfTime{0U, 0U, 0U, 0U}, LightOfTime{0U, 0U, 0U, 0U});
		mTimeOfLight["Light3"] = std::make_pair(LightOfTime{0U, 0U, 0U, 0U}, LightOfTime{0U, 0U, 0U, 0U});
		mTimeOfLight["Light4"] = std::make_pair(LightOfTime{0U, 0U, 0U, 0U}, LightOfTime{0U, 0U, 0U, 0U});

		mAllTimeData = {0U, 0U, 0U, 0U, 0U, 0U, 0U};

		LOGI(" =========== RTC ===========");
	}

	void RTC::init()
	{
#if NOT_CONNECT_DEVICE
		LOGW("Skip init RTC due to NOT_CONNECT_DEVICE is defined");
		return;
#endif

#ifndef INIT_I2C
#define INIT_I2C
		Wire.begin();
#endif
	retry:
		if (checkAddress())
		{
			mAllTimeData = getTimeData();
		}
		else
		{
			if (mCountRetry < RETRY)
			{
				mCountRetry++;
				LOGW("retry %d", mCountRetry);
				goto retry;
			}
		}
		LOGI("Initialization RTC!");
	}

	void RTC::handleSignal(const utils::SignalType &signal, const utils::Package *data)
	{
		if (mCountRetry >= RETRY)
		{
			LOGE("Can't connect to RTC");
			return;
		}
		else
		{
			// LOGD("Handle signal value: %d", signal);
			switch (signal)
			{
			case utils::SignalType::RTC_DISPLAY_ALL_TIME:
			{
				mAllTimeData = getTimeData();
				sendAllTimeDataToLCD();
				break;
			}
			case utils::SignalType::RTC_SETTING_RTC_INCREASE_VALUE:
			{
				increaseValueOfTimeData();
				sendAllTimeTempDataToLCD();
				break;
			}
			case utils::SignalType::RTC_SETTING_RTC_DECREASE_VALUE:
			{
				decreaseValueOfTimeData();
				sendAllTimeTempDataToLCD();
				break;
			}
			case utils::SignalType::RTC_SETTING_RTC_SHIFT_LEFT_VALUE:
			{
				shiftIndexOfAllTimeData(true);
				break;
			}
			case utils::SignalType::RTC_SETTING_RTC_SHIFT_RIGHT_VALUE:
			{
				shiftIndexOfAllTimeData(false);
				break;
			}
			case utils::SignalType::RTC_SETTING_RTC_TIME_OK:
			{
				setTimeData(mAllTimeDataTemp);
				break;
			}
			case utils::SignalType::RTC_SETTING_RTC_TIME_START:
			{
				mAllTimeDataTemp = mAllTimeData;
				sendAllTimeTempDataToLCD();
				break;
			}
			case utils::SignalType::RTC_MOVE_LEFT_SETTING_CONFIG:
			{
				if (mIndexListLight > 0U)
				{
					mIndexListLight--;
				}
				else if (mIndexListLight == 0U)
				{
					mIndexListLight = 3U;
				}
				std::vector<int32_t> vecData = {static_cast<int32_t>(mIndexListLight)};
				std::unique_ptr<utils::Package> packData = std::make_unique<utils::Package>(vecData);
				mRML->handleSignal(utils::SignalType::LCD_MOVE_LEFT_MENU_MODE, packData.get());
				break;
			}
			case utils::SignalType::RTC_MOVE_RIGHT_SETTING_CONFIG:
			{
				if (mIndexListLight < 3U)
				{
					mIndexListLight++;
				}
				else if (mIndexListLight >= 3U)
				{
					mIndexListLight = 0U;
				}
				std::vector<int32_t> vecData = {static_cast<int32_t>(mIndexListLight)};
				std::unique_ptr<utils::Package> packData = std::make_unique<utils::Package>(vecData);
				mRML->handleSignal(utils::SignalType::LCD_MOVE_RIGHT_MENU_MODE, packData.get());
				break;
			}
			case utils::SignalType::RTC_OK_SETTING_CONFIG:
			{
				sendTimeOfLightToLCD();
				break;
			}
			case utils::SignalType::RTC_INCREASE_VALUE_SETTING_LIGHT_CONFIG:
			{
				increaseValueOfMenuMode();
				sendTimeOfLightToLCD();
				break;
			}
			case utils::SignalType::RTC_DECREASE_VALUE_SETTING_LIGHT_CONFIG:
			{
				decreaseValueOfMenuMode();
				sendTimeOfLightToLCD();
				break;
			}
			case utils::SignalType::RTC_MOVE_RIGHT_INTO_SETTING_CONFIG:
			{
				++mIndexLight;
				break;
			}
			case utils::SignalType::RTC_MOVE_LEFT_INTO_SETTING_CONFIG:
			{
				--mIndexLight;
				break;
			}
			case utils::SignalType::RTC_OK_INTO_SETTING_CONFIG:
			{;
				handleOKIntoSettingConfig();
				break;
			}
			case utils::SignalType::RTC_CHECK_CONFIGURED_TIME_FOR_LIGHT:
			{
				checkConfigureTimeForLight();
				break;
			}
			case utils::SignalType::NETWORK_SEND_TIME_DATE_FROM_NTP:
			{
				receiveTimeDateFromNTP(data);
				break;
			}
			case utils::SignalType::RTC_GET_ALL_TIME_DATA:
			{
				mAllTimeData = getTimeData();
				break;
			}
			case utils::SignalType::RTC_GET_LIGHT_ON_OFF_DATA:
			{
				getLightOnOffDataFromEEPROM(data);
				break;
			}
			case utils::SignalType::RTC_CHECK_ENABLE_BUTTON_LED:
			{
				handleEnableButtonLED();
				break;
			}
			case utils::SignalType::RTC_HANDLE_ALL_TIME_DATA:
			{
				sendCurrentTimeToNetwork();
				break;
			}
			default:
			{
				LOGW("Signal is not supported yet.");
				break;
			}
			}
		}
	}

	bool RTC::checkAddress()
	{
		if (scanAddress(RTC_ADDRESS) == HardwareBase::INVALID)
		{
			LOGI("No find RTC");
			return false;
		}
		else
		{
			mRTCAddr = RTC_ADDRESS;
			LOGI("Find out RTC");
			return true;
		}
	}

	byte RTC::bcdToDec(byte val)
	{
		return ((val / 16 * 10) + (val % 16));
	};

	uint8_t RTC::decToHex(uint8_t val)
	{
		return ((val / 10 * 16) + (val % 10));
	};

	struct TimeDS1307 RTC::getTimeData()
	{
		static uint8_t sec = 0;
		struct TimeDS1307 data{0U, 0U, 0U, 0U, 0U, 0U, 0U};
#if NOT_CONNECT_DEVICE
		LOGW("getTimeData is dummy data due to NOT_CONNECT_DEVICE is defined");
		data.second = sec++;
		data.minute = 0U;
		data.hour = 12U;
		data.day = 3U;
		data.date = 25U;
		data.month = 8U;
		data.year = 2025U;
		return data;
#endif
		if (mRTCAddr == 0)
		{
			return data;
		}
		else
		{
			Wire.beginTransmission(mRTCAddr);
			Wire.write(0x00);
			Wire.endTransmission();
			if (Wire.requestFrom(mRTCAddr, 7U) == 7U)
			{
				data.second = bcdToDec(Wire.read() & 0x7f);
				data.minute = bcdToDec(Wire.read());
				data.hour 	= bcdToDec(Wire.read() & 0x3f);
				data.day 	= bcdToDec(Wire.read());
				data.date 	= bcdToDec(Wire.read());
				data.month 	= bcdToDec(Wire.read());
				data.year 	= bcdToDec(Wire.read()) + 2000;
				// LOGI("%d/%d/%d %d %d:%d:%d", data.date, data.month, data.year, data.day, data.hour, data.minute, data.second);
				return data;
			}
			else
			{
				LOGE("Failed to get RTC date and time");
				return data;
			};
		}
	}

	bool RTC::writeData(uint8_t reg, uint8_t data)
	{
#if NOT_CONNECT_DEVICE
		LOGW("writeData is skipped due to NOT_CONNECT_DEVICE is defined");
		return true;
#endif
		bool result = false;
		if (mRTCAddr == 0)
		{
			LOGE("Address ds1307 is invalid!!!");
		}
		else
		{
			Wire.beginTransmission(mRTCAddr);
			Wire.write(reg);
			Wire.write(data);
			if (Wire.endTransmission() == 0)
			{
				LOGD("Set data successfully!");
				result = true;
			}
			else
			{
				LOGW("Set data failed!");
				result = false;
			};
		}
		return result;
	}

	bool RTC::setTimeData(struct TimeDS1307 data)
	{
#if NOT_CONNECT_DEVICE
		LOGW("setTimeData is skipped due to NOT_CONNECT_DEVICE is defined");
		return true;
#endif
		struct
		{
			uint8_t value;
			uint8_t reg;
			uint8_t min;
			uint8_t max;
			const char *name;
		} fields[] = {
			{data.second, 	REG_SEC, 	0, 59, 	"SECOND"},
			{data.minute, 	REG_MIN, 	0, 59, 	"MINUTE"},
			{data.hour, 	REG_HOUR, 	0, 23, 	"HOUR"},
			{data.day, 		REG_DAY, 	1, 7, 	"DAY"},
			{data.date, 	REG_DATE, 	1, 31, 	"DATE"},
			{data.month, 	REG_MTH, 	1, 12, 	"MONTH"},
			{static_cast<uint8_t>(data.year - 2000), REG_YEAR, 0, 99, "YEAR"} // Save as year - 2000
		};
		for (const auto &field : fields)
		{
			if (field.value < field.min || field.value > field.max)
			{
				LOGW("%s data is invalid!!!", field.name);
				return false;
			}
			LOGI("%s: %d", field.name, static_cast<int32_t>(field.value));
			uint8_t valueToWrite = decToHex(field.value);
			if (!writeData(field.reg, valueToWrite))
			{
				return false;
			}
		}
		return true;
	}

	void RTC::sendAllTimeDataToLCD()
	{
		std::vector<int32_t> vecData = {
			static_cast<int32_t>(mAllTimeData.second),
			static_cast<int32_t>(mAllTimeData.minute),
			static_cast<int32_t>(mAllTimeData.hour),
			static_cast<int32_t>(mAllTimeData.day),
			static_cast<int32_t>(mAllTimeData.date),
			static_cast<int32_t>(mAllTimeData.month),
			static_cast<int32_t>(mAllTimeData.year)};
		std::unique_ptr<utils::Package> package = std::make_unique<utils::Package>(vecData);
		LOGI("%d %d/%d/%d", vecData[3], vecData[4], vecData[5], vecData[6]);
		LOGI("%d:%d:%d", vecData[0], vecData[1], vecData[2]);
		mRML->handleSignal(utils::SignalType::LCD_DISPLAY_ALL_TIME, package.get());
	}

	void RTC::sendAllTimeTempDataToLCD()
	{
		std::vector<int32_t> vecData = {
			static_cast<int32_t>(mAllTimeDataTemp.second),
			static_cast<int32_t>(mAllTimeDataTemp.minute),
			static_cast<int32_t>(mAllTimeDataTemp.hour),
			static_cast<int32_t>(mAllTimeDataTemp.day),
			static_cast<int32_t>(mAllTimeDataTemp.date),
			static_cast<int32_t>(mAllTimeDataTemp.month),
			static_cast<int32_t>(mAllTimeDataTemp.year)};
		std::unique_ptr<utils::Package> package = std::make_unique<utils::Package>(vecData);
		LOGI("%d %d/%d/%d", vecData[3], vecData[4], vecData[5], vecData[6]);
		LOGI("%d:%d:%d", vecData[0], vecData[1], vecData[2]);
		mRML->handleSignal(utils::SignalType::LCD_DISPLAY_ALL_TIME, package.get());
	}

	void RTC::increaseValueOfTimeData()
	{
		switch (mIndexOfAllTimeData)
		{
		case TimeField::Second:
			mAllTimeDataTemp.second = incDecTimeData(mAllTimeDataTemp.second, MAX_SECOND, MIN_SECOND, INCREASE);
			break;
		case TimeField::Minute:
			mAllTimeDataTemp.minute = incDecTimeData(mAllTimeDataTemp.minute, MAX_MINUTE, MIN_MINUTE, INCREASE);
			break;
		case TimeField::Hour:
			mAllTimeDataTemp.hour = incDecTimeData(mAllTimeDataTemp.hour, MAX_HOUR, MIN_HOUR, INCREASE);
			break;
		case TimeField::Year:
			mAllTimeDataTemp.year = incDecTimeData(mAllTimeDataTemp.year, MAX_YEAR, MIN_YEAR, INCREASE);
			break;
		case TimeField::Month:
			mAllTimeDataTemp.month = incDecTimeData(mAllTimeDataTemp.month, MAX_MONTH, MIN_MONTH, INCREASE);
			break;
		case TimeField::Date:
			mAllTimeDataTemp.date = incDecTimeData(mAllTimeDataTemp.date, MAX_DATE, MIN_DATE, INCREASE);
			break;
		case TimeField::Day:
			mAllTimeDataTemp.day = incDecTimeData(mAllTimeDataTemp.day, MAX_DAY, MIN_DAY, INCREASE);
			break;
		default:
			break;
		}
	}

	void RTC::decreaseValueOfTimeData()
	{
		switch (mIndexOfAllTimeData)
		{
		case TimeField::Second:
			mAllTimeDataTemp.second = incDecTimeData(mAllTimeDataTemp.second, MAX_SECOND, MIN_SECOND, DECREASE);
			break;
		case TimeField::Minute:
			mAllTimeDataTemp.minute = incDecTimeData(mAllTimeDataTemp.minute, MAX_MINUTE, MIN_MINUTE, DECREASE);
			break;
		case TimeField::Hour:
			mAllTimeDataTemp.hour = incDecTimeData(mAllTimeDataTemp.hour, MAX_HOUR, MIN_HOUR, DECREASE);
			break;
		case TimeField::Year:
			mAllTimeDataTemp.year = incDecTimeData(mAllTimeDataTemp.year, MAX_YEAR, MIN_YEAR, DECREASE);
			break;
		case TimeField::Month:
			mAllTimeDataTemp.month = incDecTimeData(mAllTimeDataTemp.month, MAX_MONTH, MIN_MONTH, DECREASE);
			break;
		case TimeField::Date:
			mAllTimeDataTemp.date = incDecTimeData(mAllTimeDataTemp.date, MAX_DATE, MIN_DATE, DECREASE);
			break;
		case TimeField::Day:
			mAllTimeDataTemp.day = incDecTimeData(mAllTimeDataTemp.day, MAX_DAY, MIN_DAY, DECREASE);
			break;
		default:
			break;
		}
	}

	void RTC::shiftIndexOfAllTimeData(bool adjust)
	{
		if (adjust)
		{
			++mIndexOfAllTimeData;
		}
		else
		{
			--mIndexOfAllTimeData;
		}
		LOGI("mIndexOfAllTimeData %d:",static_cast<int>(mIndexOfAllTimeData));
	}

	void RTC::increaseValueOfMenuMode()
	{
		switch (mIndexLight)
		{
		case LightOfTimeField::swOn:
			if (mTimeOfLight[LISTLIGHT[mIndexListLight]].first.sw == 0U)
			{
				mTimeOfLight[LISTLIGHT[mIndexListLight]].first.sw = 1U;
			}
			else if (mTimeOfLight[LISTLIGHT[mIndexListLight]].first.sw)
			{
				mTimeOfLight[LISTLIGHT[mIndexListLight]].first.sw = 0U;
			}
			break;
		case LightOfTimeField::hourOn:
			mTimeOfLight[LISTLIGHT[mIndexListLight]].first.hour = incDecTimeData(mTimeOfLight[LISTLIGHT[mIndexListLight]].first.hour, MAX_HOUR, MIN_HOUR, INCREASE);
			break;
		case LightOfTimeField::minuteOn:
			mTimeOfLight[LISTLIGHT[mIndexListLight]].first.minute = incDecTimeData(mTimeOfLight[LISTLIGHT[mIndexListLight]].first.minute, MAX_MINUTE, MIN_MINUTE, INCREASE);
			break;
		case LightOfTimeField::secondOn:
			mTimeOfLight[LISTLIGHT[mIndexListLight]].first.second = incDecTimeData(mTimeOfLight[LISTLIGHT[mIndexListLight]].first.second, MAX_SECOND, MIN_SECOND, INCREASE);
			break;
		case LightOfTimeField::swOff:
			if (mTimeOfLight[LISTLIGHT[mIndexListLight]].second.sw == 0U)
			{
				mTimeOfLight[LISTLIGHT[mIndexListLight]].second.sw = 1U;
			}
			else if (mTimeOfLight[LISTLIGHT[mIndexListLight]].second.sw)
			{
				mTimeOfLight[LISTLIGHT[mIndexListLight]].second.sw = 0U;
			}
			break;
		case LightOfTimeField::hourOff:
			mTimeOfLight[LISTLIGHT[mIndexListLight]].second.hour = incDecTimeData(mTimeOfLight[LISTLIGHT[mIndexListLight]].second.hour, MAX_HOUR, MIN_HOUR, INCREASE);
			break;
		case LightOfTimeField::minuteOff:
			mTimeOfLight[LISTLIGHT[mIndexListLight]].second.minute = incDecTimeData(mTimeOfLight[LISTLIGHT[mIndexListLight]].second.minute, MAX_MINUTE, MIN_MINUTE, INCREASE);
			break;
		case LightOfTimeField::secondOff:
			mTimeOfLight[LISTLIGHT[mIndexListLight]].second.second = incDecTimeData(mTimeOfLight[LISTLIGHT[mIndexListLight]].second.second, MAX_SECOND, MIN_SECOND, INCREASE);
			break;
		default:
			break;
		}
	}

	void RTC::decreaseValueOfMenuMode()
	{
		switch (mIndexLight)
		{
		case LightOfTimeField::swOn:
			if (mTimeOfLight[LISTLIGHT[mIndexListLight]].first.sw == 0U)
			{
				mTimeOfLight[LISTLIGHT[mIndexListLight]].first.sw = 1U;
			}
			else if (mTimeOfLight[LISTLIGHT[mIndexListLight]].first.sw)
			{
				mTimeOfLight[LISTLIGHT[mIndexListLight]].first.sw = 0U;
			}
			break;
		case LightOfTimeField::hourOn:
			mTimeOfLight[LISTLIGHT[mIndexListLight]].first.hour = incDecTimeData(mTimeOfLight[LISTLIGHT[mIndexListLight]].first.hour, MAX_HOUR, MIN_HOUR, DECREASE);
			break;
		case LightOfTimeField::minuteOn:
			mTimeOfLight[LISTLIGHT[mIndexListLight]].first.minute = incDecTimeData(mTimeOfLight[LISTLIGHT[mIndexListLight]].first.minute, MAX_MINUTE, MIN_MINUTE, DECREASE);
			break;
		case LightOfTimeField::secondOn:
			mTimeOfLight[LISTLIGHT[mIndexListLight]].first.second = incDecTimeData(mTimeOfLight[LISTLIGHT[mIndexListLight]].first.second, MAX_SECOND, MIN_SECOND, DECREASE);
			break;
		case LightOfTimeField::swOff:
			if (mTimeOfLight[LISTLIGHT[mIndexListLight]].second.sw == 0U)
			{
				mTimeOfLight[LISTLIGHT[mIndexListLight]].second.sw = 1U;
			}
			else if (mTimeOfLight[LISTLIGHT[mIndexListLight]].second.sw)
			{
				mTimeOfLight[LISTLIGHT[mIndexListLight]].second.sw = 0U;
			}
			break;
		case LightOfTimeField::hourOff:
			mTimeOfLight[LISTLIGHT[mIndexListLight]].second.hour = incDecTimeData(mTimeOfLight[LISTLIGHT[mIndexListLight]].second.hour, MAX_HOUR, MIN_HOUR, DECREASE);
			break;
		case LightOfTimeField::minuteOff:
			mTimeOfLight[LISTLIGHT[mIndexListLight]].second.minute = incDecTimeData(mTimeOfLight[LISTLIGHT[mIndexListLight]].second.minute, MAX_MINUTE, MIN_MINUTE, DECREASE);
			break;
		case LightOfTimeField::secondOff:
			mTimeOfLight[LISTLIGHT[mIndexListLight]].second.second = incDecTimeData(mTimeOfLight[LISTLIGHT[mIndexListLight]].second.second, MAX_SECOND, MIN_SECOND, DECREASE);
			break;
		default:
			break;
		}
	}

	void RTC::sendTimeOfLightToLCD()
	{
		const auto &time = mTimeOfLight[LISTLIGHT[mIndexListLight]];
		std::vector<int32_t> vecData = {
			static_cast<int32_t>(mIndexListLight),
			static_cast<int32_t>(time.first.sw),
			static_cast<int32_t>(time.first.hour),
			static_cast<int32_t>(time.first.minute),
			static_cast<int32_t>(time.first.second),
			static_cast<int32_t>(time.second.sw),
			static_cast<int32_t>(time.second.hour),
			static_cast<int32_t>(time.second.minute),
			static_cast<int32_t>(time.second.second)};
		std::unique_ptr<utils::Package> packData = std::make_unique<utils::Package>(vecData);
		mRML->handleSignal(utils::SignalType::LCD_MENU_MODE_OK, packData.get());
	}

	void RTC::checkConfigureTimeForLight()
	{
		mAllTimeData = getTimeData();

		std::map<String, LightMapValue>::iterator it;
		for (it = mTimeOfLight.begin(); it != mTimeOfLight.end(); it++)
		{
			if (it->second.first.sw != 0 &&
				it->second.first.hour == mAllTimeData.hour &&
				it->second.first.minute == mAllTimeData.minute &&
				it->second.first.second == mAllTimeData.second)
			{
				if (it->first == "Light1")
				{
					mRML->handleSignal(utils::SignalType::RTC_TURN_ON_LIGHT1);
				}
				else if (it->first == "Light2")
				{
					mRML->handleSignal(utils::SignalType::RTC_TURN_ON_LIGHT2);
				}
				else if (it->first == "Light3")
				{
					mRML->handleSignal(utils::SignalType::RTC_TURN_ON_LIGHT3);
				}
				else if (it->first == "Light4")
				{
					mRML->handleSignal(utils::SignalType::RTC_TURN_ON_LIGHT4);
				}
				else
				{
					/*Do nothing*/
				}
			}
			if (it->second.second.sw != 0 &&
				it->second.second.hour == mAllTimeData.hour &&
				it->second.second.minute == mAllTimeData.minute &&
				it->second.second.second == mAllTimeData.second)
			{
				if (it->first == "Light1")
				{
					mRML->handleSignal(utils::SignalType::RTC_TURN_OFF_LIGHT1);
				}
				else if (it->first == "Light2")
				{
					mRML->handleSignal(utils::SignalType::RTC_TURN_OFF_LIGHT2);
				}
				else if (it->first == "Light3")
				{
					mRML->handleSignal(utils::SignalType::RTC_TURN_OFF_LIGHT3);
				}
				else if (it->first == "Light4")
				{
					mRML->handleSignal(utils::SignalType::RTC_TURN_OFF_LIGHT4);
				}
				else
				{
					/*Do nothing*/
				}
			}
		}

		if (mAllTimeData.hour == BUTTON_LED_ON_TIME.hour &&
			mAllTimeData.minute == BUTTON_LED_ON_TIME.minute &&
			mAllTimeData.second == BUTTON_LED_ON_TIME.second)
		{
			std::vector<int32_t> vecData{1U};
			std::unique_ptr<utils::Package> packData = std::make_unique<utils::Package>(vecData);
			mRML->handleSignal(utils::SignalType::LIGHT_IS_ENABLE_BUTTON_LED, packData.get());
			mRML->handleSignal(utils::SignalType::EEPROM_STORE_LIGHT_IS_ENABLE_BUTTON_LED, packData.get());
		}
		else if (mAllTimeData.hour == BUTTON_LED_OFF_TIME.hour &&
				 mAllTimeData.minute == BUTTON_LED_OFF_TIME.minute &&
				 mAllTimeData.second == BUTTON_LED_OFF_TIME.second)
		{
			std::vector<int32_t> vecData{0U};
			std::unique_ptr<utils::Package> packData = std::make_unique<utils::Package>(vecData);
			mRML->handleSignal(utils::SignalType::LIGHT_IS_ENABLE_BUTTON_LED, packData.get());
			mRML->handleSignal(utils::SignalType::EEPROM_STORE_LIGHT_IS_ENABLE_BUTTON_LED, packData.get());
		}
		else
		{
			// Do nothing
		}
	}

	void RTC::receiveTimeDateFromNTP(const utils::Package *data)
	{
		if (data->getSize() == 7)
		{
			const int32_t *parseData = data->getPackage();
			int32_t checkSum = 0U;
			for (int32_t i = 1; i < 7; i++)
			{
				checkSum += parseData[i];
			}
			if (checkSum <= 0U)
			{
				LOGE("Data from NTP is invalid! Double check connecting!");
			}
			else
			{
				// Format:        hour minute second day date month year
				mAllTimeData.hour 		= static_cast<uint8_t>(parseData[0]);
				mAllTimeData.minute 	= static_cast<uint8_t>(parseData[1]);
				mAllTimeData.second 	= static_cast<uint8_t>(parseData[2]);
				mAllTimeData.day 		= static_cast<uint8_t>(parseData[3]);
				mAllTimeData.date 		= static_cast<uint8_t>(parseData[4]);
				mAllTimeData.month 		= static_cast<uint8_t>(parseData[5]);
				mAllTimeData.year 		= static_cast<uint16_t>(parseData[6]);
				setTimeData(mAllTimeData);
			}
		}
		else
		{
			LOGE("Lenght is invalid!!!");
		}
	}

	void RTC::getLightOnOffDataFromEEPROM(const utils::Package *data)
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
			if (size == 0)
			{
				LOGE("Data from EEPROM is invalid. Size %d", size);
				return;
			}
			else
			{
				std::map<String, LightMapValue>::iterator it;
				int8_t i = 0;
				for (it = mTimeOfLight.begin(); it != mTimeOfLight.end(); it++)
				{
					it->second.first.sw 		= value[i++];
					it->second.first.hour 		= value[i++];
					it->second.first.minute 	= value[i++];
					it->second.first.second 	= value[i++];
					it->second.second.sw 		= value[i++];
					it->second.second.hour 		= value[i++];
					it->second.second.minute 	= value[i++];
					it->second.second.second 	= value[i++];

					LOGD("%s TIME ON: %d-%d:%d:%d", it->first,
						 it->second.first.sw,
						 it->second.first.hour,
						 it->second.first.minute,
						 it->second.first.second);
					LOGD("%s TIME OFF: %d-%d:%d:%d", it->first,
						 it->second.second.sw,
						 it->second.second.hour,
						 it->second.second.minute,
						 it->second.second.second);
				}
			}
		}
	}

	uint32_t RTC::incDecTimeData(uint32_t value, const uint32_t max, const uint32_t min, const bool incDec)
	{
		// if incDec == true then increase value
		if (incDec == true)
		{
			if (value < max)
			{
				value++;
			}
			else if (value >= max)
			{
				value = min;
			}
			else
			{
				// Do nothing
			}
		}
		else
		{
			if (value > 0)
			{
				value--;
			}
			else if (value == 0)
			{
				value = max;
			}
			else
			{
				// Do nothing
			}
		}
		return value;
	}

	void RTC::handleOKIntoSettingConfig()
	{
		std::vector<int32_t> vecData = {static_cast<int32_t>(mIndexListLight)};
		std::unique_ptr<utils::Package> packData = std::make_unique<utils::Package>(vecData);
		mRML->handleSignal(utils::SignalType::LCD_MENU_SETTING_CONFIG_BACK, packData.get());
		std::vector<int32_t> vecDataTime = {
			static_cast<int32_t>(mIndexListLight),
			static_cast<int32_t>(mTimeOfLight[LISTLIGHT[mIndexListLight]].first.sw),
			static_cast<int32_t>(mTimeOfLight[LISTLIGHT[mIndexListLight]].first.hour),
			static_cast<int32_t>(mTimeOfLight[LISTLIGHT[mIndexListLight]].first.minute),
			static_cast<int32_t>(mTimeOfLight[LISTLIGHT[mIndexListLight]].first.second),
			static_cast<int32_t>(mTimeOfLight[LISTLIGHT[mIndexListLight]].second.sw),
			static_cast<int32_t>(mTimeOfLight[LISTLIGHT[mIndexListLight]].second.hour),
			static_cast<int32_t>(mTimeOfLight[LISTLIGHT[mIndexListLight]].second.minute),
			static_cast<int32_t>(mTimeOfLight[LISTLIGHT[mIndexListLight]].second.second)};
		for (auto it = vecDataTime.begin(); it != vecDataTime.end(); ++it)
		{
			LOGD("Data time to EEPROM: %d", *it);
		}
		std::unique_ptr<utils::Package> packDataTime = std::make_unique<utils::Package>(vecDataTime);
		mRML->handleSignal(utils::SignalType::EEPROM_RTC_SEND_LIGHT_ON_OFF_DATA, packDataTime.get());
		mRML->handleSignal(utils::SignalType::NETWORK_SEND_LIGHT_STATUS_TO_FIREBASE, packDataTime.get());
		mIndexLight = LightOfTimeField::swOn;
	}

	uint16_t  RTC::convertTimeToMinute(const struct LightOfTime& time)
	{
		uint16_t result = 0U;
		result = static_cast<uint16_t>(time.hour * 60 + time.minute);
		return result;
	}

	uint16_t  RTC::convertTimeToMinute(const struct TimeDS1307& time)
	{
		uint16_t result = 0U;
		result = static_cast<uint16_t>(time.hour * 60 + time.minute);
		return result;
	}

	void RTC::handleEnableButtonLED()
	{
		uint16_t currentTimeInMinute 	= convertTimeToMinute(mAllTimeData);
		uint16_t enableButtonLED 		= convertTimeToMinute(BUTTON_LED_ON_TIME);
		uint16_t disableButtonLED 		= convertTimeToMinute(BUTTON_LED_OFF_TIME);

		if (currentTimeInMinute < disableButtonLED)
		{
			// +1440 minutes to skip the next day
			currentTimeInMinute += 1440U;
		}
		disableButtonLED += 1440;

		if (currentTimeInMinute >= enableButtonLED && currentTimeInMinute < disableButtonLED)
		{
			std::vector<int32_t> vecData{1U};
			std::unique_ptr<utils::Package> packData = std::make_unique<utils::Package>(vecData);
			mRML->handleSignal(utils::SignalType::LIGHT_IS_ENABLE_BUTTON_LED, packData.get());
			mRML->handleSignal(utils::SignalType::EEPROM_STORE_LIGHT_IS_ENABLE_BUTTON_LED, packData.get());
		}
		else
		{
			std::vector<int32_t> vecData{0U};
			std::unique_ptr<utils::Package> packData = std::make_unique<utils::Package>(vecData);
			mRML->handleSignal(utils::SignalType::LIGHT_IS_ENABLE_BUTTON_LED, packData.get());
			mRML->handleSignal(utils::SignalType::EEPROM_STORE_LIGHT_IS_ENABLE_BUTTON_LED, packData.get());
		}
	}

	void RTC::sendCurrentTimeToNetwork()
	{
		std::vector<int32_t> vecData = {
			static_cast<int32_t>(mAllTimeData.date),
			static_cast<int32_t>(mAllTimeData.month),
			static_cast<int32_t>(mAllTimeData.year),
			static_cast<int32_t>(mAllTimeData.hour),
			static_cast<int32_t>(mAllTimeData.minute),
			static_cast<int32_t>(mAllTimeData.second),
		};
		std::unique_ptr<utils::Package> packData = std::make_unique<utils::Package>(vecData);
		mRML->handleSignal(utils::SignalType::RTC_SEND_ALL_TIME_DATA, packData.get());
	}
}