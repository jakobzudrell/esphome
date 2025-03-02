#include <cstddef>
#include <cstdint>

#include "ens161_i2c.h"
#include "esphome/components/i2c/i2c.h"
#include "../ens161_base/ens161_base.h"

namespace esphome {
namespace ens161_i2c {

static const char *const TAG = "ens161_i2c.sensor";

bool ENS161I2CComponent::read_byte(uint8_t a_register, uint8_t *data) {
  return I2CDevice::read_byte(a_register, data);
};
bool ENS161I2CComponent::write_byte(uint8_t a_register, uint8_t data) {
  return I2CDevice::write_byte(a_register, data);
};
bool ENS161I2CComponent::read_bytes(uint8_t a_register, uint8_t *data, size_t len) {
  return I2CDevice::read_bytes(a_register, data, len);
};
bool ENS161I2CComponent::write_bytes(uint8_t a_register, uint8_t *data, size_t len) {
  return I2CDevice::write_bytes(a_register, data, len);
};

void ENS161I2CComponent::dump_config() {
  ENS161Component::dump_config();
  LOG_I2C_DEVICE(this);
}

}  // namespace ens161_i2c
}  // namespace esphome
