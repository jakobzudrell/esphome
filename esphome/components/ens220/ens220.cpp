// ENS220 relative pressure and temperature sensor with I2C interface from ScioSense
//
// Datasheet: https://www.sciosense.com/wp-content/uploads/2021/01/ENS220.pdf
//
// Implementation based on:
//   https://github.com/maarten-pennings/ENS220
//   https://github.com/sciosense/ENS220_driver

#include "ens220.h"
#include "esphome/core/log.h"
#include "esphome/core/hal.h"

namespace esphome {
namespace ens220 {

static const char *const TAG = "ens220";

// ENS220 chip constants
static const uint8_t ENS220_BOOTING_MS = 5;  // Booting time in ms (also after reset, or going to high power)
static const uint8_t ENS220_SINGLE_MEASURMENT_CONVERSION_TIME_MS =
    50;                                         // Conversion time in ms for single shot T/H measurement
static const uint16_t ENS220_PART_ID = 0x0321;  // The expected part id of the ENS220

// Addresses of the ENS220 registers
static const uint8_t ENS220_REGISTER_PART_ID = 0x00;
static const uint8_t ENS220_REGISTER_UID = 0x02;
static const uint8_t ENS220_REGISTER_MODE_CFG = 0x06;
static const uint8_t ENS220_REGISTER_MEAS_CFG = 0x07;
static const uint8_t ENS220_REGISTER_STBY_CFG = 0x08;
static const uint8_t ENS220_REGISTER_DATA_STAT = 0x14;

void ENS220Component::setup() {
  ESP_LOGCONFIG(TAG, "Setting up ENS220...");
  uint8_t data[2];
  uint16_t part_id = 0;

  // Reset
  // Suggested sequence according to the datasheet
  if (!this->write_byte(ENS220_REGISTER_MODE_CFG, (1 << 3))) {
    this->error_code_ = WRITE_FAILED;
    this->mark_failed();
    return;
  }

  // Wait to boot after reset
  delay(ENS220_BOOTING_MS);

  // Enable high power mode

  // Must disable low power to read PART_ID
  if (!set_low_power_(false)) {
    // Try to go back to default mode (low power enabled)
    set_low_power_(true);
    this->error_code_ = COMMUNICATION_FAILED;
    this->mark_failed();
    return;
  }

  // Read the PART_ID
  if (!this->read_bytes(ENS220_REGISTER_PART_ID, data, 2)) {
    // Try to go back to default mode (low power enabled)
    set_low_power_(true);
    this->error_code_ = COMMUNICATION_FAILED;
    this->mark_failed();
    return;
  }

  // Pack bytes into partid
  part_id = data[1] * 256U + data[0] * 1U;
  // Check expected part id of the ENS220
  if (part_id != ENS220_PART_ID) {
    this->error_code_ = INVALID_ID;
    this->mark_failed();
  }
  // Set default power mode (low power enabled)
  set_low_power_(true);
}

void ENS220Component::dump_config() {
  ESP_LOGCONFIG(TAG, "ENS220:");
  LOG_I2C_DEVICE(this);
  if (this->is_failed()) {
    ESP_LOGE(TAG, "%s", LOG_STR_ARG(this->error_code_));
  }
  LOG_UPDATE_INTERVAL(this);
  LOG_SENSOR("  ", "Temperature", this->temperature_sensor_);
  LOG_SENSOR("  ", "Pressure", this->pressure_sensor_);
}

float ENS220Component::get_setup_priority() const { return setup_priority::DATA; }

void ENS220Component::update() {
  // Set to one-shot mode
  if (!this->write_byte(ENS220_REGISTER_STBY_CFG, (1 << 1))) {
    ESP_LOGE(TAG, "Setting one-shot mode failed!");
    this->status_set_warning();
    return;
  }

  // Start temperature and pressure measurement
  if (!this->write_byte(ENS220_REGISTER_MODE_CFG, ((1 << 4) | (1 << 1) | (1 << 0)))) {
    ESP_LOGE(TAG, "Trigger of measurement failed!");
    this->status_set_warning();
    return;
  }

  // Wait for measurement to complete
  this->set_timeout("data", uint32_t(ENS220_SINGLE_MEASURMENT_CONVERSION_TIME_MS), [this]() {
    int temperature_data, temperature_status, pressure_data, pressure_status;
    uint8_t data[5];
    uint32_t p_val_data, t_val_data;

    // Set default status for early bail out
    temperature_status = NONE;
    pressure_status = NONE;

    // Read PRESS_OUT and TEMP_OUT
    if (!this->read_bytes(0x17, data, 5)) {
      ESP_LOGE(TAG, "Communication with ENS220 failed!");
      this->status_set_warning();
      return;
    }

    // Pack bytes for pressure
    p_val_data = (uint32_t) ((uint32_t) data[2] << 16 | (uint32_t) data[1] << 8 | (uint32_t) data[0] << 0);
    pressure_data = (p_val_data >> 0) & 0xffffff;

    if (pressure_status == NONE) {
      if (this->pressure_sensor_ != nullptr) {
        float pressure = pressure_data / 64.0;
        this->pressure_sensor_->publish_state(pressure);
      }
    } else {
      ESP_LOGW(TAG, "Pressure status failure");
      this->status_set_warning();
      return;
    }

    // Pack bytes for temperature
    t_val_data = (uint32_t) ((uint32_t) data[4] << 8 | (uint32_t) data[3] << 0);
    temperature_data = (t_val_data >> 0) & 0xffffff;

    if (temperature_status == NONE) {
      if (this->temperature_sensor_ != nullptr) {
        // Temperature in Celsius
        float temperature = temperature_data / 128.0 - 27315L / 100.0;
        this->temperature_sensor_->publish_state(temperature);
      }
    } else {
      ESP_LOGW(TAG, "Temperature status failure");
    }
  });
}

// Sets ENS220 to low (true) or high (false) power. Returns false on I2C problems.
bool ENS220Component::set_low_power_(bool enable) {
  uint8_t low_power_cmd = enable ? 0x80 : 0x00;
  ESP_LOGD(TAG, "Enable low power: %s", enable ? "true" : "false");
  bool result = this->write_byte(ENS220_REGISTER_MODE_CFG, low_power_cmd);
  delay(ENS220_BOOTING_MS);
  return result;
}

}  // namespace ens220
}  // namespace esphome
