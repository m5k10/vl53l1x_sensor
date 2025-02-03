#include "esphome/core/log.h"
#include "vl53l1x_sensor.h"

namespace esphome {
namespace vl53l1x {

static const char *const TAG = "vl53l1x";

std::list<VL53L1XSensor *> VL53L1XSensor::vl53_sensors;  // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
bool VL53L1XSensor::enable_pin_setup_complete = false;   // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
static const uint32_t TimingGuard = 4528;

const uint8_t VL51L1X_DEFAULT_CONFIGURATION[] =
{
   0x00, /* 0x2d : set bit 2 and 5 to 1 for fast plus mode (1MHz I2C), else don't touch */
   0x00, /* 0x2e : bit 0 if I2C pulled up at 1.8V, else set bit 0 to 1 (pull up at AVDD) */
   0x00, /* 0x2f : bit 0 if GPIO pulled up at 1.8V, else set bit 0 to 1 (pull up at AVDD) */
   0x01, /* 0x30 : set bit 4 to 0 for active high interrupt and 1 for active low (bits 3:0 must be 0x1), use SetInterruptPolarity() */
   0x02, /* 0x31 : bit 1 = interrupt depending on the polarity, use CheckForDataReady() */
   0x00, /* 0x32 : not user-modifiable */
   0x02, /* 0x33 : not user-modifiable */
   0x08, /* 0x34 : not user-modifiable */
   0x00, /* 0x35 : not user-modifiable */
   0x08, /* 0x36 : not user-modifiable */
   0x10, /* 0x37 : not user-modifiable */
   0x01, /* 0x38 : not user-modifiable */
   0x01, /* 0x39 : not user-modifiable */
   0x00, /* 0x3a : not user-modifiable */
   0x00, /* 0x3b : not user-modifiable */
   0x00, /* 0x3c : not user-modifiable */
   0x00, /* 0x3d : not user-modifiable */
   0xff, /* 0x3e : not user-modifiable */
   0x00, /* 0x3f : not user-modifiable */
   0x0F, /* 0x40 : not user-modifiable */
   0x00, /* 0x41 : not user-modifiable */
   0x00, /* 0x42 : not user-modifiable */
   0x00, /* 0x43 : not user-modifiable */
   0x00, /* 0x44 : not user-modifiable */
   0x00, /* 0x45 : not user-modifiable */
   0x20, /* 0x46 : interrupt configuration 0->level low detection, 1-> level high, 2-> Out of window, 3->In window, 0x20-> New sample ready , TBC */
   0x0b, /* 0x47 : not user-modifiable */
   0x00, /* 0x48 : not user-modifiable */
   0x00, /* 0x49 : not user-modifiable */
   0x02, /* 0x4a : not user-modifiable */
   0x0a, /* 0x4b : not user-modifiable */
   0x21, /* 0x4c : not user-modifiable */
   0x00, /* 0x4d : not user-modifiable */
   0x00, /* 0x4e : not user-modifiable */
   0x05, /* 0x4f : not user-modifiable */
   0x00, /* 0x50 : not user-modifiable */
   0x00, /* 0x51 : not user-modifiable */
   0x00, /* 0x52 : not user-modifiable */
   0x00, /* 0x53 : not user-modifiable */
   0xc8, /* 0x54 : not user-modifiable */
   0x00, /* 0x55 : not user-modifiable */
   0x00, /* 0x56 : not user-modifiable */
   0x38, /* 0x57 : not user-modifiable */
   0xff, /* 0x58 : not user-modifiable */
   0x01, /* 0x59 : not user-modifiable */
   0x00, /* 0x5a : not user-modifiable */
   0x08, /* 0x5b : not user-modifiable */
   0x00, /* 0x5c : not user-modifiable */
   0x00, /* 0x5d : not user-modifiable */
   0x01, /* 0x5e : not user-modifiable */
   0xcc, /* 0x5f : not user-modifiable */
   0x0f, /* 0x60 : not user-modifiable */
   0x01, /* 0x61 : not user-modifiable */
   0xf1, /* 0x62 : not user-modifiable */
   0x0d, /* 0x63 : not user-modifiable */
   0x01, /* 0x64 : Sigma threshold MSB (mm in 14.2 format for MSB+LSB), use SetSigmaThreshold(), default value 90 mm  */
   0x68, /* 0x65 : Sigma threshold LSB */
   0x00, /* 0x66 : Min count Rate MSB (MCPS in 9.7 format for MSB+LSB), use SetSignalThreshold() */
   0x80, /* 0x67 : Min count Rate LSB */
   0x08, /* 0x68 : not user-modifiable */
   0xb8, /* 0x69 : not user-modifiable */
   0x00, /* 0x6a : not user-modifiable */
   0x00, /* 0x6b : not user-modifiable */
   0x00, /* 0x6c : Intermeasurement period MSB, 32 bits register, use SetIntermeasurementInMs() */
   0x00, /* 0x6d : Intermeasurement period */
   0x0f, /* 0x6e : Intermeasurement period */
   0x89, /* 0x6f : Intermeasurement period LSB */
   0x00, /* 0x70 : not user-modifiable */
   0x00, /* 0x71 : not user-modifiable */
   0x00, /* 0x72 : distance threshold high MSB (in mm, MSB+LSB), use SetD:tanceThreshold() */
   0x00, /* 0x73 : distance threshold high LSB */
   0x00, /* 0x74 : distance threshold low MSB ( in mm, MSB+LSB), use SetD:tanceThreshold() */
   0x00, /* 0x75 : distance threshold low LSB */
   0x00, /* 0x76 : not user-modifiable */
   0x01, /* 0x77 : not user-modifiable */
   0x0f, /* 0x78 : not user-modifiable */
   0x0d, /* 0x79 : not user-modifiable */
   0x0e, /* 0x7a : not user-modifiable */
   0x0e, /* 0x7b : not user-modifiable */
   0x00, /* 0x7c : not user-modifiable */
   0x00, /* 0x7d : not user-modifiable */
   0x02, /* 0x7e : not user-modifiable */
   0xc7, /* 0x7f : ROI center, use SetROI() */
   0xff, /* 0x80 : XY ROI (X=Width, Y=Height), use SetROI() */
   0x9B, /* 0x81 : not user-modifiable */
   0x00, /* 0x82 : not user-modifiable */
   0x00, /* 0x83 : not user-modifiable */
   0x00, /* 0x84 : not user-modifiable */
   0x01, /* 0x85 : not user-modifiable */
   0x00, /* 0x86 : clear interrupt, use ClearInterrupt() */
   0x00  /* 0x87 : start ranging, use StartRanging() or StopRanging(), If you want an automatic start after VL53L1X_init() call, put 0x40 in location 0x87 */
};

VL53L1XSensor::VL53L1XSensor() { VL53L1XSensor::vl53_sensors.push_back(this); }

void VL53L1XSensor::setup() {
    ESP_LOGD(TAG, "'%s' - setup BEGIN", this->name_.c_str());

    if (!esphome::vl53l1x::VL53L1XSensor::enable_pin_setup_complete) {
        for (auto &vl53_sensor : vl53_sensors) {
            if (vl53_sensor->enable_pin_ != nullptr) {
                // Set enable pin as OUTPUT and disable the enable pin to force vl53 to HW Standby mode
                vl53_sensor->enable_pin_->setup();
                vl53_sensor->enable_pin_->digital_write(true);
                vl53_sensor->enable_pin_->digital_write(false);
            }
        }
        esphome::vl53l1x::VL53L1XSensor::enable_pin_setup_complete = true;
        delay(5);
    }

    if (this->enable_pin_ != nullptr) {
        this->enable_pin_->digital_write(true);
        delay(5);
    }

    uint8_t final_address = address_;
    ESP_LOGD(TAG, "'%s' - set addr 0x%02X", this->name_.c_str(), final_address);
    //InitSensor
    this->set_i2c_address(0x29);

    {
        uint8_t byteData;
        uint8_t wordData;
        byteData = reg16(0x010F).get(); // VL53L1_IDENTIFICATION__MODEL_ID
        ESP_LOGD(TAG, "VL53L1X Model_ID: 0x%02X", byteData);
        byteData = reg16(0x0110).get(); // VL53L1_IDENTIFICATION__MODULE_TYPE
        ESP_LOGD(TAG, "VL53L1X Module_Type: 0x%02X", byteData);
        wordData = readWord(0x010F); // VL53L1_IDENTIFICATION__MODEL_ID
        ESP_LOGD(TAG, "VL53L1X: 0x%04X", wordData);
    }

    this->timeout_start_us_ = micros();
    while (reg16(0x00E5).get() == 0x00) {
        if (this->timeout_us_ > 0 && ((uint16_t) (micros() - this->timeout_start_us_) > this->timeout_us_)) {
            ESP_LOGE(TAG, "'%s' - setup timeout", this->name_.c_str());
            this->mark_failed();
            return;
        }
        yield();
    }
    ESP_LOGD(TAG, "'%s' - SensorInit", this->name_.c_str());
    // SensorInit
    uint8_t Addr = 0x00;
    for (Addr = 0x2D; Addr <= 0x87; Addr++) {
        reg16(Addr) = VL51L1X_DEFAULT_CONFIGURATION[Addr - 0x2D];
    }
    ESP_LOGD(TAG, "'%s' - startRanging", this->name_.c_str());
    startRanging();
    bool ready = 0;
    ESP_LOGD(TAG, "'%s' - checkForDataReady", this->name_.c_str());
    while(ready==0) {
        ready = checkForDataReady();
    }
    ESP_LOGD(TAG, "'%s' - clearInterrupt", this->name_.c_str());
    clearInterrupt();
    ESP_LOGD(TAG, "'%s' - stopRanging", this->name_.c_str());
    stopRanging();
    ESP_LOGD(TAG, "'%s' - 0x0008=0x09 0x000b=0x00", this->name_.c_str());
    reg16(0x0008) = 0x09;
    reg16(0x000b) = 0x00;
    uint16_t sensor_id = sensorId();
    if(sensor_id != 0xEACC) {
        ESP_LOGE(TAG,"Wrong sensor id for '%s': 0x%04X", this->name_.c_str(), sensor_id);
        this->mark_failed();
        return;
    }
    set_distance_mode();
    set_measurement_timing_budget();
    set_signal_threshold();
    set_roi();

    // Set the sensor to the desired final address
    reg16(0x0001) = final_address & 0x7F;
    this->set_i2c_address(final_address);

    ESP_LOGI(TAG,"'%s' - Setup completed", this->name_.c_str());
    startRanging();
}

void VL53L1XSensor::loop() {
  
}

void VL53L1XSensor::update() {
    if (checkForDataReady()) {
        int16_t distance_mm = distance();
        if (this->ambient_rate_sensor != nullptr) {
          int16_t ambient_rate_mcps = ambientRate();
          this->ambient_rate_sensor->publish_state(ambient_rate_mcps);
        }
        if (this->avg_signal_rate_sensor != nullptr) {
          int16_t avg_signal_rate_mcps = avgSignalRate();
          this->avg_signal_rate_sensor->publish_state(avg_signal_rate_mcps);
        }
        if (this->peak_signal_rate_sensor != nullptr) {
          int16_t peak_signal_rate_mcps = peakSignalRate();
          this->peak_signal_rate_sensor->publish_state(peak_signal_rate_mcps);
        }
        if (distance_mm == -1) {
          // something went wrong!
          ESP_LOGD(TAG, "'%s' - Couldn't get distance: 0x%02X", this->name_.c_str(), rangeStatus);
          this->publish_state(NAN);
        } else {
          float distance_m = (float)distance_mm / 1000.0;
          ESP_LOGD(TAG, "'%s' - Got distance %i mm", this->name_.c_str(), distance_mm);
          this->publish_state(distance_m);
        }
    } else {
        ESP_LOGD(TAG, "'%s' - data not ready", this->name_.c_str());
    }
}

void VL53L1XSensor::dump_config() {
    LOG_SENSOR("", "VL53L1X", this);
    LOG_UPDATE_INTERVAL(this);
    LOG_I2C_DEVICE(this);
    if (this->enable_pin_ != nullptr) {
        LOG_PIN("  Enable Pin: ", this->enable_pin_);
    }
    if (this->irq_pin_ != nullptr) {
        LOG_PIN("  IRQ Pin: ", this->irq_pin_);
    }
    ESP_LOGCONFIG(TAG, "  Timeout: %u%s", this->timeout_us_, this->timeout_us_ > 0 ? "µs" : " (no timeout)");
    ESP_LOGCONFIG(TAG, "  Distance mode: %u", this->distance_mode_);
    ESP_LOGCONFIG(TAG, "  Timing budget: %uµs", this->timing_budget_us_);
    ESP_LOGCONFIG(TAG, "  Signal threshold: %u", this->signal_threshold_);
    ESP_LOGCONFIG(TAG, "  ROI center SPAD: %u", this->roi_center_);
    ESP_LOGCONFIG(TAG, "  ROI size: x=%u, y=%u", this->roi_size_x_, this->roi_size_y_);
}

void VL53L1XSensor::startRanging() {
    reg16(0x0087) = 0x40;
}

void VL53L1XSensor::stopRanging() {
    reg16(0x0087) = 0x00;
}

bool VL53L1XSensor::checkForDataReady() {
    uint8_t intPol = getInterruptPolarity();
    uint8_t temp = reg16(0x0031).get();
    /* Read in the register to check if a new value is available */
    if ((temp & 1) == intPol)
        return 1;
    else
        return 0;
}

uint8_t VL53L1XSensor::getInterruptPolarity() {
    uint8_t temp = reg16(0x0030).get();
    temp = temp & 0x10;
    return !(temp>>4);
}

void VL53L1XSensor::clearInterrupt() {
    reg16(0x0086) = 0x01;
}

uint16_t VL53L1XSensor::sensorId() {
   return readWord(0x010F);
}

uint16_t VL53L1XSensor::readWord(uint16_t reg_idx) {
    uint16_t buffer[2] = {0,0};
    buffer[0] = reg16(reg_idx).get();
    buffer[1] = reg16(reg_idx + 1).get();
    ESP_LOGD(TAG, "'%s' - 0x%04X = {0x%02X 0x%02X}", this->name_.c_str(), reg_idx, buffer[0], buffer[1]);
    uint16_t data = (buffer[0] << 8) + buffer[1];
    return data;
}

void VL53L1XSensor::writeWord(uint16_t reg_idx, uint16_t data) {
    reg16(reg_idx) = data >> 8;
    reg16(reg_idx + 1) = (uint8_t)data;
}

void VL53L1XSensor::setI2CAddress(uint8_t addr) {
    reg16(0x0001) = addr;
    this->set_i2c_address(addr);
}

int16_t VL53L1XSensor::distance() {
    rangeStatus = getRangeStatus();
    if (rangeStatus != 0x0) {
        return -1;
    }
    uint16_t distance = readWord(0x0096);
    return (int16_t)distance;
}

int16_t VL53L1XSensor::ambientRate() {
    uint16_t ambientRate = readWord(0x0090);
    return (int16_t)ambientRate;
}

int16_t VL53L1XSensor::avgSignalRate() {
    uint16_t avgSignalRate = readWord(0x009E);
    return (int16_t)avgSignalRate;
}

int16_t VL53L1XSensor::peakSignalRate() {
    uint16_t peakSignalRate = readWord(0x0098);
    return (int16_t)peakSignalRate;
}

int8_t VL53L1XSensor::getRangeStatus() {
    uint8_t RgSt = reg16(0x0089).get();
    RgSt = RgSt&0x1F;
    switch (RgSt)
    {
    case 9:
        RgSt = 0;
        break;
    case 6:
        RgSt = 1;
        break;
    case 4:
        RgSt = 2;
        break;
    case 8:
        RgSt = 3;
        break;
    case 5:
        RgSt = 4;
        break;
    case 3:
        RgSt = 5;
        break;
    case 19:
        RgSt = 6;
        break;
    case 7:
        RgSt = 7;
        break;
    case 12:
        RgSt = 9;
        break;
    case 18:
        RgSt = 10;
      break;
    case 22:
        RgSt = 11;
        break;
    case 23:
        RgSt = 12;
        break;
    case 13:
        RgSt = 13;
        break;
    default:
        RgSt = 255;
        break;
    }
    return RgSt;
}

void VL53L1XSensor::set_distance_mode() {
    switch (distance_mode_) {
    case Short:
        reg16(0x004B) = 0x14;
        reg16(0x0060) = 0x07;
        reg16(0x0063) = 0x05;
        reg16(0x0069) = 0x38;
        writeWord(0x0078, 0x0705);
        writeWord(0x007A, 0x0606);
        break;
    case Medium:
        reg16(0x004B) = 0x0D;
        reg16(0x0060) = 0x0B;
        reg16(0x0063) = 0x09;
        reg16(0x0069) = 0x78;
        writeWord(0x0078, 0x0B09);
        writeWord(0x007A, 0x0A0A);
        break;
    case Long:
        reg16(0x004B) = 0x0A;
        reg16(0x0060) = 0x0F;
        reg16(0x0063) = 0x0D;
        reg16(0x0069) = 0xB8;
        writeWord(0x0078, 0x0F0D);
        writeWord(0x007A, 0x0E0E);
        break;
    default:
        break;
    }
}

uint32_t VL53L1XSensor::calc_macro_period(uint8_t vcsel_period)
{
  // from VL53L1_calc_pll_period_us()
  // fast osc frequency in 4.12 format; PLL period in 0.24 format
  uint16_t fast_osc_frequency = reg16(0x0006).get();
  uint32_t pll_period_us = ((uint32_t)0x01 << 30) / fast_osc_frequency;

  // from VL53L1_decode_vcsel_period()
  uint8_t vcsel_period_pclks = (vcsel_period + 1) << 1;

  // VL53L1_MACRO_PERIOD_VCSEL_PERIODS = 2304
  uint32_t macro_period_us = (uint32_t)2304 * pll_period_us;
  macro_period_us >>= 6;
  macro_period_us *= vcsel_period_pclks;
  macro_period_us >>= 6;

  return macro_period_us;
}

void VL53L1XSensor::set_measurement_timing_budget()
{
    // assumes PresetMode is LOWPOWER_AUTONOMOUS

    if (timing_budget_us_ <= TimingGuard) {
        ESP_LOGE(TAG, "'%s' - invalid timing budget: %iµs (distance mode: %i)", this->name_.c_str(), timing_budget_us_);
        return;
    }

    uint32_t range_config_timeout_us = timing_budget_us_ -= TimingGuard;
    if (range_config_timeout_us > 1100000) {
        ESP_LOGE(TAG, "'%s' - invalid timing budget: %iµs (distance mode: %i)", this->name_.c_str(), timing_budget_us_);
        return; // FDA_MAX_TIMING_BUDGET_US * 2
    }

    range_config_timeout_us /= 2;

    // VL53L1_calc_timeout_register_values() begin

    uint32_t macro_period_us;

    // "Update Macro Period for Range A VCSEL Period"
    macro_period_us = calc_macro_period(reg(0x0060).get());

    // "Update Phase timeout - uses Timing A"
    // Timeout of 1000 is tuning parm default (TIMED_PHASECAL_CONFIG_TIMEOUT_US_DEFAULT)
    // via VL53L1_get_preset_mode_timing_cfg().
    uint32_t phasecal_timeout_mclks = timeout_microseconds_to_mclks(1000, macro_period_us);
    if (phasecal_timeout_mclks > 0xFF) { phasecal_timeout_mclks = 0xFF; }
    reg16(0x004B) = phasecal_timeout_mclks;

    // "Update MM Timing A timeout"
    // Timeout of 1 is tuning parm default (LOWPOWERAUTO_MM_CONFIG_TIMEOUT_US_DEFAULT)
    // via VL53L1_get_preset_mode_timing_cfg(). With the API, the register
    // actually ends up with a slightly different value because it gets assigned,
    // retrieved, recalculated with a different macro period, and reassigned,
    // but it probably doesn't matter because it seems like the MM ("mode
    // mitigation"?) sequence steps are disabled in low power auto mode anyway.
    writeWord(0x005A, encode_timeout(
    timeout_microseconds_to_mclks(1, macro_period_us)));

    // "Update Range Timing A timeout"
    writeWord(0x005E, encode_timeout(
    timeout_microseconds_to_mclks(range_config_timeout_us, macro_period_us)));

    // "Update Macro Period for Range B VCSEL Period"
    macro_period_us = calc_macro_period(reg16(0x0063).get());

    // "Update MM Timing B timeout"
    // (See earlier comment about MM Timing A timeout.)
    writeWord(0x005C, encode_timeout(
    timeout_microseconds_to_mclks(1, macro_period_us)));

    // "Update Range Timing B timeout"
    writeWord(0x0061, encode_timeout(
    timeout_microseconds_to_mclks(range_config_timeout_us, macro_period_us)));

    // VL53L1_calc_timeout_register_values() end
}

uint32_t VL53L1XSensor::timeout_microseconds_to_mclks(uint32_t timeout_us, uint32_t macro_period_us)
{
  return (((uint32_t)timeout_us << 12) + (macro_period_us >> 1)) / macro_period_us;
}

// Encode sequence step timeout register value from timeout in MCLKs
// based on VL53L1_encode_timeout()
uint16_t VL53L1XSensor::encode_timeout(uint32_t timeout_mclks)
{
  // encoded format: "(LSByte * 2^MSByte) + 1"

  uint32_t ls_byte = 0;
  uint16_t ms_byte = 0;

  if (timeout_mclks > 0)
  {
    ls_byte = timeout_mclks - 1;

    while ((ls_byte & 0xFFFFFF00) > 0)
    {
      ls_byte >>= 1;
      ms_byte++;
    }

    return (ms_byte << 8) | (ls_byte & 0xFF);
  }
  else { return 0; }
}

void VL53L1XSensor::set_signal_threshold() {
    writeWord(0x0066, signal_threshold_>>3); // div by 1024 to convert KCPS to MCPS
}

void VL53L1XSensor::set_roi() {
    reg16(0x007F) = this->roi_center_;
    uint8_t roi_size = (this->roi_size_y_ - 1) << 4 | (this->roi_size_x_ - 1);
    reg16(0x0080) = roi_size;
}


} //namespace vl53l1x
} //namespace esphome
