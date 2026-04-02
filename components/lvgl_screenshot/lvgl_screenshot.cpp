#include "lvgl_screenshot.h"
#include "lodepng.h"
#include "esphome/core/log.h"

// Standard LVGL include for ESPHome
#include <lvgl.h>

// Attempting the most direct include for the snapshot utility in v8
#include "src/extra/others/snapshot/lv_snapshot.h"
#include <cstdio>

namespace esphome {
namespace lvgl_screenshot {

static const char *const TAG = "lvgl_screenshot";

void LVGLScreenshot::setup() {
    ESP_LOGI(TAG, "LVGL PNG Screenshot tool initialized.");
}

void LVGLScreenshot::save_png(const std::string &filename) {
    lv_obj_t *screen = lv_scr_act();
    uint32_t w = lv_obj_get_width(screen);
    uint32_t h = lv_obj_get_height(screen);

    ESP_LOGI(TAG, "Capturing %ux%u PNG...", w, h);

    // 1. Take Snapshot (ARGB8888 for high quality)
    lv_img_dsc_t *snapshot = lv_snapshot_take(screen, LV_IMG_CF_TRUE_COLOR_ALPHA);
    
    if (snapshot == nullptr) {
        ESP_LOGE(TAG, "Snapshot failed! Check if PSRAM is properly initialized.");
        return;
    }

    // 2. Color Swap (LVGL ARGB to PNG RGBA)
    uint8_t *data = (uint8_t *)snapshot->data;
    for (uint32_t i = 0; i < w * h * 4; i += 4) {
        uint8_t b = data[i];     // Blue
        uint8_t r = data[i + 2]; // Red
        data[i]     = r;         // Put Red in first byte
        data[i + 2] = b;         // Put Blue in third byte
    }

    // 3. Encode and Save
    std::string full_path = "/sdcard/" + filename;
    unsigned char* png_out = nullptr;
    size_t png_size = 0;
    
    unsigned int error = lodepng_encode32(&png_out, &png_size, data, w, h);

    if (!error) {
        FILE *f = fopen(full_path.c_str(), "wb");
        if (f) {
            fwrite(png_out, 1, png_size, f);
            fclose(f);
            ESP_LOGI(TAG, "Successfully saved to %s", full_path.c_str());
        } else {
            ESP_LOGE(TAG, "SD Card Error: Could not open file for writing.");
        }
    } else {
        ESP_LOGE(TAG, "PNG Encoder Error: %s", lodepng_error_text(error));
    }

    // 4. Cleanup
    lv_snapshot_free(snapshot);
    free(png_out);
}

}  // namespace lvgl_screenshot
}  // namespace esphome
