#pragma once
#include "esphome/core/component.h"
#include <lvgl.h>
#include <string>

namespace esphome {
namespace lvgl_screenshot {

class LVGLScreenshot : public Component {
 public:
  void setup() override;
  void save_png(const std::string &filename);
  float get_setup_priority() const override { return esphome::setup_priority::LATE; }
};

}  // namespace lvgl_screenshot
}  // namespace esphome
