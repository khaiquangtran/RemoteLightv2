#include "./HardwareBase.h"

namespace hardware
{
	uint8_t HardwareBase::scanAddress(uint8_t addressInput)
	{
		uint8_t error, address;

		LOGI("Scanning address …");

		for (address = 1; address < 127; address++)
		{
			Wire.beginTransmission(address);
			error = Wire.endTransmission();

			if (error == 0)
			{
				if (address == addressInput)
				{
					LOGD("I2C found at address 0x%x", address);
					return address;
					break;
				}
			}
			else if (error == 4)
			{
				LOGE("Unknow error at address 0x%x", address);
				return INVALID;
			}
		}
		LOGD("Scanning done!!!");
		return INVALID;
	}
} // namespace hardware