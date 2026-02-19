#include "./Light.h"
#include "./../RemoteLight.h"

namespace hardware
{
	Light::Light(std::shared_ptr<remoteLight::RemoteLight> rml) : mRML(rml)
	{

		// Trigger LOW level
		mListLight[LIGHT_1] 	= HIGH;
		mListLight[LIGHT_2] 	= HIGH;
		mListLight[LIGHT_3] 	= HIGH;
		mListLight[LIGHT_4] 	= HIGH;
		mListLight[LED_BTN_RST] = HIGH;
		mListLight[LED_BTN_1] 	= HIGH;
		mListLight[LED_BTN_2] 	= HIGH;

		mSignalLightMap[utils::SignalType::IR_BTN_1_SIGNAL] 					= LIGHT_1;
		mSignalLightMap[utils::SignalType::BTN_PRESS_BTN_1_SIGNAL] 				= LIGHT_1;
		mSignalLightMap[utils::SignalType::RTC_TURN_ON_LIGHT1] 					= LIGHT_1;
		mSignalLightMap[utils::SignalType::RTC_TURN_OFF_LIGHT1] 				= LIGHT_1;
		mSignalLightMap[utils::SignalType::IR_BTN_2_SIGNAL] 					= LIGHT_2;
		mSignalLightMap[utils::SignalType::BTN_PRESS_BTN_2_SIGNAL] 				= LIGHT_2;
		mSignalLightMap[utils::SignalType::RTC_TURN_ON_LIGHT2] 					= LIGHT_2;
		mSignalLightMap[utils::SignalType::RTC_TURN_OFF_LIGHT2] 				= LIGHT_2;
		mSignalLightMap[utils::SignalType::IR_BTN_3_SIGNAL] 					= LIGHT_3;
		mSignalLightMap[utils::SignalType::RTC_TURN_ON_LIGHT3] 					= LIGHT_3;
		mSignalLightMap[utils::SignalType::RTC_TURN_OFF_LIGHT3] 				= LIGHT_3;
		mSignalLightMap[utils::SignalType::IR_BTN_4_SIGNAL] 					= LIGHT_4;
		mSignalLightMap[utils::SignalType::RTC_TURN_ON_LIGHT4] 					= LIGHT_4;
		mSignalLightMap[utils::SignalType::RTC_TURN_OFF_LIGHT4] 				= LIGHT_4;

		mIsBtnLedEnabled = false;

		std::map<uint8_t, bool>::iterator it;
		#if NOT_CONNECT_DEVICE
			LOGW("LIGHT define skipped. NOT_CONNECT_DEVICE is defined");
		#else
			for (it = mListLight.begin(); it != mListLight.end(); it++)
			{
				pinMode(it->first, OUTPUT);
				digitalWrite(it->first, it->second);
			}
		#endif
		LOGI("================== Light ==================");
	}

	void Light::handleSignal(const utils::SignalType& signal, const utils::Package* data)
	{
		#if NOT_CONNECT_DEVICE
			LOGW("NOT_CONNECT_DEVICE is defined. Signal can't handle!");
			return;
		#endif
		switch (signal)
		{
		case utils::SignalType::IR_BTN_1_SIGNAL:
		case utils::SignalType::BTN_PRESS_BTN_1_SIGNAL:
		case utils::SignalType::IR_BTN_2_SIGNAL:
		case utils::SignalType::BTN_PRESS_BTN_2_SIGNAL:
		case utils::SignalType::IR_BTN_3_SIGNAL:
		case utils::SignalType::IR_BTN_4_SIGNAL:
		{
			controlLight(mSignalLightMap[signal]);
			controlButtonLED(signal);
			break;
		}
		case utils::SignalType::RTC_TURN_ON_LIGHT1:
		case utils::SignalType::RTC_TURN_ON_LIGHT2:
		case utils::SignalType::RTC_TURN_ON_LIGHT3:
		case utils::SignalType::RTC_TURN_ON_LIGHT4:
		{
			controlLight(mSignalLightMap[signal], 1);
			controlButtonLED(signal);
			break;
		}
		case utils::SignalType::RTC_TURN_OFF_LIGHT1:
		case utils::SignalType::RTC_TURN_OFF_LIGHT2:
		case utils::SignalType::RTC_TURN_OFF_LIGHT3:
		case utils::SignalType::RTC_TURN_OFF_LIGHT4:
		{
			controlLight(mSignalLightMap[signal], 0);
			controlButtonLED(signal);
			break;
		}
		case utils::SignalType::LIGHT_LED_RESET_BTN_ON:
		{
			controlLight(LED_BTN_RST, 1);
			break;
		}
		case utils::SignalType::LIGHT_LED_RESET_BTN_OFF:
		{
			controlLight(LED_BTN_RST, 0);
			break;
		}
		case utils::SignalType::LIGHT_IS_ENABLE_BUTTON_LED:
		{
			handleEnableButtonLED(data);
			break;
		}
		case (utils::SignalType::LIGHT_STATE_FROM_EEPROM):
		{
			parseLightStateDataFromEEPROM(data);
			break;
		}
		default:
		{
			LOGW("Signal is not supported yet.");
			break;
		}
		}
	}

	void Light::controlLight(uint8_t light, uint8_t state)
	{
		if(state == 3)
		{
			mListLight[light] = (mListLight[light] == HIGH ? LOW : HIGH);
			#if NOT_CONNECT_DEVICE == 0
				digitalWrite(light, mListLight[light]);
			#endif
			if (mListLight[light])
			{
				LOGI("Turn OFF %d", light);
			}
			else
			{
				LOGI("Turn ON %d", light);
			}
		}
		else if(state == 1)
		{
			if(mListLight[light]  != LOW)
			{
				mListLight[light] = LOW;
				#if NOT_CONNECT_DEVICE == 0
					digitalWrite(light, mListLight[light]);
				#endif
			}
		}
		else if(state == 0)
		{
			if(mListLight[light]  != HIGH)
			{
				mListLight[light] = HIGH;
				#if NOT_CONNECT_DEVICE == 0
					digitalWrite(light, mListLight[light]);
				#endif
			}
		}
		else {
			/*Do nothing*/
		}

		std::vector<int32_t> vecData = {static_cast<int32_t>(light), static_cast<int32_t>(mListLight[light] == LOW ? 1 : 0)};
		std::unique_ptr<utils::Package> packData = std::make_unique<utils::Package>(vecData);
		mRML->handleSignal(utils::SignalType::EEPROM_STORE_STATE_LIGHT, packData.get());
	}

	void Light::controlButtonLED(const utils::SignalType& signal)
	{
		if(mIsBtnLedEnabled == false)
		{
			return;
		}
		else
		{
			if(signal == utils::SignalType::BTN_PRESS_BTN_1_SIGNAL || signal == utils::SignalType::IR_BTN_1_SIGNAL
				|| signal == utils::SignalType::RTC_TURN_ON_LIGHT1 || signal == utils::SignalType::RTC_TURN_OFF_LIGHT1)
			{
				if(mListLight[LIGHT_1] == HIGH)  // Light 1 turn OFF
				{
					mListLight[LED_BTN_1] = LOW;
					digitalWrite(LED_BTN_1, mListLight[LED_BTN_1]);
				}
				else
				{
					mListLight[LED_BTN_1] = HIGH;
					digitalWrite(LED_BTN_1, mListLight[LED_BTN_1]);
				}
			}
			else if(signal == utils::SignalType::BTN_PRESS_BTN_2_SIGNAL || signal == utils::SignalType::IR_BTN_2_SIGNAL
					|| signal == utils::SignalType::RTC_TURN_ON_LIGHT2 || signal == utils::SignalType::RTC_TURN_OFF_LIGHT2)
			{
				if(mListLight[LIGHT_2] == HIGH)  // Light 2 turn OFF
				{
					mListLight[LED_BTN_2] = LOW;
					digitalWrite(LED_BTN_2, mListLight[LED_BTN_2]);
				}
				else
				{
					mListLight[LED_BTN_2] = HIGH;
					digitalWrite(LED_BTN_2, mListLight[LED_BTN_2]);
				}
			}
		}
	}

	void Light::parseLightStateDataFromEEPROM(const utils::Package* data)
	{
		if(data == nullptr)
		{
			LOGE("Data from EEPROM is null.");
			return;
		}
		else
		{
			const int32_t size = data->getSize();
			const int32_t* value = data->getPackage();
			if(size % 2 != 0)
			{
				LOGE("Data from EEPROM with length is invalid.");
				return;
			}
			else
			{
				for(int32_t i = 0; i < size; i += 2)
				{
					const uint8_t lightIndex = static_cast<uint8_t>(value[i]);
					const uint8_t lightState = static_cast<uint8_t>(value[i + 1]);
					if(lightState == 1)
					{
						controlLight(lightIndex, 1);
					}
					else
					{
						controlLight(lightIndex, 0);
					}
				}
			}
		}
	}

	void Light::handleEnableButtonLED(const utils::Package *data)
	{
		if (data->getSize() != 1)
		{
			LOGW("Data is invalid");
			return;
		}
		else
		{
			const int32_t *parseData = data->getPackage();
			if (parseData[0] == 1)
			{
				mIsBtnLedEnabled = true;
				if (mListLight[LIGHT_1] == HIGH)
				{
					mListLight[LED_BTN_1] = LOW;
					digitalWrite(LED_BTN_1, mListLight[LED_BTN_1]);
				}
				if (mListLight[LIGHT_2] == HIGH)
				{
					mListLight[LED_BTN_2] = LOW;
					digitalWrite(LED_BTN_2, mListLight[LED_BTN_2]);
				}
			}
			else
			{
				mIsBtnLedEnabled = false;
				mListLight[LED_BTN_1] = HIGH;
				digitalWrite(LED_BTN_1, mListLight[LED_BTN_1]);
				mListLight[LED_BTN_2] = HIGH;
				digitalWrite(LED_BTN_2, mListLight[LED_BTN_2]);
			}
		}
	}
} // namespace hardware