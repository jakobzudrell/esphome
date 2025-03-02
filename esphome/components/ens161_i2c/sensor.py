import esphome.codegen as cg
from esphome.components import i2c

from ..ens161_base import CONFIG_SCHEMA_BASE, cv, to_code_base

AUTO_LOAD = ["ens161_base"]
CODEOWNERS = ["@latonita"]
DEPENDENCIES = ["i2c"]

ens161_ns = cg.esphome_ns.namespace("ens161_i2c")

ENS161I2CComponent = ens161_ns.class_(
    "ENS161I2CComponent", cg.PollingComponent, i2c.I2CDevice
)

CONFIG_SCHEMA = CONFIG_SCHEMA_BASE.extend(
    i2c.i2c_device_schema(default_address=0x52)
).extend({cv.GenerateID(): cv.declare_id(ENS161I2CComponent)})


async def to_code(config):
    var = await to_code_base(config)
    await i2c.register_i2c_device(var, config)
