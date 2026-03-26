#include <iostream>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "esp_log.h"
  #include "mdns.h"
  #include "wifi.h"
#include "Control_device.h"
#include  "spiffs.h"
#include  "webserver.h"
#include <driver/gpio.h>
extern "C"{
#include "ds1302.h"
}


extern "C" void app_main();

//get the actual time from the ds1302

//ds1302 pin definitions
#define DS1302_RST GPIO_NUM_13
#define DS1302_DAT GPIO_NUM_12
#define DS1302_CLK GPIO_NUM_14 


//config ds1302_t rtc;
ds1302_t rtc = {
    .ce_pin = GPIO_NUM_13,
    .io_pin = GPIO_NUM_12,
    .sclk_pin = GPIO_NUM_14,
    .ch=false
};

void app_rtc()
{
    ESP_LOGI("RTC", "Initializing DS1302...");

    ds1302_init(&rtc);
    ds1302_start(&rtc, true);

   

    // VERY IMPORTANT: Read back the time to confirm
    struct tm read_time;
    if (ds1302_get_time(&rtc, &read_time) == ESP_OK) {
        ESP_LOGI("RTC", "RTC Time -> %02d:%02d:%02d",
                 read_time.tm_hour,
                 read_time.tm_min,
                 read_time.tm_sec);
    } else {
        ESP_LOGE("RTC", "Failed to read time!");
    }
}


// Server initialization
void server_init(void)
{
    start_webserver();
}

/// MDNS CONFIGURATION  
void mdns_config(){
  ESP_ERROR_CHECK( mdns_init());
  //set the hostname
  ESP_ERROR_CHECK(mdns_hostname_set("Dswitch"));
    ESP_LOGI("MDNS","domain name is set to Dswitch.local");

}

// initialize gpio pins
void gpio_init_pins() {
    gpio_config_t io_conf = {};

    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (1ULL<<GPIO_NUM_2) |
                           (1ULL<<GPIO_NUM_4) |
                           (1ULL<<GPIO_NUM_5) |
                           (1ULL<<GPIO_NUM_18);

    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    io_conf.intr_type = GPIO_INTR_DISABLE;

    gpio_config(&io_conf);

    gpio_set_level(GPIO_NUM_2, 0);
    gpio_set_level(GPIO_NUM_4, 0);
    gpio_set_level(GPIO_NUM_5, 0);
    gpio_set_level(GPIO_NUM_18, 0);
}
    



// **************--- Main app ---***********//
extern "C" void app_main(void)
{
wifi_init();
spiffs_init();
gpio_init_pins();
mdns_config();
server_init();
app_rtc();



      /// rtos task 
      xTaskCreate(
        control_task,
        "control_task",
        4096,
        NULL,
        5,
        NULL
      );
}