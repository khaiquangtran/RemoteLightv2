#ifndef REMOTE_LIGHT_H
#define REMOTE_LIGHT_H

#include <iostream>
#include <memory>
#include <mutex>
#include <deque>
#include <unordered_set>

#include "./Hardware/HardwareBase.h"
#include "./Hardware/RTC.h"
#include "./Hardware/LCD16x2.h"
#include "./Hardware/IRRemotePartner.h"
#include "./Hardware/Button.h"
#include "./Hardware/Light.h"
#include "./Hardware/SerialPartner.h"
#include "./Hardware/EEPROMPartner.h"
#include "./Timer/TimerMgr.h"
#include "./Tasks/Tasks.h"
#include "./Network/Network.h"
#include "./Utils/ConvertToString.h"
#include "./Utils/Define.h"
#include "./Utils/StateControl.h"

class Tasks;

constexpr uint32_t MAX_COUNTER_CHECK_DATA_NTP 		= 43'200U; 		// => 12 hours
constexpr uint32_t MAX_COUNTER_CHECK_DATA_FIREBASE 	= 86'400U; 		// => 1 day
constexpr uint32_t MAX_COUNTER_HANDLE_ERROR 		= 259'200U; 	// => 3 days
namespace remoteLight
{
	class RemoteLight : public std::enable_shared_from_this<RemoteLight>
	{
	public:
		RemoteLight();
		~RemoteLight();

		/**
		 * @brief Initialize all components of RemoteLight
		 */
		void init();

		/**
		 * @brief Main run loop for RemoteLight
		 */
		void run();

		/**
		 * @brief handle signal from other module
		 * @param signal The signal to be handled
		 * @param data Optional data associated with the signal
		 */
		void handleSignal(const utils::SignalType signal, utils::Package *data = nullptr);

	private:
		std::shared_ptr<hardware::HardwareBase> mSerial;
		std::shared_ptr<hardware::HardwareBase> mRTC;
		std::shared_ptr<hardware::HardwareBase> mIR;
		std::shared_ptr<hardware::HardwareBase> mLCD;
		std::shared_ptr<hardware::HardwareBase> mBTN;
		std::shared_ptr<hardware::HardwareBase> mLIGHT;
		std::shared_ptr<hardware::HardwareBase> mEEPROM;
		std::shared_ptr<Network> mNetwork;
		std::shared_ptr<Tasks> mTasks;

		std::shared_ptr<TimerManager> mTimerMgr;
		std::shared_ptr<Timer> mTimer;
		std::shared_ptr<Timer> mTimerOnOffLight;

		std::mutex mMutex;

		std::map<utils::SignalType, utils::CONTROL_MODE> mMapSettingRTCSignalToControlMode;
		std::map<utils::SignalType, utils::CONTROL_MODE> mMapSettingLightTimeConfigSignalToControlMode;

		std::deque<utils::StateControl> mQueue;
		std::unordered_set<utils::CONTROL_MODE> mExists;

		bool mFlagReconnectFirebase;
		bool mIsLightScheduleChecking;

		uint32_t mCounterCheckDataFirebase;
		uint32_t mCounterCheckDataNTP;
		uint32_t mCounterHandleError;

		/**
		 * @brief Process the current control mode in the queue
		 */
		void process();

		/**
		 * @brief Add a new control mode to the processing queue
		 * @param mode The control mode to be added
		 */
		void addControlMode(utils::CONTROL_MODE mode);

		/**
		 * @brief Update data counters and handle periodic tasks
		 */
		void handleUpdateData();

		/**
		 * @brief Set the flag for checking configured time for lights
		 * @param state The state to set the flag to
		 */
		void setLightScheduleCheckEnabled(const bool state = true);

		/**
		 * @brief Get the current state of the check configured time for lights flag
		 * @return The current state of the flag
		 */
		bool isLightScheduleCheckEnabled();

		/**
		 * @brief Handle timer timeout events
		 * @param signal The timer signal that triggered the timeout
		 */
		void onTimeout(const utils::TIMER_SIGNAL signal = utils::TIMER_SIGNAL::TIMER_SIGNAL_NONE);
	};
} // namespace remoteLight
#endif // ! REMOTE_LIGHT_H