#ifndef RTCDEF_H
#define RTCDEF_H

#include <Arduino.h>
#include "HardwareBase.h"
namespace hardware
{
	constexpr uint8_t 		MAX_SECOND 		= 59;
	constexpr uint8_t 		MIN_SECOND 		= 0;
	constexpr uint8_t 		MAX_MINUTE 		= 59;
	constexpr uint8_t 		MIN_MINUTE 		= 0;
	constexpr uint8_t 		MAX_HOUR 		= 23;
	constexpr uint8_t 		MIN_HOUR 		= 0;
	constexpr uint32_t 		MAX_YEAR 		= 2050;
	constexpr uint32_t 		MIN_YEAR 		= 2000;
	constexpr uint8_t 		MAX_MONTH 		= 12;
	constexpr uint8_t 		MIN_MONTH 		= 0;
	constexpr uint8_t 		MAX_DATE 		= 31; // Date of month
	constexpr uint8_t 		MIN_DATE 		= 0; // Date of month
	constexpr uint8_t 		MAX_DAY 		= 7; // Date of week
	constexpr uint8_t 		MIN_DAY 		= 0; // Date of week
	constexpr bool 			INCREASE 		= true;
	constexpr bool		 	DECREASE 		= false;

	enum REGISTER_ADDRESS : uint8_t
	{
		REG_SEC = 0x00,
		REG_MIN,
		REG_HOUR,
		REG_DAY,
		REG_DATE,
		REG_MTH,
		REG_YEAR,
	};

	struct TimeDS1307
	{
		uint8_t second;
		uint8_t minute;
		uint8_t hour;
		uint8_t day;
		uint8_t date;
		uint8_t month;
		uint16_t year;
	};

	struct LightOfTime
	{
		uint8_t sw;
		uint8_t hour;
		uint8_t minute;
		uint8_t second;
	};

	enum class TimeField : uint8_t
	{
		Second = 0U,
		Minute,
		Hour,
		Year,
		Month,
		Date,
		Day
	};

	inline TimeField& operator++(TimeField& field)
	{
		if (field < TimeField::Day)
		{
			field = static_cast<TimeField>(static_cast<uint8_t>(field) + 1);
		}
		else if (field >= TimeField::Day)
		{
			field = TimeField::Second;
		}
		return field;
	}

	inline TimeField& operator--(TimeField& field)
	{
		if (field > TimeField::Second)
		{
			field = static_cast<TimeField>(static_cast<uint8_t>(field) - 1);
		}
		else if (field == TimeField::Second)
		{
			field = TimeField::Day;
		}
		return field;
	}

	enum class LightOfTimeField : uint8_t
	{
		swOn = 0U,
		hourOn,
		minuteOn,
		secondOn,
		swOff,
		hourOff,
		minuteOff,
		secondOff
	};

	inline LightOfTimeField& operator++(LightOfTimeField& field)
	{
		if (field < LightOfTimeField::secondOff)
		{
			field = static_cast<LightOfTimeField>(static_cast<uint8_t>(field) + 1);
		}
		else if (field >= LightOfTimeField::secondOff)
		{
			field = LightOfTimeField::swOn;
		}
		return field;
	}

	inline LightOfTimeField& operator--(LightOfTimeField& field)
	{
		if (field > LightOfTimeField::swOn)
		{
			field = static_cast<LightOfTimeField>(static_cast<uint8_t>(field) - 1);
		}
		else if (field == LightOfTimeField::swOn)
		{
			field = LightOfTimeField::secondOff;
		}
		return field;
	}

} // namespace hardware
#endif // RTCDEF_H