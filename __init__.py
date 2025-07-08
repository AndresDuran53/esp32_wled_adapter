import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import light
from esphome.const import CONF_ID, CONF_PORT

DEPENDENCIES = ['network']
AUTO_LOAD = ['light']

esp32_wled_adapter_ns = cg.esphome_ns.namespace('esp32_wled_adapter')
WLEDUDPComponent = esp32_wled_adapter_ns.class_('WLEDUDPComponent', cg.Component)

# Configuration
CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(WLEDUDPComponent),
    cv.Required(CONF_PORT): cv.port,
    cv.Required('light_id'): cv.use_id(light.AddressableLight),
}).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    
    light_var = await cg.get_variable(config['light_id'])
    cg.add(var.set_strip(light_var))
    cg.add(var.set_port(config[CONF_PORT]))