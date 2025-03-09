// STHS34PF80
#include "sths34pf80.h"
#include "esphome/core/log.h"
#include "esphome/core/hal.h"

namespace esphome {
namespace sths34pf80 {

static const char *const TAG = "sths34pf80";

// STHS34PF80 chip constants
static const uint8_t STHS34PF80_BOOTING_MS = 5;  // Booting time in ms (also after reset, or going to high power)

static const uint16_t STHS34PF80_PART_ID = 0xd3;  // The expected part id of the STHS34PF80

// Addresses of the STHS34PF80 registers
static const uint8_t STHS34PF80_REGISTER_LPF1 = 0x0c;
static const uint8_t STHS34PF80_REGISTER_LPF2 = 0x0d;

static const uint8_t STHS34PF80_REGISTER_SENS_DATA = 0x1d;
static const uint8_t STHS34PF80_REGISTER_AVG_TRIM = 0x10;

static const uint8_t STHS34PF80_REGISTER_CTRL0 = 0x17;
static const uint8_t STHS34PF80_REGISTER_CTRL1 = 0x20;
static const uint8_t STHS34PF80_REGISTER_CTRL2 = 0x21;
static const uint8_t STHS34PF80_REGISTER_CTRL3 = 0x22;

static const uint8_t STHS34PF80_REGISTER_STATUS = 0x23;
static const uint8_t STHS34PF80_REGISTER_FUNC_STATUS = 0x25;

static const uint8_t STHS34PF80_REGISTER_CFG_ADDR = 0x08;
static const uint8_t STHS34PF80_REGISTER_CFG_DATA = 0x09;
static const uint8_t STHS34PF80_REGISTER_PAGE_RW = 0x11;

static const uint8_t STHS34PF80_REGISTER_PRESENCE_THS_L = 0x20;
static const uint8_t STHS34PF80_REGISTER_PRESENCE_THS_H = 0x21;

static const uint8_t STHS34PF80_REGISTER_MOTION_THS_L = 0x22;
static const uint8_t STHS34PF80_REGISTER_MOTION_THS_H = 0x23;

static const uint8_t STHS34PF80_REGISTER_TAMB_SHOCK_THS_L = 0x24;
static const uint8_t STHS34PF80_REGISTER_TAMB_SHOCK_THS_H = 0x25;

static const uint8_t STHS34PF80_REGISTER_PRESENCE_HYST = 0x27;
static const uint8_t STHS34PF80_REGISTER_MOTION_HYST = 0x26;
static const uint8_t STHS34PF80_REGISTER_TAMB_SHOCK_HYST = 0x29;

static const uint8_t STHS34PF80_REGISTER_ALGO_CONF = 0x28;
static const uint8_t STHS34PF80_REGISTER_RESET_ALGO = 0x2a;

void STHS34PF80Component::setup() {
  ESP_LOGCONFIG(TAG, "Setting up STHS34PF80...");

  // Reboot
  if (!this->write_byte(STHS34PF80_REGISTER_CTRL2, 0x80)) {
    this->error_code_ = WRITE_FAILED;
    this->mark_failed();
    return;
  }

  delay(STHS34PF80_BOOTING_MS);

  // Enter power down mode according to AN5867
  uint8_t data[1];
  this->read_byte(STHS34PF80_REGISTER_FUNC_STATUS, data);

  // Wait until DRDY flag is cleared
  uint8_t counter = 0;
  while (counter < 100) {
    this->read_byte(STHS34PF80_REGISTER_STATUS, data);
    if (!(data[0] & 0x04)) {
      break;
    }
    counter++;
    delay(1);
  }

  if (!(data[0] & 0x04)) {
    // Power down
    this->write_byte(STHS34PF80_REGISTER_CTRL1, 0x00);
  } else {
    this->error_code_ = DATA_CLEARED_TIMEOUT;
    this->mark_failed();
    return;
  }

  // Read func status to clear DRDY
  this->read_byte(STHS34PF80_REGISTER_FUNC_STATUS, data);

  //// LOW PASS FILTER
  uint8_t LPF_P_M = 0x00;  // ODR/9
  uint8_t LPF_M = 0x04;    // ODR/200
  uint8_t LPF_P = 0x04;    // ODR/200
  uint8_t LPF_A_T = 0x02;  // ODR/50

  // LPF1
  this->write_byte(STHS34PF80_REGISTER_LPF1, (lpf_p_m_ << 3) | lpf_m_);

  // LPF2
  this->write_byte(STHS34PF80_REGISTER_LPF2, (lpf_p_ << 3) | lpf_a_t_);

  //// AVG TRIM
  uint8_t AVG_T = 0x00;
  uint8_t AVG_TMOS = 0x03;
  this->write_byte(STHS34PF80_REGISTER_AVG_TRIM, (AVG_T << 4 | AVG_TMOS));

  //// GAIN
  this->write_byte(STHS34PF80_REGISTER_CTRL0, (0x07 << 4) | 0x81);

  //// DISABLE INTERRUPTS
  this->write_byte(STHS34PF80_REGISTER_CTRL3, 0x00);

  //// CONFIGURE ALGORITHMS
  uint8_t PRESENCE_THRESHOLD = 100;      // default 200
  uint8_t MOTION_THRESHOLD = 200;        // default 200
  uint16_t TAMB_SHOCK_THRESHOLD = 2000;  // default 200

  uint8_t PRESENCE_HYST = 32;
  uint8_t MOTION_HYST = 32;
  uint8_t TAMB_SHOCK_HYST = 2;

  this->write_byte(STHS34PF80_REGISTER_CTRL2, 0x10);    // enable access to embedded functions
  this->write_byte(STHS34PF80_REGISTER_PAGE_RW, 0x40);  // enable write access to embedded functions

  this->write_byte(STHS34PF80_REGISTER_CFG_ADDR, 0x20);
  this->write_byte(STHS34PF80_REGISTER_CFG_DATA, presence_threshold_ & 0x00ff);  // lower byte
  this->write_byte(STHS34PF80_REGISTER_CFG_ADDR, 0x21);
  this->write_byte(STHS34PF80_REGISTER_CFG_DATA, presence_threshold_ >> 8);  // upper byte

  this->write_byte(STHS34PF80_REGISTER_CFG_ADDR, 0x22);
  this->write_byte(STHS34PF80_REGISTER_CFG_DATA, motion_threshold_ & 0x00ff);  // lower byte
  this->write_byte(STHS34PF80_REGISTER_CFG_ADDR, 0x23);
  this->write_byte(STHS34PF80_REGISTER_CFG_DATA, motion_threshold_ >> 8);  // upper byte

  this->write_byte(STHS34PF80_REGISTER_CFG_ADDR, 0x24);
  this->write_byte(STHS34PF80_REGISTER_CFG_DATA, tamb_shock_threshold_ & 0x00ff);  // lower byte
  this->write_byte(STHS34PF80_REGISTER_CFG_ADDR, 0x25);
  this->write_byte(STHS34PF80_REGISTER_CFG_DATA, tamb_shock_threshold_ >> 8);  // upper byte

  this->write_byte(STHS34PF80_REGISTER_CFG_ADDR, 0x27);
  this->write_byte(STHS34PF80_REGISTER_CFG_DATA, presence_hysteresis_);

  this->write_byte(STHS34PF80_REGISTER_CFG_ADDR, 0x26);
  this->write_byte(STHS34PF80_REGISTER_CFG_DATA, motion_hysteresis_);

  this->write_byte(STHS34PF80_REGISTER_CFG_ADDR, 0x29);
  this->write_byte(STHS34PF80_REGISTER_CFG_DATA, tamb_shock_hysteresis_);

  this->write_byte(STHS34PF80_REGISTER_CFG_ADDR, 0x28);
  this->write_byte(STHS34PF80_REGISTER_CFG_DATA, 0x04);  // TEMP_COMP

  this->write_byte(STHS34PF80_REGISTER_CFG_ADDR, 0x2a);
  this->write_byte(STHS34PF80_REGISTER_CFG_DATA, 0x01);  // RESET ALGORITHM

  this->write_byte(STHS34PF80_REGISTER_PAGE_RW, 0x00);  // disable write access to embedded functions
  this->write_byte(STHS34PF80_REGISTER_CTRL2, 0x00);    // disable access to embedded functions

  //// POWER UP
  this->write_byte(STHS34PF80_REGISTER_CTRL1, 0x05);  // ODR 4Hz
}

void STHS34PF80Component::dump_config() {
  ESP_LOGCONFIG(TAG, "STHS34PF80:");
  LOG_I2C_DEVICE(this);
  if (this->is_failed()) {
    ESP_LOGE(TAG, "%s", LOG_STR_ARG(this->error_code_));
  }

  // To read the algorithm configuration
  uint8_t data;
  this->read_byte(STHS34PF80_REGISTER_LPF1, &data);
  ESP_LOGCONFIG(TAG, "  lpf_p_m: %d (actual: %d)", this->lpf_p_m_, (data & 0x38) >> 3);
  this->read_byte(STHS34PF80_REGISTER_LPF1, &data);
  ESP_LOGCONFIG(TAG, "  lpf_m: %d (actual: %d)", this->lpf_m_, (data & 0x07));

  this->read_byte(STHS34PF80_REGISTER_LPF2, &data);
  ESP_LOGCONFIG(TAG, "  lpf_p: %d (actual: %d)", this->lpf_p_, (data & 0x38) >> 3);
  this->read_byte(STHS34PF80_REGISTER_LPF2, &data);
  ESP_LOGCONFIG(TAG, "  lpf_a_t: %d (actual: %d)", this->lpf_a_t_, (data & 0x07));

  uint16_t data;
  this->read_byte(STHS34PF80_REGISTER_PRESENCE_THS_L, reinterpret_cast<uint8_t *>(&data, 2));
  ESP_LOGCONFIG(TAG, "  presence_threshold: %d (actual: %d)", this->presence_threshold_, data);

  this->read_byte(STHS34PF80_REGISTER_MOTION_THS_L, reinterpret_cast<uint8_t *>(&data, 2));
  ESP_LOGCONFIG(TAG, "  motion_threshold: %d (actual: %d)", this->motion_threshold_, data);

  this->read_byte(STHS34PF80_REGISTER_TAMB_SHOCK_THS_L, reinterpret_cast<uint8_t *>(&data, 2));
  ESP_LOGCONFIG(TAG, "  tamb_shock_threshold: %d (actual: %d)", this->tamb_shock_threshold_, data);

  ESP_LOGCONFIG(TAG, "  presence_hysteresis: %d (actual: %d)", this->presence_hysteresis_, 0x00);
  ESP_LOGCONFIG(TAG, "  motion_hysteresis: %d (actual: %d)", this->motion_hysteresis_, 0x00);
  ESP_LOGCONFIG(TAG, "  tamb_shock_hysteresis: %d (actual: %d)", this->tamb_shock_hysteresis_, 0x00);

  LOG_UPDATE_INTERVAL(this);
  LOG_SENSOR("  ", "Presence", this->presence_sensor_);
  LOG_SENSOR("  ", "Motion", this->motion_sensor_);
}

float STHS34PF80Component::get_setup_priority() const { return setup_priority::DATA; }

void STHS34PF80Component::update() {
  uint8_t data[1];

  // read data ready flag
  if (!this->read_byte(STHS34PF80_REGISTER_STATUS, data)) {
    ESP_LOGE(TAG, "Reading data ready flag failed!");
    this->status_set_warning();
    return;
  }

  // check if data is ready
  if (data[0] & (1 << 2)) {
    // read function flags
    uint8_t flags[1];
    if (!this->read_byte(STHS34PF80_REGISTER_FUNC_STATUS, flags)) {
      ESP_LOGE(TAG, "Reading function status flags failed!");
      this->status_set_warning();
      return;
    }

    if (flags[0] & (1 << 2)) {
      this->presence_sensor_->publish_state(1);
    } else {
      this->presence_sensor_->publish_state(0);
    }

    if (flags[0] & (1 << 1)) {
      this->motion_sensor_->publish_state(1);
    } else {
      this->motion_sensor_->publish_state(0);
    }
  }
}
}  // namespace sths34pf80
}  // namespace esphome
