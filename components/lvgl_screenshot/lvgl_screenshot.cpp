#include "lvgl_screenshot.h"
#include "esphome/core/log.h"
#include <lvgl.h>
#include <src/extra/others/snapshot/lv_snapshot.h>
#include <cstdio>
#include "esp_heap_caps.h"

// --- LodePNG Memory Override ---
// This forces LodePNG to use PSRAM instead of Internal RAM
#include <stdlib.h>
#define LODEPNG_NO_COMPILE_ALLOCATORS
void* lodepng_malloc(size_t size) {
    return heap_caps_malloc(size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
}
void* lodepng_realloc(void* ptr, size_t new_size) {
    return heap_caps_realloc(ptr, new_size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
}
void lodepng_free(void* ptr) {
    heap_caps_free(ptr);
}
// Now include the actual lodepng code
#include "lodepng.h"
// -------------------------------

namespace esphome {
namespace lvgl_screenshot {

static const char *const TAG = "lvgl_screenshot";

void LVGLScreenshot::setup() {}

void LVGLScreenshot::save_png(const std::string &filename) {
    // Add this debug line at the very top of the function
    size_t free_psram = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
    ESP_LOGI(TAG, "Free PSRAM before capture: %u bytes", free_psram);
    
    lv_obj_t *screen = lv_scr_act();
    uint32_t w = lv_obj_get_width(screen);
    uint32_t h = lv_obj_get_height(screen);

    ESP_LOGI(TAG, "Capturing %ux%u PNG to PSRAM...", w, h);

    // 1. Take Snapshot
    // Note: LVGL is already routed to PSRAM via your sdkconfig
    lv_img_dsc_t *snapshot = lv_snapshot_take(screen, LV_IMG_CF_TRUE_COLOR_ALPHA);
    
    if (snapshot == nullptr) {
        ESP_LOGE(TAG, "Snapshot failed! LVGL PSRAM heap is full.");
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
    std::string full_path = "/sdcard/" + filename;
    unsigned char* png_out = nullptr;
    size_t png_size = 0;

    // Because of the overrides above, this will now use PSRAM
    unsigned int error = lodepng_encode32(&png_out, &png_size, data, w, h);

    if (!error) {
        FILE *f = fopen(full_path.c_str(), "wb");
        if (f) {
            // Write in chunks to avoid stressing the SPI bus
            size_t written = fwrite(png_out, 1, png_size, f);
            fclose(f);
            if (written == png_size) {
                ESP_LOGI(TAG, "Successfully saved %s (%u bytes)", full_path.c_str(), (uint32_t)png_size);
            } else {
                ESP_LOGE(TAG, "SD Write incomplete. Card full?");
            }
        } else {
            ESP_LOGE(TAG, "SD Card error: Could not open file.");
        }
    } else {
        ESP_LOGE(TAG, "PNG Error: %s", lodepng_error_text(error));
    }

    // 4. Cleanup
    lv_snapshot_free(snapshot);
    if(png_out) lodepng_free(png_out);
}

}  // namespace lvgl_screenshot
}  // namespace esphome
