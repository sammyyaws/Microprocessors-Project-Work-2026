#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "esp_log.h"
#include "Control_device.h"
#include <cJSON.h>

// load devices
cJSON* load_devices_from_file() {
    FILE *file = fopen("/spiffs/devices.json", "r");

    if (!file) {
        ESP_LOGW("JSON", "File not found, returning empty array");
        return cJSON_CreateArray();
    }

    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *buf = (char*) malloc(size + 1);
    if (!buf) {
        fclose(file);
        ESP_LOGE("JSON", "Memory allocation failed");
        return NULL;
    }

    fread(buf, 1, size, file);
    buf[size] = '\0';
    fclose(file);

    cJSON *root = cJSON_Parse(buf);
    free(buf);

    if (!root) {
        ESP_LOGW("JSON", "Invalid JSON, resetting");
        return cJSON_CreateArray();
    }

    return root;
}


// MAIN CONTROL FUNCTION
void Control_device(void) {

    cJSON *devices = load_devices_from_file();

    if (!devices) {
        ESP_LOGE("CONTROL", "Failed to load devices");
        return;
    }

    ESP_LOGI("TEST", "JSON loaded successfully");

    int count = cJSON_GetArraySize(devices);

    for (int i = 0; i < count; i++) {

        cJSON *device = cJSON_GetArrayItem(devices, i);

        if (device) {
            ESP_LOGI("TEST", "Found device at index %d", i);
        }
    }

    // free memory
    cJSON_Delete(devices);
}