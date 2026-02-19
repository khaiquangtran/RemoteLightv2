#include "LCD16x2.h"
#include "./../RemoteLight.h"

namespace hardware
{
	LCD16x2::LCD16x2(std::shared_ptr<remoteLight::RemoteLight> rml) : mRML(rml), mRetry(0U)
	{
		mButtonStringMap = {
			{0, "Button 1"},
			{1, "Button 2"},
			{2, "Button 3"},
			{3, "Button 4"},
			{4, "Button 5"},
			{5, "Button Up"},
			{6, "Button Down"},
			{7, "Button Right"},
			{8, "Button Left"},
			{9, "Button Ok"},
			{10, "Button Menu"},
			{11, "Button App"},
			{12, "Button Back"},
		};

		DAY[0] = new char[2];
		DAY[0][0] = ' ';
		DAY[0][1] = '\0';
		DAY[1] = new char[4];
		DAY[1][0] = 'S';
		DAY[1][1] = 'U';
		DAY[1][2] = 'N';
		DAY[1][3] = '\0';
		DAY[2] = new char[4];
		DAY[2][0] = 'M';
		DAY[2][1] = 'O';
		DAY[2][2] = 'N';
		DAY[2][3] = '\0';
		DAY[3] = new char[4];
		DAY[3][0] = 'T';
		DAY[3][1] = 'U';
		DAY[3][2] = 'E';
		DAY[3][3] = '\0';
		DAY[4] = new char[4];
		DAY[4][0] = 'W';
		DAY[4][1] = 'E';
		DAY[4][2] = 'D';
		DAY[4][3] = '\0';
		DAY[5] = new char[4];
		DAY[5][0] = 'T';
		DAY[5][1] = 'H';
		DAY[5][2] = 'U';
		DAY[5][3] = '\0';
		DAY[6] = new char[4];
		DAY[6][0] = 'F';
		DAY[6][1] = 'R';
		DAY[6][2] = 'I';
		DAY[6][3] = '\0';
		DAY[7] = new char[4];
		DAY[7][0] = 'S';
		DAY[7][1] = 'A';
		DAY[7][2] = 'T';
		DAY[7][3] = '\0';

		mFlagLCDLightOn = false;
		LOGD(" =========== LCD =========== ");
	}

	void LCD16x2::init()
	{
#if NOT_CONNECT_DEVICE_LCD
		LOGW("LCD16x2 init skipped due to NOT_CONNECT_DEVICE_LCD is defined");
		return;
#endif
#ifndef INIT_I2C
#define INIT_I2C
		Wire.begin();
#endif
	retry:
		if (checkAddress())
		{
			mLCD = new LiquidCrystal_I2C(LCD_ADDR, 20, 4); // set the LCD address to 0x27 for a 16 chars and 2 line display
			mLCD->init();
			mLCD->backlight();
		}
		else
		{
			if (mRetry < RETRY)
			{
				mRetry++;
				LOGW("retry %d", mRetry);
				goto retry;
			}
		}
	}

	LCD16x2::~LCD16x2()
	{
		delete mLCD;
		delete[] DAY[0];
		delete[] DAY[1];
		delete[] DAY[2];
		delete[] DAY[3];
		delete[] DAY[4];
		delete[] DAY[5];
		delete[] DAY[6];
		delete[] DAY[7];
	}

	void LCD16x2::handleSignal(const utils::SignalType &signal, const utils::Package *data)
	{
		if (mRetry >= RETRY)
		{
			LOGE("Can't connect to LCD16x2");
			return;
		}
		else
		{
			// LOGD("Handle signal value: %d", signal);
			switch (signal)
			{
			case (utils::SignalType::LCD_DISPLAY_ALL_TIME):
			{
				displayTimeFromDS1307(data);
				break;
			}
			case (utils::SignalType::LCD_CLEAR_TURN_OFF_SCREEN):
			{
				mLCD->clear();
				mLCD->noBacklight();
				mFlagLCDLightOn = false;
				break;
			}
			case (utils::SignalType::LCD_CLEAR_SCREEN):
			{
				mLCD->clear();
				break;
			}
			case (utils::SignalType::LCD_SETTING_RTC_TIME_START):
			{
				displaySettingRTCTimeStart();
				break;
			}
			case (utils::SignalType::LCD_TURN_ON_LIGHT):
			{
				if (mFlagLCDLightOn == false)
				{
					mFlagLCDLightOn = true;
					mLCD->backlight();
				}
				else
				{
					LOGD("LCD backlight is already ON");
				}
				break;
			}
			case (utils::SignalType::LCD_SETTING_RTC_TIME_END):
			{
				displaySettingRTCTimeEnd();
				break;
			}
			case (utils::SignalType::IR_BTN_MENU_SIGNAL):
			{
				displayMenuMode(0);
				break;
			}
			case (utils::SignalType::LCD_MENU_SETTING_CONFIG_BACK):
			case (utils::SignalType::LCD_MOVE_RIGHT_MENU_MODE):
			case (utils::SignalType::LCD_MOVE_LEFT_MENU_MODE):
			{
				const int32_t *value = data->getPackage();
				displayMenuMode(*value);
				break;
			}
			case (utils::SignalType::LCD_MENU_MODE_OK):
			{
				displaySelectedLightTimeConfig(data);
				break;
			}
			case (utils::SignalType::LCD_DISPLAY_START_CONNECT_WIFI):
			{
				displayStartConnectWifi();
				break;
			}
			case (utils::SignalType::LCD_DISPLAY_CONNECT_WIFI):
			{
				displayConnecting();
				break;
			}
			case (utils::SignalType::LCD_CONNECT_WIFI_SUCCESS):
			{
				displayConnectWifiSuccess();
				break;
			}
			case (utils::SignalType::LCD_CONNECT_WIFI_FAILED):
			{
				displayConnectWifiFailed();
				break;
			}
			case (utils::SignalType::LCD_DISPLAY_START_CONNECT_FIREBASE):
			{
				displayStartConnectFirebase();
				break;
			}
			case (utils::SignalType::LCD_DISPLAY_START_CONNECT_NTP):
			{
				displayStartConnectNTP();
				break;
			}
			case (utils::SignalType::LCD_CONNECT_FIREBASE_SUCCESS):
			{
				displayConnectFirebaseSuccess();
				break;
			}
			case (utils::SignalType::LCD_CONNECT_NTP_SUCCESS):
			{
				displayConnectNTPSuccess();
				break;
			}
			case (utils::SignalType::LCD_CONNECT_FIREBASE_FAILED):
			{
				displayConnectFirebaseFailed();
				break;
			}
			case (utils::SignalType::LCD_CONNECT_NTP_FAILED):
			{
				displayConnectNTPFailed();
				break;
			}
			case (utils::SignalType::LCD_INSTALL_BUTTON1):
			{
				displayInstallButton1();
				break;
			}
			case (utils::SignalType::LCD_DISPLAY_IRBUTTON_INSTALL):
			{
				displayInstallButton(data);
				break;
			}
			case (utils::SignalType::LCD_START_PROVISIONING):
			{
				displayStartProvisioning(data);
				break;
			}
			case (utils::SignalType::LCD_PROVISIONING_FAILED):
			{
				displayProvisioningFailed();
				break;
			}
			case (utils::SignalType::LCD_PROVISIONING_SUCCESS):
			{
				displayProvisioningSuccess();
				break;
			}
			case (utils::SignalType::LCD_INSTALL_BUTTON_START):
			{
				displayInstallButtonModeStart();
				break;
			}
			case (utils::SignalType::LCD_LIGHT_IRBUTTON_INSTALL_DONE):
			{
				displayInstallButtonDone(data);
				break;
			}
			case (utils::SignalType::LCD_INSTALL_BUTTON_DONE):
			{
				displayInstallButtonModeDone();
				break;
			}
			default:
				LOGW("Signal is not supported yet.");
				break;
			}
		}
	}

	bool LCD16x2::checkAddress()
	{
		if (scanAddress(LCD_ADDR) == HardwareBase::INVALID)
		{
			LOGD("No find LCD");
			return false;
		}
		else
		{
			LOGD("Find out LCD");
			return true;
		}
	}

	void LCD16x2::displayTimeFromDS1307(const utils::Package *data)
	{
		const int32_t *value = data->getPackage();
		mReceiverTime.second = static_cast<uint8_t>(*value++);
		mReceiverTime.minute = static_cast<uint8_t>(*value++);
		mReceiverTime.hour = static_cast<uint8_t>(*value++);
		mReceiverTime.day = static_cast<uint8_t>(*value++);
		mReceiverTime.date = static_cast<uint8_t>(*value++);
		mReceiverTime.month = static_cast<uint8_t>(*value++);
		mReceiverTime.year = static_cast<uint16_t>(*value);
#if NOT_CONNECT_DEVICE_LCD
		LOGW("displayTimeFromDS1307 skipped due to NOT_CONNECT_DEVICE_LCD is defined");
		LOGD("%d %d/%d/%d", static_cast<int32_t>(mReceiverTime.day),
			 static_cast<int32_t>(mReceiverTime.date),
			 static_cast<int32_t>(mReceiverTime.month),
			 static_cast<int32_t>(mReceiverTime.year));
		LOGD("%d:%d:%d", static_cast<int32_t>(mReceiverTime.hour),
			 static_cast<int32_t>(mReceiverTime.minute),
			 static_cast<int32_t>(mReceiverTime.second));
		return;
#endif
		mLCD->setCursor(0, 0);
		mLCD->print(mReceiverTime.day[DAY]);
		mLCD->print(" ");
		if (mReceiverTime.date < 10)
		{
			mLCD->print("0");
		}
		mLCD->print(mReceiverTime.date);
		mLCD->print("/");
		if (mReceiverTime.month < 10)
		{
			mLCD->print("0");
		}
		mLCD->print(mReceiverTime.month);
		mLCD->print("/");
		mLCD->print(mReceiverTime.year);
		mLCD->print("   ");

		mLCD->setCursor(0, 1);
		mLCD->print("Time: ");
		if (mReceiverTime.hour < 10)
		{
			mLCD->print("0");
		}
		mLCD->print(mReceiverTime.hour);
		mLCD->print(":");
		if (mReceiverTime.minute < 10)
		{
			mLCD->print("0");
		}
		mLCD->print(mReceiverTime.minute);
		mLCD->print(":");
		if (mReceiverTime.second < 10)
		{
			mLCD->print("0");
		}
		mLCD->print(mReceiverTime.second);
		mLCD->print("   ");
		LOGD("%d %d/%d/%d", static_cast<int32_t>(mReceiverTime.day),
			 static_cast<int32_t>(mReceiverTime.date),
			 static_cast<int32_t>(mReceiverTime.month),
			 static_cast<int32_t>(mReceiverTime.year));
		LOGD("%d:%d:%d", static_cast<int32_t>(mReceiverTime.hour),
			 static_cast<int32_t>(mReceiverTime.minute),
			 static_cast<int32_t>(mReceiverTime.second));
	}

	void LCD16x2::displaySettingRTCTimeStart()
	{
		LOGD(".");
#if NOT_CONNECT_DEVICE_LCD
		LOGW("displaySettingRTCTimeStart skipped due to NOT_CONNECT_DEVICE_LCD is defined");
		return;
#endif
		mLCD->clear();
		mLCD->backlight();
		mLCD->setCursor(0, 0);
		mLCD->print("    SETTING     ");
		mLCD->setCursor(0, 1);
		mLCD->print("   TIME RTC     ");
	}

	void LCD16x2::displaySettingRTCTimeEnd()
	{
		LOGD(".");
#if NOT_CONNECT_DEVICE_LCD
		LOGW("displaySettingRTCTimeEnd skipped due to NOT_CONNECT_DEVICE_LCD is defined");
		return;
#endif
		mLCD->clear();
		mLCD->backlight();
		mLCD->setCursor(0, 0);
		mLCD->print("    SETTING     ");
		mLCD->setCursor(0, 1);
		mLCD->print("   COMPLETE     ");
	}

	void LCD16x2::displayMenuMode(const uint8_t light)
	{
#if NOT_CONNECT_DEVICE_LCD
		LOGW("displayMenuMode skipped due to NOT_CONNECT_DEVICE_LCD is defined");
		return;
#endif
		LOGD("Light %d", light);
		mLCD->setCursor(0, 0);
		mLCD->print("  SELECT LIGHT ");
		mLCD->setCursor(0, 1);
		switch (light)
		{
		case 0:
			mLCD->print("  >1< 2  3  4   ");
			break;
		case 1:
			mLCD->print("   1 >2< 3  4   ");
			break;
		case 2:
			mLCD->print("   1  2 >3< 4   ");
			break;
		case 3:
			mLCD->print("   1  2  3 >4<  ");
			break;
		default:
			break;
		}
	}

	void LCD16x2::displaySelectedLightTimeConfig(const utils::Package *data)
	{
#if NOT_CONNECT_DEVICE_LCD
		LOGW("displaySelectedLightTimeConfig skipped due to NOT_CONNECT_DEVICE_LCD is defined");
		return;
#endif

		if (data == nullptr)
		{
			LOGW("displaySelectedLightTimeConfig: data is null");
			return;
		}

		const int32_t *pack = data->getPackage();
		const int32_t size = data->getSize();
		if (pack == nullptr || size != 9U)
		{
			LOGW("displaySelectedLightTimeConfig: package is null or size is invalid");
			return;
		}

		const int32_t lightIndex = pack[0];

		const bool onEnable = pack[1];
		const int32_t onHour = pack[2];
		const int32_t onMin = pack[3];
		const int32_t onSec = pack[4];

		const bool offEnable = pack[5];
		const int32_t offHour = pack[6];
		const int32_t offMin = pack[7];
		const int32_t offSec = pack[8];

		// -------- Line 1 --------
		mLCD->setCursor(0, 0);
		mLCD->print("ON ");
		mLCD->print(lightIndex + 1);
		mLCD->print(onEnable ? " EN " : " DIS");

		print2Digit(onHour);
		mLCD->print(":");
		print2Digit(onMin);
		mLCD->print(":");
		print2Digit(onSec);

		// -------- Line 2 --------
		mLCD->setCursor(0, 1);
		mLCD->print("OFF");
		mLCD->print(lightIndex + 1);
		mLCD->print(offEnable ? " EN " : " DIS");

		print2Digit(offHour);
		mLCD->print(":");
		print2Digit(offMin);
		mLCD->print(":");
		print2Digit(offSec);
	}

	void LCD16x2::displayStartConnectWifi()
	{
		LOGD("Connecting Wifi");
#if NOT_CONNECT_DEVICE_LCD
		LOGW("displayStartConnectWifi skipped due to NOT_CONNECT_DEVICE_LCD is defined");
		return;
#endif
		mLCD->setCursor(0, 0);
		mLCD->print("Connecting Wifi  ");
		mLCD->setCursor(0, 1);
	}

	void LCD16x2::displayStartConnectFirebase()
	{
		LOGD("Connecting FB");
#if NOT_CONNECT_DEVICE_LCD
		LOGW("displayStartConnectFirebase skipped due to NOT_CONNECT_DEVICE_LCD is defined");
		return;
#endif
		mLCD->setCursor(0, 0);
		mLCD->print("Connecting FB    ");
		mLCD->setCursor(0, 1);
	}

	void LCD16x2::displayStartConnectNTP()
	{
		LOGD("Connecting NTP");
#if NOT_CONNECT_DEVICE_LCD
		LOGW("displayStartConnectNTP skipped due to NOT_CONNECT_DEVICE_LCD is defined");
		return;
#endif
		mLCD->setCursor(0, 0);
		mLCD->print("Connecting NTP  ");
		mLCD->setCursor(0, 1);
	}

	void LCD16x2::displayConnecting()
	{
		LOGD(".");
#if NOT_CONNECT_DEVICE_LCD
		return;
#endif
		mLCD->print(".");
	}

	void LCD16x2::displayConnectWifiSuccess()
	{
		LOGD("WiFi connection Successful");
#if NOT_CONNECT_DEVICE_LCD
		return;
#endif
		mLCD->clear();
		mLCD->setCursor(0, 0);
		mLCD->print("WiFi connection ");
		mLCD->setCursor(0, 1);
		mLCD->print("Successful");
	}

	void LCD16x2::displayConnectWifiFailed()
	{
		LOGD("WiFi connection Failed");
#if NOT_CONNECT_DEVICE_LCD
		return;
#endif
		mLCD->clear();
		mLCD->setCursor(0, 0);
		mLCD->print("WiFi connection ");
		mLCD->setCursor(0, 1);
		mLCD->print("Failed");
	}

	void LCD16x2::displayConnectFirebaseSuccess()
	{
		LOGD("Firebase connection Successful");
#if NOT_CONNECT_DEVICE_LCD
		return;
#endif
		mLCD->clear();
		mLCD->setCursor(0, 0);
		mLCD->print("FB connection  ");
		mLCD->setCursor(0, 1);
		mLCD->print("Successful");
	}

	void LCD16x2::displayConnectNTPSuccess()
	{
		LOGD("NTP connection Successful");
#if NOT_CONNECT_DEVICE_LCD
		return;
#endif
		mLCD->clear();
		mLCD->setCursor(0, 0);
		mLCD->print("NTP connection   ");
		mLCD->setCursor(0, 1);
		mLCD->print("Successful");
	}

	void LCD16x2::displayConnectFirebaseFailed()
	{
		LOGD("Firebase connection Failed");
#if NOT_CONNECT_DEVICE_LCD
		return;
#endif
		mLCD->clear();
		mLCD->setCursor(0, 0);
		mLCD->print("FB connection  ");
		mLCD->setCursor(0, 1);
		mLCD->print("Failed");
	}

	void LCD16x2::displayConnectNTPFailed()
	{
		LOGD("NTP connection Failed");
#if NOT_CONNECT_DEVICE_LCD
		return;
#endif
		mLCD->clear();
		mLCD->setCursor(0, 0);
		mLCD->print("NTP connection ");
		mLCD->setCursor(0, 1);
		mLCD->print("Failed");
	}

	void LCD16x2::displayInstallButton(const utils::Package *data)
	{
#if NOT_CONNECT_DEVICE_LCD
		return;
#endif
		if (data->getSize() == 2)
		{
			const int32_t *parseData = data->getPackage();
			if (mButtonStringMap.find(parseData[0]) != mButtonStringMap.end())
			{
				String str = mButtonStringMap.at(parseData[0]);
				int32_t total = parseData[1];
				LOGD("Total: %x", total);
				mLCD->clear();
				mLCD->setCursor(0, 0);
				mLCD->print(str);
				mLCD->setCursor(0, 1);
				mLCD->print("0x");
				mLCD->print(total, HEX);
			}
			else
			{
				LOGE("Can't find button in mButtonStringMap");
			}
		}
		else
		{
			LOGE("Data size is invalid");
		}
	}

	void LCD16x2::displayInstallButtonDone(const utils::Package *data)
	{
#if NOT_CONNECT_DEVICE_LCD
		return;
#endif
		if (data->getSize() == 1)
		{
			const int32_t *parseData = data->getPackage();
			const int32_t buttonNumber = parseData[0] - 1;
			if (mButtonStringMap.find(buttonNumber) != mButtonStringMap.end())
			{
				String str = mButtonStringMap.at(buttonNumber);
				mLCD->clear();
				mLCD->setCursor(0, 0);
				mLCD->print(str);
				mLCD->setCursor(0, 1);
				mLCD->print("DONE");
			}
			else
			{
				LOGE("Can't find button in mButtonStringMap");
			}
		}
		else
		{
			LOGE("Data size is invalid");
		}
	}

	void LCD16x2::displayInstallButtonModeDone()
	{
#if NOT_CONNECT_DEVICE_LCD
		LOGW("displayInstallButtonModeDone skipped due to NOT_CONNECT_DEVICE_LCD is defined");
		return;
#endif
		mLCD->clear();
		mLCD->setCursor(0, 0);
		mLCD->print("INSTALL IR BTN");
		mLCD->setCursor(0, 1);
		mLCD->print("   COMPLETE    ");
	}

	void LCD16x2::displayStartProvisioning(const utils::Package *data)
	{
#if NOT_CONNECT_DEVICE_LCD
		LOGW("displayStartProvisioning skipped due to NOT_CONNECT_DEVICE_LCD is defined");
		return;
#endif
		const int32_t *dataStr = data->getPackage();
		int32_t size = data->getSize();

		String str;
		str.reserve(size);
		for (int32_t i = 0; i < size; i++)
		{
			str += static_cast<char>(dataStr[i]);
		}
		int32_t sep = str.indexOf('%');
		String serviceName = "";
		// otherwise called a PIN
		String pop = "";
		if (sep != -1)
		{
			serviceName = str.substring(0, sep);
			pop = str.substring(sep + 1);
		}
		mLCD->clear();
		mLCD->setCursor(0, 0);
		mLCD->print(serviceName.c_str());
		mLCD->setCursor(0, 1);
		mLCD->print(pop.c_str());
	}

	void LCD16x2::displayProvisioningFailed()
	{
		LOGD("Provisioning Failed");
#if NOT_CONNECT_DEVICE_LCD
		return;
#endif
		mLCD->clear();
		mLCD->setCursor(0, 0);
		mLCD->print("  PROVISIONING ");
		mLCD->setCursor(0, 1);
		mLCD->print("     FAILED!   ");
	}

	void LCD16x2::displayProvisioningSuccess()
	{
		LOGD("Provisioning Success");
#if NOT_CONNECT_DEVICE_LCD
		return;
#endif
		mLCD->clear();
		mLCD->setCursor(0, 0);
		mLCD->print("  PROVISIONING ");
		mLCD->setCursor(0, 1);
		mLCD->print("     SUCCESS   ");
	}

	void LCD16x2::displayInstallButtonModeStart()
	{
#if NOT_CONNECT_DEVICE_LCD
		LOGW("displayInstallButtonModeStart skipped due to NOT_CONNECT_DEVICE_LCD is defined");
		return;
#endif
		mLCD->clear();
		mLCD->setCursor(0, 0);
		mLCD->print(" INSTALL IR BTN ");
		mLCD->setCursor(0, 1);
		mLCD->print(" PRESS ANY BTN  ");
	}

	void LCD16x2::displayInstallButton1()
	{
#if NOT_CONNECT_DEVICE_LCD
		LOGW("displayInstallButton1 skipped due to NOT_CONNECT_DEVICE_LCD is defined");
		return;
#endif
		mLCD->clear();
		mLCD->setCursor(0, 0);
		mLCD->print("Button 1");
	}

	void LCD16x2::print2Digit(const int32_t& value)
	{
#if NOT_CONNECT_DEVICE_LCD
		LOGW("print2Digit skipped due to NOT_CONNECT_DEVICE_LCD is defined");
		return;
#endif
		if (value < 10)
		{
			mLCD->print("0");
		}
        mLCD->print(value);
	}

} // namespace hardware