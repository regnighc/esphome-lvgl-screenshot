#include "lvgl_screenshot.h"
#include "lodepng.h"
#include "esphome/core/log.h"

// Correct include paths for ESPHome/LVGL v8
#include <lvgl.h>
#include <src/extra/others/snapshot/lv_snapshot.h>
#include <cstdio>

namespace esphome {
namespace lvgl_screenshot {

static const char *const TAG = "lvgl_screenshot";

void LVGLScreenshot::setup() {
    ESP_LOGI(TAG, "LVGL Screenshot component ready.");
}

void LVGLScreenshot::save_png(const std::string &filename) {
    lv_obj_t *screen = lv_scr_act();
    uint32_t w = lv_obj_get_width(screen);
    uint32_t h = lv_obj_get_height(screen);

    ESP_LOGI(TAG, "Capturing %ux%u PNG...", w, h);

    // 1. Take Snapshot
    lv_img_dsc_t *snapshot = lv_snapshot_take(screen, LV_IMG_CF_TRUE_COLOR_ALPHA);
    
    if (snapshot == nullptr) {
        ESP_LOGE(TAG, "Snapshot failed! Is 'LV_USE_SNAPSHOT: 1' in your YAML?");
        return;
    }

    // 2. Color Swap (BGRA to RGBA)
    // ESP32 is Little Endian. LVGL's ARGB8888 is stored in memory as [B, G, R, A].
    // LodePNG requires [R, G, B, A].
    uint8_t *data = (uint8_t *)snapshot->data;
    for (uint32_t i = 0; i < w * h * 4; i += 4) {
        uint8_t temp_blue = data[i];
        data[i]     = data[i + 2]; // Move Red to Byte 0
        data[i + 2] = temp_blue;   // Move Blue to Byte 2
    }

    // 3. Encode to PNG
    std::string full_path = "/sdcard/" + filename;
    unsigned char* png_out = nullptr;
    size_t png_size = 0;
    
    unsigned int error = lodepng_encode32(&png_out, &png_size, data, w, h);

    if (!error) {
        FILE *f = fopen(full_path.c_str(), "wb");
        if (f) {
            fwrite(png_out, 1, png_size, f);
            fclose(f);
            ESP_LOGI(TAG, "Success! Saved %s (%u bytes)", full_path.c_str(), (uint32_t)png_size);
        } else {
            ESP_LOGE(TAG, "Could not open file on SD card.");
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
