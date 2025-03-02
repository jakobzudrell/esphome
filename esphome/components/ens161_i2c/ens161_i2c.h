#pragma once

#include "esphome/components/ens161_base/ens161_base.h"
#include "esphome/components/i2c/i2c.h"

namespace esphome {
namespace ens161_i2c {

class ENS161I2CComponent : public esphome::ens161_base::ENS161Component, public i2c::I2CDevice {
  void dump_config() override;

  bool read_byte(uint8_t a_register, uint8_t *data) override;
  bool write_byte(uint8_t a_register, uint8_t data) override;
  bool read_bytes(uint8_t a_register, uint8_t *data, size_t len) override;
  bool write_bytes(uint8_t a_register, uint8_t *data, size_t len) override;
};

}  // namespace ens161_i2c
}  // namespace esphome
