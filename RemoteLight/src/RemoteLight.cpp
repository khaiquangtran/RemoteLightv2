#include "./RemoteLight.h"

namespace remoteLight
{
	RemoteLight::RemoteLight()
	{
		mMapSettingRTCSignalToControlMode = {
			{utils::SignalType::IR_BTN_UP_SIGNAL, utils::CONTROL_MODE::SETTING_RTC_TIME_UP},
			{utils::SignalType::IR_BTN_DOWN_SIGNAL, utils::CONTROL_MODE::SETTING_RTC_TIME_DOWN},
			{utils::SignalType::IR_BTN_LEFT_SIGNAL, utils::CONTROL_MODE::SETTING_RTC_TIME_LEFT},
			{utils::SignalType::IR_BTN_RIGHT_SIGNAL, utils::CONTROL_MODE::SETTING_RTC_TIME_RIGHT},
			{utils::SignalType::IR_BTN_OK_SIGNAL, utils::CONTROL_MODE::SETTING_RTC_TIME_OK},
		};

		mMapSettingLightTimeConfigSignalToControlMode = {
			{utils::SignalType::IR_BTN_UP_SIGNAL, utils::CONTROL_MODE::SETTING_LIGHT_TIME_CONFIG_UP},
			{utils::SignalType::IR_BTN_DOWN_SIGNAL, utils::CONTROL_MODE::SETTING_LIGHT_TIME_CONFIG_DOWN},
			{utils::SignalType::IR_BTN_LEFT_SIGNAL, utils::CONTROL_MODE::SETTING_LIGHT_TIME_CONFIG_LEFT},
			{utils::SignalType::IR_BTN_RIGHT_SIGNAL, utils::CONTROL_MODE::SETTING_LIGHT_TIME_CONFIG_RIGHT},
			{utils::SignalType::IR_BTN_OK_SIGNAL, utils::CONTROL_MODE::SETTING_LIGHT_TIME_CONFIG_OK},
			{utils::SignalType::IR_BTN_BACK_SIGNAL, utils::CONTROL_MODE::SETTING_LIGHT_TIME_CONFIG_BACK},
		};
		mCounterCheckDataFirebase = 0U;
		mCounterCheckDataNTP = 0U;
		mCounterHandleError = 0U;
		mFlagReconnectFirebase = false;

		LOGI(" ================== RemoteLight ================== ");
	}

	RemoteLight::~RemoteLight()
	{
	}

	void RemoteLight::init()
	{
		mSerial 	= std::make_shared<hardware::SerialPartner>(shared_from_this());
		mEEPROM 	= std::make_shared<hardware::EEPROMPartner>(shared_from_this());
		mRTC 		= std::make_shared<hardware::RTC>(shared_from_this());
		mIR 		= std::make_shared<hardware::IRRemotePartner>(shared_from_this());
		mLCD 		= std::make_shared<hardware::LCD16x2>(shared_from_this());
		mBTN 		= std::make_shared<hardware::Button>(shared_from_this());
		mLIGHT 		= std::make_shared<hardware::Light>(shared_from_this());
		mNetwork 	= std::make_shared<Network>(shared_from_this());

		mTasks = std::make_shared<Tasks>(shared_from_this(), mLCD, mRTC, mIR, mLIGHT, mNetwork);

		mTimerMgr = std::make_shared<TimerManager>(2);
		mTimer = mTimerMgr->createTimer([this]()
										{ this->onTimeout(); });
		mTimerOnOffLight = mTimerMgr->createTimer([this]()
												  { this->setLightScheduleCheckEnabled(); });
		mTimerOnOffLight->updateTimer([this]()
									  { this->setLightScheduleCheckEnabled(true); }, DELAY_1S);
		mTimerOnOffLight->startTimerInterval();

		mRTC->init();
		mLCD->init();
		mEEPROM->init();
		mBTN->init();

		// EEPROM module sends stored data to the RTC module
		mEEPROM->handleSignal(utils::SignalType::EEPROM_IR_REMOTE_GET_DATA);

		// EERPOM module will check SSID and Password, if there is data, it will be sent to the network module
		mEEPROM->handleSignal(utils::SignalType::EEPROM_IS_STORED_SSID_PASSWORD);

		// When the light is turned on/off, its state is saved to the EEPROM module
		// when it is reset, the previous state is sent to the Light.
		mEEPROM->handleSignal(utils::SignalType::EEPROM_SEND_STATE_LIGHT_TO_LIGHT);

		// Read time data from RTC
		mRTC->handleSignal(utils::SignalType::RTC_GET_ALL_TIME_DATA);

		// Check if the button is available
		mRTC->handleSignal(utils::SignalType::RTC_CHECK_ENABLE_BUTTON_LED);

		addControlMode(utils::CONTROL_MODE::CONNECT_WIFI);
	}

	void RemoteLight::run()
	{
		mIR->listenning();
		mBTN->listenning();
		// mSerial->listenning();
		process();

		if (isLightScheduleCheckEnabled() == true)
		{
			setLightScheduleCheckEnabled(false);

			// After 1 second, it will check if the light on/off time has been set
			mRTC->handleSignal(utils::SignalType::RTC_CHECK_CONFIGURED_TIME_FOR_LIGHT);
			handleUpdateData();
		}
	}

	void RemoteLight::handleSignal(const utils::SignalType signal, utils::Package *data)
	{
		// LOGD("Handle signal value: %d", signal);
		switch (signal)
		{
		case (utils::SignalType::BTN_PRESS_BTN_1_SIGNAL):
		case (utils::SignalType::BTN_PRESS_BTN_2_SIGNAL):
			if (mQueue.front().getControlMode() == utils::CONTROL_MODE::SETTING_BUTTON_IR_REMOTE && mQueue.front().getState() == utils::STATE::DELAY)
			{
				mTasks->handleControlMode(utils::CONTROL_MODE::SETTING_BUTTON_IR_REMOTE);
				break;
			}
		case (utils::SignalType::IR_BTN_1_SIGNAL):
		case (utils::SignalType::IR_BTN_2_SIGNAL):
		case (utils::SignalType::IR_BTN_3_SIGNAL):
		case (utils::SignalType::IR_BTN_4_SIGNAL):
		case (utils::SignalType::RTC_TURN_ON_LIGHT1):
		case (utils::SignalType::RTC_TURN_ON_LIGHT2):
		case (utils::SignalType::RTC_TURN_ON_LIGHT3):
		case (utils::SignalType::RTC_TURN_ON_LIGHT4):
		case (utils::SignalType::RTC_TURN_OFF_LIGHT1):
		case (utils::SignalType::RTC_TURN_OFF_LIGHT2):
		case (utils::SignalType::RTC_TURN_OFF_LIGHT3):
		case (utils::SignalType::RTC_TURN_OFF_LIGHT4):
		case (utils::SignalType::LIGHT_LED_RESET_BTN_ON):
		{
			mLIGHT->handleSignal(signal);
			break;
		}
		case (utils::SignalType::LCD_MOVE_LEFT_MENU_MODE):
		case (utils::SignalType::LCD_MOVE_RIGHT_MENU_MODE):
		case (utils::SignalType::LCD_MENU_MODE_OK):
		case (utils::SignalType::LCD_START_PROVISIONING):
		case (utils::SignalType::LCD_MENU_SETTING_CONFIG_BACK):
		case (utils::SignalType::LCD_DISPLAY_ALL_TIME):
		case (utils::SignalType::LCD_LIGHT_IRBUTTON_INSTALL_DONE):
		{
			mLCD->handleSignal(signal, data);
			break;
		}
		case (utils::SignalType::EEPROM_STORE_IRBUTTON_INSTALL):
		case (utils::SignalType::EEPROM_RTC_SEND_LIGHT_ON_OFF_DATA):
		{
			mEEPROM->handleSignal(signal, data);
			break;
		}
		case (utils::SignalType::LCD_DISPLAY_IRBUTTON_INSTALL):
		{
			mLCD->handleSignal(signal, data);
			break;
		}
		case (utils::SignalType::LCD_DISPLAY_CONNECT_WIFI):
		case (utils::SignalType::LCD_CONNECT_WIFI_SUCCESS):
		case (utils::SignalType::LCD_CONNECT_FIREBASE_SUCCESS):
		case (utils::SignalType::LCD_CONNECT_NTP_SUCCESS):
		case (utils::SignalType::LCD_CONNECT_WIFI_FAILED):
		case (utils::SignalType::LCD_CONNECT_FIREBASE_FAILED):
		case (utils::SignalType::LCD_CONNECT_NTP_FAILED):
		case (utils::SignalType::LCD_PROVISIONING_SUCCESS):
		case (utils::SignalType::LCD_CLEAR_SCREEN):
		{
			mLCD->handleSignal(signal);
			break;
		}
		case (utils::SignalType::IR_BTN_MENU_SIGNAL):
		{
			addControlMode(utils::CONTROL_MODE::SETTING_LIGHT_TIME_CONFIG);
			break;
		}
		case (utils::SignalType::IR_BTN_APP_SIGNAL):
		{
			addControlMode(utils::CONTROL_MODE::SETTING_RTC_TIME_START);
			break;
		}
		case (utils::SignalType::IR_BTN_UP_SIGNAL):
		case (utils::SignalType::IR_BTN_DOWN_SIGNAL):
		case (utils::SignalType::IR_BTN_LEFT_SIGNAL):
		case (utils::SignalType::IR_BTN_RIGHT_SIGNAL):
		case (utils::SignalType::IR_BTN_OK_SIGNAL):
		case (utils::SignalType::IR_BTN_BACK_SIGNAL):
		{
			if (mQueue.front().getControlMode() == utils::CONTROL_MODE::SETTING_RTC_TIME && mQueue.front().getState() == utils::STATE::DELAY)
			{
				if (mMapSettingRTCSignalToControlMode.find(signal) != mMapSettingRTCSignalToControlMode.end())
				{
					mTasks->handleControlMode(mMapSettingRTCSignalToControlMode[signal]);
				}
				else
				{
					LOGW("Can't find signal %s in mMapSettingRTCSignalToControlMode!", utils::SIGNALTOSTRING(signal).c_str());
				}
			}
			else if (mQueue.front().getControlMode() == utils::CONTROL_MODE::SETTING_LIGHT_TIME_CONFIG && mQueue.front().getState() == utils::STATE::DELAY)
			{
				if (mMapSettingLightTimeConfigSignalToControlMode.find(signal) != mMapSettingLightTimeConfigSignalToControlMode.end())
				{
					LOGD("Found signal %s in mMapSettingLightTimeConfigSignalToControlMode.", utils::SIGNALTOSTRING(signal).c_str());
					mTasks->handleControlMode(mMapSettingLightTimeConfigSignalToControlMode[signal]);
				}
				else
				{
					LOGW("Can't find signal %s in mMapSettingLightTimeConfigSignalToControlMode!", utils::SIGNALTOSTRING(signal).c_str());
				}
			}
			break;
		}
		case (utils::SignalType::TASKS_CONNECT_WIFI_SUCCESS):
		{
			if (mQueue.front().getControlMode() == utils::CONTROL_MODE::CONNECT_WIFI)
			{
				mQueue.front().setState(utils::STATE::DONE);
			}
			addControlMode(utils::CONTROL_MODE::CONNECT_WIFI_DONE);
			break;
		}
		case (utils::SignalType::REMOTE_LIGHT_CONNECT_WIFI_TIMEOUT):
		{
			if (mQueue.front().getControlMode() == utils::CONTROL_MODE::CONNECT_WIFI)
			{
				LOGD("CONNECT WIFI control mode is TIMEOUT.");
				mQueue.front().setState(utils::STATE::DONE);
			}
			addControlMode(utils::CONTROL_MODE::CONNECT_WIFI_FAILED);
			break;
		}
		case (utils::SignalType::REMOTE_LIGHT_CONNECT_FIREBASE_TIMEOUT):
		{
			if (mQueue.front().getControlMode() == utils::CONTROL_MODE::CONNECT_FIREBASE)
			{
				mQueue.front().setState(utils::STATE::DONE);
			}
			addControlMode(utils::CONTROL_MODE::CONNECT_FIREBASE_FAILED);
			break;
		}
		case (utils::SignalType::TASKS_CONNECT_WIFI_FAILED):
		{
			LOGW("Connect WIFI failed!");
			break;
		}
		case (utils::SignalType::TASKS_CONNECT_WIFI_FAILED_SSID_PASSWORD_EMPTY):
		{
			LOGW("SSID or Password is empty!");
			mTasks->handleControlMode(utils::CONTROL_MODE::CONNEC_WIFI_FAILED_SSID_PASSWORD_EMPTY);
			break;
		}
		case (utils::SignalType::TASKS_CONNECT_FIREBASE_SUCCESS):
		{
			if (mQueue.front().getControlMode() == utils::CONTROL_MODE::CONNECT_FIREBASE)
			{
				mQueue.front().setState(utils::STATE::DONE);
			}
			addControlMode(utils::CONTROL_MODE::CONNECT_FIREBASE_DONE);
			break;
		}
		case (utils::SignalType::TASKS_CONNECT_FIREBASE_FAILED):
		{
			LOGW("Connect FIREBASE failed!");
			break;
		}
		case (utils::SignalType::TASKS_CONNECT_NTP_SUCCESS):
		{
			if (mQueue.front().getControlMode() == utils::CONTROL_MODE::CONNECT_NTP)
			{
				mQueue.front().setState(utils::STATE::DONE);
			}
			addControlMode(utils::CONTROL_MODE::CONNECT_NTP_DONE);
			break;
		}
		case (utils::SignalType::TASKS_CONNECT_NTP_FAILED):
		{
			LOGW("Connect NTP failed!");
			break;
		}
		case (utils::SignalType::REMOTE_LIGHT_CONNECT_NTP_TIMEOUT):
		{
			if (mQueue.front().getControlMode() == utils::CONTROL_MODE::CONNECT_NTP)
			{
				mQueue.front().setState(utils::STATE::DONE);
			}
			addControlMode(utils::CONTROL_MODE::CONNECT_NTP_FAILED);
			break;
		}
		case (utils::SignalType::RTC_COUNTER_INSTALL_IRBUTTON_REACHED):
		{
			LOGW("RTC counter install IR button reached!");
			break;
		}
		case (utils::SignalType::NETWORK_SSID_PASSWORD_STORED):
		case (utils::SignalType::NETWORK_SEND_LIGHT_STATUS_TO_FIREBASE):
		case (utils::SignalType::RTC_SEND_ALL_TIME_DATA):
		{
			mNetwork->handleSignal(signal, data);
			break;
		}
		case (utils::SignalType::NETWORK_SEND_TIME_DATE_FROM_NTP):
		case (utils::SignalType::RTC_GET_LIGHT_ON_OFF_DATA):
		{
			mRTC->handleSignal(signal, data);
			break;
		}
		case (utils::SignalType::BTN_PRESS_BTN_1_2_COMBO_SIGNAL):
		{
			addControlMode(utils::CONTROL_MODE::SETTING_BUTTON_IR_REMOTE_START);
			break;
		}
		case (utils::SignalType::NETWORK_GET_TIME_DATE_FROM_NTP):
		{
			mNetwork->handleSignal(signal);
			break;
		}
		case (utils::SignalType::LIGHT_IS_ENABLE_BUTTON_LED):
		case (utils::SignalType::LIGHT_STATE_FROM_EEPROM):
		{
			mLIGHT->handleSignal(signal, data);
			break;
		}
		case (utils::SignalType::IR_ERRPROM_SEND_DATA):
		{
			mIR->handleSignal(signal, data);
			break;
		}
		case (utils::SignalType::EEPROM_STORE_LIGHT_IS_ENABLE_BUTTON_LED):
		case (utils::SignalType::EEPROM_NETWORK_SEND_SSID_PASSWORD):
		case (utils::SignalType::EEPROM_STORE_STATE_LIGHT):
		{
			mEEPROM->handleSignal(signal, data);
			break;
		}
		case (utils::SignalType::EEPROM_CLEAR_SSID_PASSOWRD_DATA):
		case (utils::SignalType::EEPROM_SEND_LIGHT_TIME_ON_OFF_DATA_TO_RTC):
		{
			mEEPROM->handleSignal(signal);
			break;
		}
		case (utils::SignalType::BTN_LONG_PRESS_SIGNAL):
		{
			addControlMode(utils::CONTROL_MODE::WIFI_PROVISIONING_START);
			break;
		}
		case (utils::SignalType::REMOTE_LIGHT_REMOVE_WIFI_PROVISIONING_DONE):
		{
			if (mQueue.front().getControlMode() == utils::CONTROL_MODE::WIFI_PROVISIONING_START && mQueue.front().getState() == utils::STATE::DELAY)
			{
				mQueue.front().setState(utils::STATE::DONE);
			}
			addControlMode(utils::CONTROL_MODE::DISPLAY_ALL);
			break;
		}
		case (utils::SignalType::REMOTE_LIGHT_DISPLAY_ALL_TIME_MODE_FINISH):
		{
			if (mQueue.front().getControlMode() == utils::CONTROL_MODE::DISPLAY_ALL)
			{
				LOGD("DISPLAY ALL TIME control mode is FINISH.");
				mQueue.front().setState(utils::STATE::DONE);
			}
			break;
		}
		case (utils::SignalType::IR_INSTALL_BUTTON_COMPLETE):
		{
			if (mQueue.front().getControlMode() == utils::CONTROL_MODE::SETTING_BUTTON_IR_REMOTE && mQueue.front().getState() == utils::STATE::DELAY)
			{
				mQueue.front().setState(utils::STATE::DONE);
			}
			addControlMode(utils::CONTROL_MODE::SETTING_BUTTON_IR_REMOTE_DONE);
			break;
		}
		case (utils::SignalType::REMOTE_LIGHT_SETTING_RTC_TIME_DONE):
		{
			if (mQueue.front().getControlMode() == utils::CONTROL_MODE::SETTING_RTC_TIME && mQueue.front().getState() == utils::STATE::DELAY)
			{
				mQueue.front().setState(utils::STATE::DONE);
			}
			addControlMode(utils::CONTROL_MODE::SETTING_RTC_TIME_DONE);
			break;
		}
		case (utils::SignalType::REMOTE_LIGHT_SETTING_LIGHT_TIME_CONFIG_DONE):
		{
			if (mQueue.front().getControlMode() == utils::CONTROL_MODE::SETTING_LIGHT_TIME_CONFIG && mQueue.front().getState() == utils::STATE::DELAY)
			{
				mQueue.front().setState(utils::STATE::DONE);
			}
			addControlMode(utils::CONTROL_MODE::DISPLAY_ALL);
			break;
		}
		case (utils::SignalType::REMOTE_LIGHT_REMOVE_WIFI_PROVISIONING_FAILED):
		{
			mNetwork->handleSignal(utils::SignalType::NETWORK_REMOVE_EVENT);
			if (mQueue.front().getControlMode() == utils::CONTROL_MODE::WIFI_PROVISIONING_START && mQueue.front().getState() == utils::STATE::DELAY)
			{
				mQueue.front().setState(utils::STATE::DONE);
			}
			addControlMode(utils::CONTROL_MODE::WIFI_PROVISIONING_FAILED);
			break;
		}
		case (utils::SignalType::IR_BTN_5_SIGNAL):
		{
			if (mQueue.size() == 0)
			{
				mLCD->handleSignal(utils::SignalType::LCD_TURN_ON_LIGHT);
				addControlMode(utils::CONTROL_MODE::DISPLAY_ALL);
			}
			else
			{
				LOGD("mQueue is not empty.");
			}
			break;
		}
		case (utils::SignalType::TASKS_SET_WIFI_ERROR):
		{
			mTasks->handleControlMode(utils::CONTROL_MODE::SET_WIFI_ERROR);
			break;
		}
		case (utils::SignalType::TASKS_SET_FIREBASE_ERROR):
		{
			mTasks->handleControlMode(utils::CONTROL_MODE::SET_FIREBASE_ERROR);
			break;
		}
		case (utils::SignalType::TASKS_SET_NTP_ERROR):
		{
			mTasks->handleControlMode(utils::CONTROL_MODE::SET_NTP_ERROR);
			break;
		}
		// case (utils::SignalType::REMOTE_LIGHT_ADD_TASK_CHECK_CONNECT_FIREBASE):
		// {
		// 	addControlMode(utils::CONTROL_MODE::CONNECT_FIREBASE);
		// 	mFlagReconnectFirebase = true;
		// 	break;
		// }
		default:
		{
			LOGW("Signal is not supported yet.");
			break;
		}
		}
	}

	void RemoteLight::onTimeout(const utils::TIMER_SIGNAL timer)
	{
		switch (timer)
		{
		case (utils::TIMER_SIGNAL::CONNECT_WIFI_SIGNAL):
		{
			if (mQueue.front().getControlMode() == utils::CONTROL_MODE::CONNECT_WIFI && mQueue.front().getState() == utils::STATE::DELAY)
			{
				mQueue.front().setState(utils::STATE::RUNNING);
			}
			break;
		}
		case (utils::TIMER_SIGNAL::CONNECT_WIFI_DONE):
		{
			if (mQueue.front().getControlMode() == utils::CONTROL_MODE::CONNECT_WIFI_DONE)
			{
				mQueue.front().setState(utils::STATE::DONE);
			}
			addControlMode(utils::CONTROL_MODE::CONNECT_FIREBASE);
			break;
		}
		case (utils::TIMER_SIGNAL::CONNECT_WIFI_FAILED_GOTO_NEXT_MODE):
		{
			if (mQueue.front().getControlMode() == utils::CONTROL_MODE::CONNECT_WIFI_FAILED)
			{
				mQueue.front().setState(utils::STATE::DONE);
			}
			addControlMode(utils::CONTROL_MODE::DISPLAY_ALL);
			break;
		}
		case (utils::TIMER_SIGNAL::CONNECT_FIREBASE_SIGNAL):
		{
			if (mQueue.front().getControlMode() == utils::CONTROL_MODE::CONNECT_FIREBASE && mQueue.front().getState() == utils::STATE::DELAY)
			{
				mQueue.front().setState(utils::STATE::RUNNING);
			}
			break;
		}
		case (utils::TIMER_SIGNAL::CONNECT_FIREBASE_DONE):
		{
			if (mQueue.front().getControlMode() == utils::CONTROL_MODE::CONNECT_FIREBASE_DONE)
			{
				mQueue.front().setState(utils::STATE::DONE);
			}
			addControlMode(utils::CONTROL_MODE::CONNECT_FIREBASE_GET_TOKEN);
			break;
		}
		case (utils::TIMER_SIGNAL::CONNECT_FIREBASE_GET_TOKEN):
		{
			if (mQueue.front().getControlMode() == utils::CONTROL_MODE::CONNECT_FIREBASE_GET_TOKEN)
			{
				mQueue.front().setState(utils::STATE::DONE);
			}
			if(mFlagReconnectFirebase != true)
			{
				addControlMode(utils::CONTROL_MODE::CONNECT_NTP);
			}
			break;
		}
		case (utils::TIMER_SIGNAL::CONNECT_FIREBASE_FAILED_GOTO_NEXT_CONNECT):
		{
			if (mQueue.front().getControlMode() == utils::CONTROL_MODE::CONNECT_FIREBASE_FAILED)
			{
				mQueue.front().setState(utils::STATE::DONE);
			}
			if(mFlagReconnectFirebase != true)
			{
				addControlMode(utils::CONTROL_MODE::CONNECT_NTP);
			}
			break;
		}
		case (utils::TIMER_SIGNAL::DISPLAY_ALL_TIME_SIGNAL):
		{
			if (mQueue.front().getControlMode() == utils::CONTROL_MODE::DISPLAY_ALL && mQueue.front().getState() == utils::STATE::DELAY)
			{
				mQueue.front().setState(utils::STATE::RUNNING);
			}
			break;
		}
		case (utils::TIMER_SIGNAL::CONNECT_NTP_SIGNAL):
		{
			if (mQueue.front().getControlMode() == utils::CONTROL_MODE::CONNECT_NTP && mQueue.front().getState() == utils::STATE::DELAY)
			{
				mQueue.front().setState(utils::STATE::RUNNING);
			}
			break;
		}
		case (utils::TIMER_SIGNAL::CONNECT_NTP_DONE):
		{
			if (mQueue.front().getControlMode() == utils::CONTROL_MODE::CONNECT_NTP_DONE)
			{
				LOGD("CONNECT_NTP_DONE control mode is DONE.");
				mQueue.front().setState(utils::STATE::DONE);
			}
			addControlMode(utils::CONTROL_MODE::DISPLAY_ALL);
			break;
		}
		case (utils::TIMER_SIGNAL::CONNECT_NTP_FAILED_GOTO_NEXT_CONNECT):
		{
			if (mQueue.front().getControlMode() == utils::CONTROL_MODE::CONNECT_NTP_FAILED)
			{
				mQueue.front().setState(utils::STATE::DONE);
			}
			addControlMode(utils::CONTROL_MODE::DISPLAY_ALL);
			break;
		}
		case (utils::TIMER_SIGNAL::SETTING_BUTTON_IR_REMOTE_START):
		{
			if (mQueue.front().getControlMode() == utils::CONTROL_MODE::SETTING_BUTTON_IR_REMOTE_START && mQueue.front().getState() == utils::STATE::DELAY)
			{
				LOGD("SETTING BUTTON IR REMOTE START control mode is TIMEOUT.");
				mQueue.front().setState(utils::STATE::DONE);
			}
			addControlMode(utils::CONTROL_MODE::SETTING_BUTTON_IR_REMOTE);
			break;
		}
		case (utils::TIMER_SIGNAL::SETTING_BUTTON_IR_REMOTE):
		{
			if (mQueue.front().getControlMode() == utils::CONTROL_MODE::SETTING_BUTTON_IR_REMOTE && mQueue.front().getState() == utils::STATE::DELAY)
			{
				LOGD("SETTING BUTTON IR REMOTE control mode is TIMEOUT.");
			}
			break;
		}
		case (utils::TIMER_SIGNAL::SETTING_BUTTON_IR_REMOTE_DONE):
		{
			if (mQueue.front().getControlMode() == utils::CONTROL_MODE::SETTING_BUTTON_IR_REMOTE_DONE && mQueue.front().getState() == utils::STATE::DELAY)
			{
				mQueue.front().setState(utils::STATE::DONE);
			}
			addControlMode(utils::CONTROL_MODE::DISPLAY_ALL);
			break;
		}
		case (utils::TIMER_SIGNAL::SETTING_RTC_TIME_START):
		{
			if (mQueue.front().getControlMode() == utils::CONTROL_MODE::SETTING_RTC_TIME_START && mQueue.front().getState() == utils::STATE::DELAY)
			{
				mQueue.front().setState(utils::STATE::DONE);
			}
			addControlMode(utils::CONTROL_MODE::SETTING_RTC_TIME);
			break;
		}
		case (utils::TIMER_SIGNAL::SETTING_RTC_TIME):
		{
			LOGW("SETTING_RTC_TIME timer timeout.");
			break;
		}
		case (utils::TIMER_SIGNAL::SETTING_RTC_TIME_DONE):
		{
			if (mQueue.front().getControlMode() == utils::CONTROL_MODE::SETTING_RTC_TIME_DONE && mQueue.front().getState() == utils::STATE::DELAY)
			{
				mQueue.front().setState(utils::STATE::DONE);
			}
			addControlMode(utils::CONTROL_MODE::DISPLAY_ALL);
			break;
		}
		case (utils::TIMER_SIGNAL::SETTING_LIGHT_TIME_CONFIG):
		{
			LOGD("SETTING_LIGHT_TIME_CONFIG timer timeout.");
			break;
		}
		case (utils::TIMER_SIGNAL::WIFI_PROVISIONING_START):
		{
			LOGD("WIFI_PROVISIONING_START timer timeout.");
			break;
		}
		case (utils::TIMER_SIGNAL::WIFI_PROVISIONING_FAILED):
		{
			if (mQueue.front().getControlMode() == utils::CONTROL_MODE::WIFI_PROVISIONING_FAILED && mQueue.front().getState() == utils::STATE::DELAY)
			{
				mQueue.front().setState(utils::STATE::DONE);
			}
			ESP.restart();
			break;
		}
		default:
			break;
		}
	}

	void RemoteLight::setLightScheduleCheckEnabled(const bool state)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		mIsLightScheduleChecking = state;
	}

	bool RemoteLight::isLightScheduleCheckEnabled()
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return mIsLightScheduleChecking;
	}

	void RemoteLight::process()
	{
		if (mQueue.empty())
		{
			return;
		}

		utils::StateControl &current = mQueue.front();

		switch (current.getState())
		{
		case (utils::STATE::IDLE):
		{
			current.setState(utils::STATE::RUNNING);
			break;
		}
		case (utils::STATE::RUNNING):
		{
			current.setState(utils::STATE::WAITING);
			mTasks->handleControlMode(current.getControlMode());
			break;
		}
		case (utils::STATE::PAUSED):
		{
			current.setState(utils::STATE::RUNNING);
			break;
		}
		case (utils::STATE::WAITING):
		{
			utils::TIMER_SIGNAL signal = current.getConfig().signal;
			uint16_t duration = current.getConfig().duration;
			// LOGD("%s is WAITING for %d ms.", utils::CONTROLMODETOSTRING(current.getControlMode()).c_str(), duration);
			current.setState(utils::STATE::DELAY);
			mTimer->updateTimer([this, signal]()
								{ this->onTimeout(signal); }, duration);
			mTimer->startTimer();
			// LOGD("mTIMER %d ms.", mTimer->getDuration());
			break;
		}
		case (utils::STATE::DONE):
		{
			// LOGD("%s is DONE.", utils::CONTROLMODETOSTRING(current.getControlMode()).c_str());
			mExists.erase(current.getControlMode());
			mQueue.pop_front();
			// LOGD("Remaining control modes in the queue: %d", mQueue.size());
			break;
		}
		default:
			break;
		}
	}

	void RemoteLight::addControlMode(utils::CONTROL_MODE mode)
	{
		utils::StateControl sc(mode);
		utils::INSERT_LEVEL level = sc.getConfig().level;
		if (mExists.count(mode))
		{
			return;
		}

		if (mQueue.empty())
		{
			mQueue.emplace_back(sc);
			mExists.insert(mode);
			return;
		}

		utils::StateConfig currentMode = mQueue.front().getConfig();
		utils::INSERT_LEVEL currentLevel = currentMode.level;
		if (level < currentLevel)
		{
			if (mQueue.front().getState() != utils::STATE::DONE)
			{
				mQueue.front().setState(utils::STATE::PAUSED);
			}
			mQueue.emplace_front(std::move(sc));
		}
		else if (level >= currentLevel)
		{
			utils::STATE currentState = mQueue.front().getState();
			if (currentState == utils::STATE::DONE)
			{
				mQueue.emplace_back(std::move(sc));
			}
			else
			{
				return;
			}
		}
		mExists.insert(mode);
	}

	void RemoteLight::handleUpdateData()
	{
		LOGD("mCounterCheckDataFirebase: %d, mCounterCheckDataNTP: %d, mCounterHandleError: %d", mCounterCheckDataFirebase, mCounterCheckDataNTP, mCounterHandleError);

		mCounterCheckDataNTP++;
		mCounterCheckDataFirebase++;
		mCounterHandleError++;

		// After 12 hours, it will check time from NTP server
		if (mCounterCheckDataNTP >= MAX_COUNTER_CHECK_DATA_NTP)
		{
			mTasks->handleControlMode(utils::CONTROL_MODE::CHECK_NTP_TIME_UPDATE);
			mCounterCheckDataNTP = 0U;
		}

		// After 1 day, it will check data from firebase server
		if (mCounterCheckDataFirebase >= MAX_COUNTER_CHECK_DATA_FIREBASE)
		{
			addControlMode(utils::CONTROL_MODE::CONNECT_FIREBASE);
			mFlagReconnectFirebase = true;
			mCounterCheckDataFirebase = 0U;
		}

		// After 3 days, if there is any errors, it will restart
		if (mCounterHandleError >= MAX_COUNTER_HANDLE_ERROR)
		{
			mTasks->handleControlMode(utils::CONTROL_MODE::HANDLE_ERROR);
			mCounterHandleError = 0U;
		}
	}
} // namespace remoteLight