#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/i2c/i2c.h"

namespace esphome {
namespace ens220 {

/// This class implements support for the ENS220 pressure and temperature i2c sensor.
class ENS220Component : public PollingComponent, public i2c::I2CDevice {
 public:
  float get_setup_priority() const override;
  void dump_config() override;
  void setup() override;
  void update() override;

  void set_pressure_sensor(sensor::Sensor *pressure_sensor) { pressure_sensor_ = pressure_sensor; }
  void set_temperature_sensor(sensor::Sensor *temperature_sensor) { temperature_sensor_ = temperature_sensor; }

  enum ErrorCode {
    NONE = 0,
    COMMUNICATION_FAILED,
    INVALID_ID,
    VALIDITY_INVALID,
    READ_FAILED,
    WRITE_FAILED,
    STD_OPMODE_FAILED,
  } error_code_{NONE};

 protected:
  bool set_low_power_(bool enable);

  sensor::Sensor *temperature_sensor_{nullptr};
  sensor::Sensor *pressure_sensor_{nullptr};
};

}  // namespace ens220
}  // namespace esphome
