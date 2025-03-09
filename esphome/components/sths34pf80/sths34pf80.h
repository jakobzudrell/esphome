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
  void set_ambient_temperature_sensor(sensor::Sensor *ambient_temperature_sensor) {
    ambient_temperature_sensor_ = ambient_temperature_sensor;
  }
  void set_object_temperature_sensor(sensor::Sensor *object_temperature_sensor) {
    object_temperature_sensor_ = object_temperature_sensor;
  }

  void set_odr(uint8_t val) { odr_ = val; }
  void set_avg_t(uint8_t val) { avg_t_ = val; }
  void set_avg_tmos(uint8_t val) { avg_tmos_ = val; }
  void set_lpf_p_m(uint8_t val) { lpf_p_m_ = val; }
  void set_lpf_m(uint8_t val) { lpf_m_ = val; }
  void set_lpf_p(uint8_t val) { lpf_p_ = val; }
  void set_lpf_a_t(uint8_t val) { lpf_a_t_ = val; }
  void set_presence_threshold(uint16_t val) { presence_threshold_ = val; }
  void set_motion_threshold(uint16_t val) { motion_threshold_ = val; }
  void set_tamb_shock_threshold(uint16_t val) { tamb_shock_threshold_ = val; }
  void set_presence_hysteresis(uint8_t val) { presence_hysteresis_ = val; }
  void set_motion_hysteresis(uint8_t val) { motion_hysteresis_ = val; }
  void set_tamb_shock_hysteresis(uint8_t val) { tamb_shock_hysteresis_ = val; }

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
  uint8_t odr_;
  uint8_t avg_t_;
  uint8_t avg_tmos_;
  uint8_t lpf_p_m_;
  uint8_t lpf_m_;
  uint8_t lpf_p_;
  uint8_t lpf_a_t_;
  uint16_t presence_threshold_;
  uint16_t motion_threshold_;
  uint16_t tamb_shock_threshold_;
  uint8_t presence_hysteresis_;
  uint8_t motion_hysteresis_;
  uint8_t tamb_shock_hysteresis_;

  float sensitivity_;

  sensor::Sensor *presence_sensor_{nullptr};
  sensor::Sensor *motion_sensor_{nullptr};
  sensor::Sensor *ambient_temperature_sensor_{nullptr};
  sensor::Sensor *object_temperature_sensor_{nullptr};
};

}  // namespace sths34pf80
}  // namespace esphome
