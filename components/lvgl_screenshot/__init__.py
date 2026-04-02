import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID

# Define the namespace and the C++ class
lvgl_screenshot_ns = cg.esphome_ns.namespace("lvgl_screenshot")
LVGLScreenshot = lvgl_screenshot_ns.class_("LVGLScreenshot", cg.Component)

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(LVGLScreenshot),
}).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    # Ensure LVGL is included in the build
    cg.add_library("lvgl", None)
