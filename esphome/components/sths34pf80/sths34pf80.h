#pragma once

#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/i2c/i2c.h"

namespace esphome {
namespace sths34pf80 {

/// This class implements support for the sths34pf80 i2c sensor.
class STHS34PF80Component : public PollingComponent, public i2c::I2CDevice {
 public:
  float get_setup_priority() const override;
  void dump_config() override;
  void setup() override;
  void update() override;

  void set_presence_sensor(sensor::Sensor *presence_sensor) { presence_sensor_ = presence_sensor; }
  void set_motion_sensor(sensor::Sensor *motion_sensor) { motion_sensor_ = motion_sensor; }

  enum ErrorCode {
    NONE = 0,
    DATA_CLEARED_TIMEOUT,
    COMMUNICATION_FAILED,
    INVALID_ID,
    VALIDITY_INVALID,
    READ_FAILED,
    WRITE_FAILED,
    STD_OPMODE_FAILED,
  } error_code_{NONE};

 protected:
  sensor::Sensor *presence_sensor_{nullptr};
  sensor::Sensor *motion_sensor_{nullptr};
};

}  // namespace sths34pf80
}  // namespace esphome
