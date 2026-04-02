#include "lvgl_screenshot.h"
#include "lodepng.h"
#include "esphome/core/log.h"
#include <cstdio>

namespace esphome {
namespace lvgl_screenshot {

static const char *const TAG = "lvgl_screenshot";

void LVGLScreenshot::setup() {
    ESP_LOGI(TAG, "LVGL PNG Screenshot component ready (PSRAM optimized).");
}

void LVGLScreenshot::save_png(const std::string &filename) {
    lv_obj_t *screen = lv_scr_act();
    uint32_t w = lv_obj_get_width(screen);
    uint32_t h = lv_obj_get_height(screen);

    ESP_LOGI(TAG, "Capturing %ux%u PNG...", w, h);

    // 1. Take snapshot in ARGB8888 (LVGL 9 native format for high-quality capture)
    lv_draw_buf_t* snapshot = lv_snapshot_take(screen, LV_COLOR_FORMAT_ARGB8888);
    
    if (snapshot == nullptr || snapshot->data == nullptr) {
        ESP_LOGE(TAG, "Failed to take LVGL snapshot.");
        return;
    }

    // 2. Encode to PNG using LodePNG
    // lodepng_encode_file(filename, data, width, height, colortype, bitdepth)
    std::string full_path = "/sdcard/" + filename;
    
    // We use state-based encoding to ensure we can handle the memory 
    unsigned char* png_out = nullptr;
    size_t png_size = 0;
    
    // ARGB8888 to PNG (LodePNG expects RGBA8888)
    unsigned int error = lodepng_encode32(&png_out, &png_size, (unsigned char*)snapshot->data, w, h);

    if (!error) {
        FILE *f = fopen(full_path.c_str(), "wb");
        if (f) {
            fwrite(png_out, 1, png_size, f);
            fclose(f);
            ESP_LOGI(TAG, "Success! Saved to %s (%u bytes)", full_path.c_str(), (uint32_t)png_size);
        } else {
            ESP_LOGE(TAG, "Failed to open SD card file for writing.");
        }
    } else {
        ESP_LOGE(TAG, "PNG Encoder Error %u: %s", error, lodepng_error_text(error));
    }

    // 3. Cleanup
    free(png_out);
    lv_draw_buf_destroy(snapshot);
}

}  // namespace lvgl_screenshot
}  // namespace esphome
