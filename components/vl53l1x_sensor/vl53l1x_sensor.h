#pragma once
#include <list>
#include "esphome/core/component.h"
#include "esphome/core/hal.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/i2c/i2c.h"

#define VL53L1_DEVICEERROR_NOUPDATE                    0
#define VL53L1_DEVICEERROR_VCSELCONTINUITYTESTFAILURE  1
#define VL53L1_DEVICEERROR_VCSELWATCHDOGTESTFAILURE    2
#define VL53L1_DEVICEERROR_NOVHVVALUEFOUND             3
#define VL53L1_DEVICEERROR_MSRCNOTARGET                4
#define VL53L1_DEVICEERROR_RANGEPHASECHECK             5
#define VL53L1_DEVICEERROR_SIGMATHRESHOLDCHECK         6
#define VL53L1_DEVICEERROR_PHASECONSISTENCY            7
#define VL53L1_DEVICEERROR_MINCLIP                     8
#define VL53L1_DEVICEERROR_RANGECOMPLETE               9
#define VL53L1_DEVICEERROR_ALGOUNDERFLOW               10
#define VL53L1_DEVICEERROR_ALGOOVERFLOW                11
#define VL53L1_DEVICEERROR_RANGEIGNORETHRESHOLD        12
#define VL53L1_DEVICEERROR_USERROICLIP                 13
#define VL53L1_DEVICEERROR_REFSPADCHARNOTENOUGHDPADS   14
#define VL53L1_DEVICEERROR_REFSPADCHARMORETHANTARGET   15
#define VL53L1_DEVICEERROR_REFSPADCHARLESSTHANTARGET   16
#define VL53L1_DEVICEERROR_MULTCLIPFAIL                17
#define VL53L1_DEVICEERROR_GPHSTREAMCOUNT0READY        18
#define VL53L1_DEVICEERROR_RANGECOMPLETE_NO_WRAP_CHECK 19
#define VL53L1_DEVICEERROR_EVENTCONSISTENCY            20
#define VL53L1_DEVICEERROR_MINSIGNALEVENTCHECK         21
#define VL53L1_DEVICEERROR_RANGECOMPLETE_MERGED_PULSE  22
#define VL53L1_DEVICEERROR_PREV_RANGE_NO_TARGETS       23

#define	VL53L1_RANGESTATUS_RANGE_VALID			            	0
#define	VL53L1_RANGESTATUS_SIGMA_FAIL				              1
#define	VL53L1_RANGESTATUS_SIGNAL_FAIL				            2
#define	VL53L1_RANGESTATUS_RANGE_VALID_MIN_RANGE_CLIPPED	3
#define	VL53L1_RANGESTATUS_OUTOFBOUNDS_FAIL			          4
#define	VL53L1_RANGESTATUS_HARDWARE_FAIL			            5
#define	VL53L1_RANGESTATUS_RANGE_VALID_NO_WRAP_CHECK_FAIL 6
#define	VL53L1_RANGESTATUS_WRAP_TARGET_FAIL			          7
#define	VL53L1_RANGESTATUS_PROCESSING_FAIL			          8
#define	VL53L1_RANGESTATUS_XTALK_SIGNAL_FAIL			        9
#define	VL53L1_RANGESTATUS_SYNCRONISATION_INT			        10
#define	VL53L1_RANGESTATUS_RANGE_VALID_MERGED_PULSE	      11
#define	VL53L1_RANGESTATUS_TARGET_PRESENT_LACK_OF_SIGNAL	12
#define	VL53L1_RANGESTATUS_MIN_RANGE_FAIL			            13
#define	VL53L1_RANGESTATUS_RANGE_INVALID			            14
#define	VL53L1_RANGESTATUS_NONE				                    255

namespace esphome {
namespace vl53l1x {

enum DistanceMode { Short=1, Medium=2, Long=3 };

class VL53L1XSensor : public sensor::Sensor, public PollingComponent, public i2c::I2CDevice {
    public:
    VL53L1XSensor();

    void setup() override;

    void update() override;
    void dump_config() override;

    void loop() override;

    void set_timeout_us(uint32_t timeout_us) { this->timeout_us_ = timeout_us; }
    void set_enable_pin(GPIOPin *enable) { this->enable_pin_ = enable; }
    void set_distance_mode(DistanceMode dm) { this->distance_mode_ = dm; };
    void set_timing_budget(uint32_t us) { this->timing_budget_us_ = us; };
    void set_signal_threshold(uint16_t signal) { this->signal_threshold_ = signal; };
    void set_roi_center(uint8_t roi_center) { this->roi_center_ = roi_center; };
    void set_roi_center(uint8_t x, uint8_t y) {
      this->roi_center_ =
        (143 - y + x*8) * (1 - (uint8_t)((15 - y) / 8)) +
        (122 + y - x*8) * (uint8_t)((15 - y) / 8);
    };
    void set_roi_size(uint8_t x, uint8_t y) { this->roi_size_x_ = x; this->roi_size_y_ = y; };

    void set_ambient_rate_sensor(sensor::Sensor *sensor) { this->ambient_rate_sensor = sensor; }
    void set_avg_signal_rate_sensor(sensor::Sensor *sensor) { this->avg_signal_rate_sensor = sensor; }
    void set_peak_signal_rate_sensor(sensor::Sensor *sensor) { this->peak_signal_rate_sensor = sensor; }
    void set_range_status_sensor(sensor::Sensor *sensor) { this->range_status_sensor = sensor; }

    private:
    void setI2CAddress(uint8_t addr);
    void startRanging();
    void stopRanging();
    bool checkForDataReady();
    uint8_t getInterruptPolarity();
    void clearInterrupt();
    uint16_t sensorId();
    uint16_t readWord(uint16_t reg_idx);
    uint32_t calc_macro_period(uint8_t vcsel_period);
    uint16_t encode_timeout(uint32_t timeout_mclks);
    uint32_t timeout_microseconds_to_mclks(uint32_t timeout_us, uint32_t macro_period_us);
    void writeWord(uint16_t reg_idx, uint16_t data);

    int8_t getRangeStatus();
    int16_t distance();
    int16_t ambientRate();
    int16_t avgSignalRate();
    int16_t peakSignalRate();
    void set_distance_mode();
    void set_measurement_timing_budget();
    void set_signal_threshold();
    void set_roi();

    sensor::Sensor *range_status_sensor{nullptr};
    sensor::Sensor *ambient_rate_sensor{nullptr};
    sensor::Sensor *avg_signal_rate_sensor{nullptr};
    sensor::Sensor *peak_signal_rate_sensor{nullptr};

    static std::list<VL53L1XSensor *> vl53_sensors;
    GPIOPin *enable_pin_{nullptr};
    GPIOPin *irq_pin_{nullptr};
    static bool enable_pin_setup_complete;
    uint32_t timeout_start_us_;
    uint32_t timeout_us_{};
    DistanceMode distance_mode_{Long};
    uint32_t timing_budget_us_{};
    uint16_t signal_threshold_{};
    uint8_t roi_center_{};
    uint8_t roi_size_x_{};
    uint8_t roi_size_y_{};
    uint8_t rangeStatus{};
};

} //namespace vl53l1x
} //namespace esphome
