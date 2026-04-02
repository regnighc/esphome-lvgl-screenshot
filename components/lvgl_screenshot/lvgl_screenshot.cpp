#include "lvgl_screenshot.h"

namespace esphome {
namespace lvgl_screenshot {

static const char *const TAG = "lvgl_screenshot";

void LVGLScreenshot::setup() {
  ESP_LOGI(TAG, "LVGL Screenshot component initialized.");
}

void LVGLScreenshot::dump_config() {
  ESP_LOGCONFIG(TAG, "LVGL Screenshot");
}

void LVGLScreenshot::take_snapshot() {
  lv_obj_t *screen = lv_scr_act();
  
  // Calculate size for RGB565 (2 bytes per pixel)
  uint32_t width = lv_obj_get_width(screen);
  uint32_t height = lv_obj_get_height(screen);
  uint32_t buffer_size = width * height * 2;

  ESP_LOGI(TAG, "Capturing %dx%d screenshot...", width, height);

  // Allocate from PSRAM (required for large screens)
  void *buf = malloc(buffer_size);

  if (buf == nullptr) {
    ESP_LOGE(TAG, "Memory allocation failed! Not enough RAM.");
    return;
  }

  lv_res_t res = lv_snapshot_take(screen, LV_IMG_CF_TRUE_COLOR, (lv_snapshot_dsc_t *)buf);

  if (res == LV_RES_OK) {
    ESP_LOGI(TAG, "Snapshot success. Buffer address: %p", buf);
    // Logic to save/send 'buf' goes here
  } else {
    ESP_LOGE(TAG, "LVGL failed to take snapshot.");
  }

  free(buf);
}

}  // namespace lvgl_screenshot
}  // namespace esphome
