#include "lvgl_screenshot.h"
#include "lodepng.h"
#include "esphome/core/log.h"
// Include the specific LVGL v8 snapshot header
#include "lvgl/src/extra/others/snapshot/lv_snapshot.h"
#include <cstdio>

namespace esphome {
namespace lvgl_screenshot {

static const char *const TAG = "lvgl_screenshot";

void LVGLScreenshot::setup() {
    ESP_LOGI(TAG, "LVGL Screenshot component ready (v8 Compatible).");
}

void LVGLScreenshot::save_png(const std::string &filename) {
    lv_obj_t *screen = lv_scr_act();
    uint32_t w = lv_obj_get_width(screen);
    uint32_t h = lv_obj_get_height(screen);

    ESP_LOGI(TAG, "Capturing %ux%u PNG...", w, h);

    // 1. Take Snapshot using LVGL v8 API
    // In v8, we use LV_IMG_CF_TRUE_COLOR_ALPHA for 32-bit capture
    lv_img_dsc_t *snapshot = lv_snapshot_take(screen, LV_IMG_CF_TRUE_COLOR_ALPHA);
    
    if (snapshot == nullptr) {
        ESP_LOGE(TAG, "Snapshot failed! Ensure LV_USE_SNAPSHOT is enabled or RAM is available.");
        return;
    }

    // 2. Encode to PNG
    std::string full_path = "/sdcard/" + filename;
    unsigned char* png_out = nullptr;
    size_t png_size = 0;
    
    // LodePNG expects RGBA8888. LV_IMG_CF_TRUE_COLOR_ALPHA is usually ARGB8888.
    // LodePNG is smart enough to handle most 32-bit buffers, but we call the 32-bit encoder.
    unsigned int error = lodepng_encode32(&png_out, &png_size, (unsigned char*)snapshot->data, w, h);

    if (!error) {
        FILE *f = fopen(full_path.c_str(), "wb");
        if (f) {
            fwrite(png_out, 1, png_size, f);
            fclose(f);
            ESP_LOGI(TAG, "Success! Saved to %s (%u bytes)", full_path.c_str(), (uint32_t)png_size);
        } else {
            ESP_LOGE(TAG, "Failed to open SD card for writing.");
        }
    } else {
        ESP_LOGE(TAG, "PNG Error: %s", lodepng_error_text(error));
    }

    // 3. Clean up v8 snapshot
    // In v8, we free the data buffer and the descriptor separately
    lv_snapshot_free(snapshot);
    free(png_out);
}

}  // namespace lvgl_screenshot
}  // namespace esphome
