import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID

# Specify that we depend on the 'lvgl' component
DEPENDENCIES = ["lvgl"]

lvgl_screenshot_ns = cg.esphome_ns.namespace("lvgl_screenshot")
LVGLScreenshot = lvgl_screenshot_ns.class_("LVGLScreenshot", cg.Component)

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(LVGLScreenshot),
}).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    
    # This replaces the need for the 'defines' block in your YAML
    cg.add_build_flag("-DLV_USE_SNAPSHOT=1")
    # Force PSRAM optimization flags
    cg.add_build_flag("-DCONFIG_SPIRAM_USE_MALLOC=1")
