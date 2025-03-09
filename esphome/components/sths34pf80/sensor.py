import esphome.codegen as cg
from esphome.components import i2c, sensor
import esphome.config_validation as cv
from esphome.const import (
    CONF_ID,
    DEVICE_CLASS_EMPTY,
    STATE_CLASS_MEASUREMENT,
    UNIT_EMPTY,
)

CODEOWNERS = ["@jakobzudrell"]
DEPENDENCIES = ["i2c"]

sths34pf80_ns = cg.esphome_ns.namespace("sths34pf80")

STHS34PF80Component = sths34pf80_ns.class_(
    "STHS34PF80Component", cg.PollingComponent, i2c.I2CDevice
)

CONF_ODR = "odr"
CONF_AVG_T = "avg_t"
CONF_AVG_TMOS = "avg_tmos"

CONF_LPF_P_M = "lpf_p_m"
CONF_LPF_M = "lpf_m"
CONF_LPF_P = "lpf_p"
CONF_LPF_A_T = "lpf_a_t"

CONF_PRESENCE_THRESHOLD = "presence_threshold"
CONF_MOTION_THRESHOLD = "motion_threshold"
CONF_TAMB_SHOCK_THRESHOLD = "tamb_shock_threshold"

CONF_PRESENCE_HYSTERESIS = "presence_hysteresis"
CONF_MOTION_HYSTERESIS = "motion_hysteresis"
CONF_TAMB_SHOCK_HYSTERESIS = "tamb_shock_hysteresis"

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(STHS34PF80Component),
            cv.Optional("presence"): sensor.sensor_schema(
                unit_of_measurement=UNIT_EMPTY,
                accuracy_decimals=0,
                device_class=DEVICE_CLASS_EMPTY,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional("motion"): sensor.sensor_schema(
                unit_of_measurement=UNIT_EMPTY,
                accuracy_decimals=0,
                device_class=DEVICE_CLASS_EMPTY,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_ODR, default=0x04): cv.uint8_t,
            cv.Optional(CONF_AVG_T, default=0x00): cv.uint8_t,
            cv.Optional(CONF_AVG_TMOS, default=0x03): cv.uint8_t,
            cv.Optional(CONF_LPF_P_M, default=0x00): cv.uint8_t,
            cv.Optional(CONF_LPF_M, default=0x04): cv.uint8_t,
            cv.Optional(CONF_LPF_P, default=0x04): cv.uint8_t,
            cv.Optional(CONF_LPF_A_T, default=0x02): cv.uint8_t,
            cv.Optional(CONF_PRESENCE_THRESHOLD, default=200): cv.uint16_t,
            cv.Optional(CONF_MOTION_THRESHOLD, default=200): cv.uint16_t,
            cv.Optional(CONF_TAMB_SHOCK_THRESHOLD, default=10): cv.uint16_t,
            cv.Optional(CONF_PRESENCE_HYSTERESIS, default=50): cv.uint8_t,
            cv.Optional(CONF_MOTION_HYSTERESIS, default=50): cv.uint8_t,
            cv.Optional(CONF_TAMB_SHOCK_HYSTERESIS, default=2): cv.uint8_t,
        }
    )
    .extend(cv.polling_component_schema("1s"))
    .extend(i2c.i2c_device_schema(0x5A))
)

SETTING_MAP = {
    CONF_ODR: "set_odr",
    CONF_AVG_T: "set_avg_t",
    CONF_AVG_TMOS: "set_avg_tmos",
    CONF_LPF_P_M: "set_lpf_p_m",
    CONF_LPF_M: "set_lpf_m",
    CONF_LPF_P: "set_lpf_p",
    CONF_LPF_A_T: "set_lpf_a_t",
    CONF_PRESENCE_THRESHOLD: "set_presence_threshold",
    CONF_MOTION_THRESHOLD: "set_motion_threshold",
    CONF_TAMB_SHOCK_THRESHOLD: "set_tamb_shock_threshold",
    CONF_PRESENCE_HYSTERESIS: "set_presence_hysteresis",
    CONF_MOTION_HYSTERESIS: "set_motion_hysteresis",
    CONF_TAMB_SHOCK_HYSTERESIS: "set_tamb_shock_hysteresis",
}


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await i2c.register_i2c_device(var, config)

    for key, funcName in SETTING_MAP.items():
        if key in config:
            cg.add(getattr(var, funcName)(config[key]))

    if "presence" in config:
        sens = await sensor.new_sensor(config["presence"])
        cg.add(var.set_presence_sensor(sens))

    if "motion" in config:
        sens = await sensor.new_sensor(config["motion"])
        cg.add(var.set_motion_sensor(sens))
