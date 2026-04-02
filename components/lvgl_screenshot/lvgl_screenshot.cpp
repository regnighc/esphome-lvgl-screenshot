#include "lvgl_screenshot.h"
#include "esphome/core/log.h"
#include <lvgl.h>
#include <src/extra/others/snapshot/lv_snapshot.h>
#include <cstdio>
#include "esp_heap_caps.h"

// LodePNG Allocators using PSRAM
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
#include "lodepng.h"

namespace esphome {
namespace lvgl_screenshot {

static const char *const TAG = "lvgl_screenshot";

void LVGLScreenshot::setup() {}

void LVGLScreenshot::save_png(const std::string &filename) {
    // 1. Check Memory Availability
    size_t free_psram = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
    ESP_LOGI(TAG, "Available PSRAM: %u bytes", free_psram);

    lv_obj_t *screen = lv_scr_act();
    uint32_t w = lv_obj_get_width(screen);
    uint32_t h = lv_obj_get_height(screen);

    // 2. Take Snapshot
    // This will now be allocated from the managed PSRAM heap
    ESP_LOGI(TAG, "Taking snapshot...");
    lv_img_dsc_t *snapshot = lv_snapshot_take(screen, LV_IMG_CF_TRUE_COLOR_ALPHA);
    
    if (snapshot == nullptr) {
        ESP_LOGE(TAG, "Snapshot allocation failed!");
        return;
    }

    // 3. Color Swap (BGRA -> RGBA)
    uint8_t *data = (uint8_t *)snapshot->data;
    for (uint32_t i = 0; i < w * h * 4; i += 4) {
        uint8_t b = data[i];
        data[i] = data[i + 2];
        data[i + 2] = b;
    }

    // 4. PNG Encode
    ESP_LOGI(TAG, "Encoding PNG (this may take a few seconds)...");
    std::string full_path = "/sdcard/" + filename;
    unsigned char* png_out = nullptr;
    size_t png_size = 0;

    unsigned int error = lodepng_encode32(&png_out, &png_size, data, w, h);

    if (!error) {
        FILE *f = fopen(full_path.c_str(), "wb");
        if (f) {
            fwrite(png_out, 1, png_size, f);
            fclose(f);
            ESP_LOGI(TAG, "PNG Saved: %s (%u bytes)", filename.c_str(), (uint32_t)png_size);
        } else {
            ESP_LOGE(TAG, "SD Card Error!");
        }
    } else {
        ESP_LOGE(TAG, "PNG Error: %s", lodepng_error_text(error));
    }

    // 5. Cleanup
    lv_snapshot_free(snapshot);
    if(png_out) lodepng_free(png_out);
}

}  // namespace lvgl_screenshot
}  // namespace esphome
