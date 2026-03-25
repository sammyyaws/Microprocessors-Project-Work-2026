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



extern "C" void app_main();

 







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

    // Optional: set default OFF statevoid gpio_init_pins() {
   

    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (1ULL<<GPIO_NUM_2) |
                           (1ULL<<GPIO_NUM_4) |
                           (1ULL<<GPIO_NUM_5) |
                           (1ULL<<GPIO_NUM_18);

    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    io_conf.intr_type = GPIO_INTR_DISABLE;

    gpio_config(&io_conf);

    // Optional: set default OFF state
    gpio_set_level(GPIO_NUM_2, 0);
    gpio_set_level(GPIO_NUM_4, 0);
    gpio_set_level(GPIO_NUM_5, 0);
    gpio_set_level(GPIO_NUM_18, 0);
}
    



// **************--- Main app ---***********//
extern "C" void app_main(void)
{
    wifi_init();
    spiffs_init();     // start AP mode
      server_init();   // start web server
      mdns_config();  //sets domain for the default ip  
      
 gpio_init_pins();

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