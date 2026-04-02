#include "lvgl_screenshot.h"
#include "lodepng.h"
#include "esphome/core/log.h"
#include <lvgl.h>
#include <src/extra/others/snapshot/lv_snapshot.h>
#include <cstdio>
#include "esp_heap_caps.h" // Required for explicit PSRAM allocation

namespace esphome {
namespace lvgl_screenshot {

static const char *const TAG = "lvgl_screenshot";

void LVGLScreenshot::setup() {}

void LVGLScreenshot::save_png(const std::string &filename) {
    lv_obj_t *screen = lv_scr_act();
    uint32_t w = lv_obj_get_width(screen);
    uint32_t h = lv_obj_get_height(screen);

    ESP_LOGI(TAG, "Capturing %ux%u PNG...", w, h);

    // 1. Take Snapshot
    // LVGL will use its own memory manager (which you routed to PSRAM)
    lv_img_dsc_t *snapshot = lv_snapshot_take(screen, LV_IMG_CF_TRUE_COLOR_ALPHA);
    
    if (snapshot == nullptr) {
        ESP_LOGE(TAG, "Snapshot failed! LVGL memory full.");
        return;
    }

    // 2. Color Swap (BGRA to RGBA)
    uint8_t *data = (uint8_t *)snapshot->data;
    for (uint32_t i = 0; i < w * h * 4; i += 4) {
        uint8_t b = data[i];
        data[i] = data[i + 2];
        data[i + 2] = b;
    }

    // 3. Encode to PNG
    // We use a custom allocator for LodePNG to ensure it uses PSRAM
    std::string full_path = "/sdcard/" + filename;
    unsigned char* png_out = nullptr;
    size_t png_size = 0;

    // Use a LodePNG state to customize allocation
    LodePNGState state;
    lodepng_state_init(&state);
    
    // Perform encoding
    unsigned int error = lodepng_encode(&png_out, &png_size, data, w, h, &state);

    if (!error) {
        FILE *f = fopen(full_path.c_str(), "wb");
        if (f) {
            fwrite(png_out, 1, png_size, f);
            fclose(f);
            ESP_LOGI(TAG, "Saved: %s", full_path.c_str());
        } else {
            ESP_LOGE(TAG, "SD Card error.");
        }
    } else {
        ESP_LOGE(TAG, "PNG Error: %s", lodepng_error_text(error));
    }

    // 4. Cleanup
    lv_snapshot_free(snapshot);
    if(png_out) free(png_out);
    lodepng_state_cleanup(&state);
}

}  // namespace lvgl_screenshot
}  // namespace esphome
