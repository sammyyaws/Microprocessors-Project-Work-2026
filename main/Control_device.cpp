    #include <freertos/FreeRTOS.h>
    #include <freertos/task.h>
    #include "esp_log.h"
    #include "Control_device.h"
    #include <cJSON.h>
#include <time.h>
#include <driver/gpio.h>
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


    //get the actual time



void get_current_time(int *hour, int *minute) {
    time_t now;
    struct tm timeinfo;

    time(&now);
    localtime_r(&now, &timeinfo);

    *hour = timeinfo.tm_hour;
    *minute = timeinfo.tm_min;
}

//PIN MAPPING  //pin mapping
    gpio_num_t map_pin(int pin) {
    switch (pin) {
        case 1: return GPIO_NUM_2;   // D2
        case 2: return GPIO_NUM_4;   // D4
        case 3: return GPIO_NUM_5;   // D5
        case 4: return GPIO_NUM_18;  // D18 (example)
        default: return GPIO_NUM_NC; // not connected
    }}

    // main control funtion for  the devices
   void Control_device(void) {

    static int last_minute = -1;

    int current_hour, current_minute;
    get_current_time(&current_hour, &current_minute);

    //  once-per-minute guard
    if (current_minute == last_minute) {
        return;
    }

    last_minute = current_minute;

    cJSON *devices = load_devices_from_file();

    if (!devices) {
        ESP_LOGE("CONTROL", "Failed to load devices");
        return;
    }

    ESP_LOGI("TEST", "JSON loaded successfully");

    int count = cJSON_GetArraySize(devices);

    for (int i = 0; i < count; i++) {

        cJSON *device = cJSON_GetArrayItem(devices, i);

        if (!device) continue;

        cJSON *name = cJSON_GetObjectItem(device, "name");
        cJSON *onTime  = cJSON_GetObjectItem(device, "onTime");
        cJSON *offTime = cJSON_GetObjectItem(device, "offTime");
        cJSON *pin  = cJSON_GetObjectItem(device, "pin");

        if (cJSON_IsString(name) && cJSON_IsString(onTime) && cJSON_IsString(offTime) && cJSON_IsString(pin)) {

            ESP_LOGI("DEVICE", "Name: %s | onTime: %s | offTime: %s | Pin: %s",
                     name->valuestring,
                     onTime->valuestring,
                     offTime->valuestring,
                     pin->valuestring);

            int on_hour, on_minute,off_hour,off_minute;
            sscanf(onTime->valuestring, "%d:%d", &on_hour, &on_minute);
            sscanf(offTime->valuestring, "%d:%d", &off_hour, &off_minute);

            ////Turn on 
            if (current_hour == on_hour && current_minute == on_minute) {
                int selected_pin = atoi(pin->valuestring);
                gpio_num_t gpio_pin = map_pin(selected_pin);
                if (gpio_pin == GPIO_NUM_NC) {
                    ESP_LOGE("CONTROL", "Invalid pin selected");
                    continue;
                }
                gpio_set_level(gpio_pin, 1);

                ESP_LOGI("CONTROL", "Turned ON pin %d for %s", gpio_pin, name->valuestring);
            }

 ////Turn off 
            if (current_hour == off_hour && current_minute == off_minute) {
                int selected_pin = atoi(pin->valuestring);
                gpio_num_t gpio_pin = map_pin(selected_pin);
                if (gpio_pin == GPIO_NUM_NC) {
                    ESP_LOGE("CONTROL", "Invalid pin selected");
                    continue;
                }
                
                gpio_set_level(gpio_pin, 0);

                ESP_LOGI("CONTROL", "Turned OFF pin %d for %s", gpio_pin, name->valuestring);
            }

        }
    }

    cJSON_Delete(devices);
}





    //rtos task scheduling

    void control_task(void *pvParameters)
{
    while (true)
    {
        Control_device();   // check devices

        vTaskDelay(pdMS_TO_TICKS(1000)); // run every 1 second
    }
}