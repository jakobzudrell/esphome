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
        }
    )
    .extend(cv.polling_component_schema("1s"))
    .extend(i2c.i2c_device_schema(0x5A))
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await i2c.register_i2c_device(var, config)

    if "presence" in config:
        sens = await sensor.new_sensor(config["presence"])
        cg.add(var.set_presence_sensor(sens))

    if "motion" in config:
        sens = await sensor.new_sensor(config["motion"])
        cg.add(var.set_motion_sensor(sens))
