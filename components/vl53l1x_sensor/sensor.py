import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import pins
from esphome.components import i2c, sensor
from esphome.const import (
    STATE_CLASS_MEASUREMENT,
    UNIT_METER,
    ICON_ARROW_EXPAND_VERTICAL,
    CONF_ADDRESS,
    CONF_TIMEOUT,
    CONF_ENABLE_PIN,
    CONF_IRQ_PIN
)

DEPENDENCIES = ["i2c"]

vl53l1x_ns = cg.esphome_ns.namespace("vl53l1x")
VL53L1XSensor = vl53l1x_ns.class_(
    "VL53L1XSensor", sensor.Sensor, cg.PollingComponent, i2c.I2CDevice
)

CONF_TIMING_BUDGET = "timing_budget"
CONF_DISTANCE_MODE = "distance_mode"
CONF_SIGNAL_THRESHOLD = "signal_threshold"
CONF_ROI_CENTER = "roi_center"
CONF_ROI_SIZE = "roi_size"
CONF_X = "x"
CONF_Y = "y"
CONF_AMBIENT_RATE_SENSOR = "ambient_rate_sensor"
CONF_AVG_SIGNAL_RATE_SENSOR = "avg_signal_rate_sensor"
CONF_PEAK_SIGNAL_RATE_SENSOR = "peak_signal_rate_sensor"
CONF_RANGE_STATUS_SENSOR = "range_status_sensor"
CONF_VALID_TIMING_BUDGET_DM_SHORT = [15, 20, 33, 50, 100, 200, 500]
CONF_VALID_TIMING_BUDGET_DM_MEDIUM_AND_LONG = [20, 33, 50, 100, 200, 500]


def check_keys(obj):
    if obj[CONF_ADDRESS] != 0x29 and CONF_ENABLE_PIN not in obj:
        msg = "Address other then 0x29 requires enable_pin definition to allow sensor\r"
        msg += "re-addressing. Also if you have more then one VL53 device on the same\r"
        msg += "i2c bus, then all VL53 devices must have enable_pin defined."
        raise cv.Invalid(msg)
    return obj


def check_timing_budget(obj):
    timing_budget = cv.positive_time_period_milliseconds(obj[CONF_TIMING_BUDGET])
    distance_mode = obj[CONF_DISTANCE_MODE]
    valid_timing_budget = CONF_VALID_TIMING_BUDGET_DM_SHORT if distance_mode == "SHORT" else CONF_VALID_TIMING_BUDGET_DM_MEDIUM_AND_LONG
    if timing_budget.total_milliseconds not in valid_timing_budget:
        valid = ", ".join([f"{v}ms" for v in valid_timing_budget])
        raise cv.Invalid(f"Timing budget can not be {timing_budget}. Valid values are: {valid}")
    return obj


def check_timeout(value):
    value = cv.positive_time_period_microseconds(value)
    if value.total_seconds > 60:
        raise cv.Invalid("Maximum timeout can not be greater then 60 seconds")
    return value


DistanceMode = vl53l1x_ns.enum("DistanceMode")

CONFIG_XY = cv.All({
    cv.Required(CONF_X): cv.int_range(4, 16),
    cv.Required(CONF_Y): cv.int_range(4, 16),
})

CONFIG_SCHEMA = cv.All(
    sensor.sensor_schema(
        VL53L1XSensor,
        unit_of_measurement=UNIT_METER,
        icon=ICON_ARROW_EXPAND_VERTICAL,
        accuracy_decimals=2,
        state_class=STATE_CLASS_MEASUREMENT,
    )
    .extend(
        {
            cv.Optional(CONF_TIMING_BUDGET, default="100ms"): cv.positive_time_period_microseconds,
            cv.Optional(CONF_DISTANCE_MODE, default="LONG"): cv.enum({"SHORT": DistanceMode.Short, "MEDIUM": DistanceMode.Medium, "LONG": DistanceMode.Long}),
            cv.Optional(CONF_SIGNAL_THRESHOLD, default="2"): cv.int_,
            cv.Optional(CONF_TIMEOUT, default="10ms"): check_timeout,
            cv.Optional(CONF_ENABLE_PIN): pins.gpio_output_pin_schema,
            cv.Optional(CONF_IRQ_PIN): pins.gpio_input_pin_schema,
            cv.Optional(CONF_AMBIENT_RATE_SENSOR): sensor.sensor_schema(
                accuracy_decimals=0,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_AVG_SIGNAL_RATE_SENSOR): sensor.sensor_schema(
                accuracy_decimals=0,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_PEAK_SIGNAL_RATE_SENSOR): sensor.sensor_schema(
                accuracy_decimals=0,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_RANGE_STATUS_SENSOR): sensor.sensor_schema(
                accuracy_decimals=0,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_ROI_CENTER, default=199): cv.int_range(0, 255),
            cv.Optional(CONF_ROI_SIZE, default={"x":16, "y": 16}): CONFIG_XY,
        }
    )
    .extend(cv.polling_component_schema("60s"))
    .extend(i2c.i2c_device_schema(0x29)),
    check_keys,
    check_timing_budget
)


async def to_code(config):
    var = await sensor.new_sensor(config)
    await cg.register_component(var, config)
    cg.add(var.set_timing_budget(config[CONF_TIMING_BUDGET]))
    cg.add(var.set_distance_mode(config[CONF_DISTANCE_MODE]))
    cg.add(var.set_signal_threshold(config[CONF_SIGNAL_THRESHOLD]))
    cg.add(var.set_timeout_us(config[CONF_TIMEOUT]))

    if CONF_ENABLE_PIN in config:
        enable = await cg.gpio_pin_expression(config[CONF_ENABLE_PIN])
        cg.add(var.set_enable_pin(enable))
    if CONF_IRQ_PIN in config:
        irq = await cg.gpio_pin_expression(config[CONF_IRQ_PIN])
        cg.add(var.set_irq_pin(irq))
    if ambient_rate_sensor_config := config.get(CONF_AMBIENT_RATE_SENSOR):
        sens = await sensor.new_sensor(ambient_rate_sensor_config)
        cg.add(var.set_ambient_rate_sensor(sens))
    if avg_signal_rate_sensor_config := config.get(CONF_AVG_SIGNAL_RATE_SENSOR):
        sens = await sensor.new_sensor(avg_signal_rate_sensor_config)
        cg.add(var.set_avg_signal_rate_sensor(sens))
    if peak_signal_rate_sensor_config := config.get(CONF_PEAK_SIGNAL_RATE_SENSOR):
        sens = await sensor.new_sensor(peak_signal_rate_sensor_config)
        cg.add(var.set_peak_signal_rate_sensor(sens))
    if range_status_sensor_config := config.get(CONF_RANGE_STATUS_SENSOR):
        sens = await sensor.new_sensor(range_status_sensor_config)
        cg.add(var.set_range_status_sensor(sens))
    if CONF_ROI_CENTER in config:
        cg.add(var.set_roi_center(config[CONF_ROI_CENTER]))
    if CONF_ROI_SIZE in config:
        roi_size = config[CONF_ROI_SIZE]
        cg.add(var.set_roi_size(roi_size[CONF_X], roi_size[CONF_Y]))

    await i2c.register_i2c_device(var, config)
