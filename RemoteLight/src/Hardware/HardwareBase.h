#ifndef HARDWARE_H
#define HARDWARE_H

#include <map>
#include <Wire.h>
#include <Arduino.h>
#include <vector>
#include <memory>
#include "./../Utils/Logging.h"
#include "./../Utils/SignalType.h"
#include "./../Utils/Package.h"
#include "./../Utils/ConvertToString.h"
#include "./RTCDef.h"

namespace hardware
{
  constexpr uint8_t SD_CS       = 5U;
  constexpr uint8_t SPI_MOSI    = 23U;
  constexpr uint8_t SPI_MISO    = 19U;
  constexpr uint8_t SPI_SCK     = 18U;
  constexpr uint8_t SCL         = 22U;
  constexpr uint8_t SDA         = 21U;
  constexpr uint8_t BNT_1       = 5U;
  constexpr uint8_t BNT_2       = 18U;
  constexpr uint8_t LIGHT_1     = 25U;
  constexpr uint8_t LIGHT_2     = 26U;
  constexpr uint8_t LIGHT_3     = 27U;
  constexpr uint8_t LIGHT_4     = 14U;
  constexpr uint8_t LED_BTN_RST = 32U;
  constexpr uint8_t LED_BTN_1   = 33U;
  constexpr uint8_t LED_BTN_2   = 19U;

#define NOT_CONNECT_DEVICE 0
#define NOT_CONNECT_DEVICE_LCD 0

  class HardwareBase
  {
  public:
    HardwareBase() {};
    virtual ~HardwareBase() = default;

    virtual void handleSignal(const utils::SignalType &signal, const utils::Package *data = nullptr) = 0;

    virtual void listenning()
    {
      LOGI(".");
    };
    virtual void init()
    {
      LOGI(".");
    };

  protected:
    /**
     * @brief Scan I2C address to check if device is connected
     * @param addressInput The I2C address to scan
     */
    uint8_t scanAddress(uint8_t addressInput);

    const uint8_t INVALID = 0x80;
  };
}

#endif // HARDWARE_H