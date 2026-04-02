#pragma once
#include "esphome/core/component.h"
#include "esphome/core/log.h"
#include <lvgl.h>

namespace esphome {
namespace lvgl_screenshot {

class LVGLScreenshot : public Component {
 public:
  void setup() override;
  void dump_config() override;
  void take_snapshot();
  
  float get_setup_priority() const override { return esphome::setup_priority::LATE; }
};

}  // namespace lvgl_screenshot
}  // namespace esphome
