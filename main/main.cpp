#include <iostream>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "esp_log.h"
  #include "mdns.h"
  #include "wifi.h"
#include "Control_device.h"
#include  "spiffs.h"
#include  "webserver.h"




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




// **************--- Main app ---***********//
extern "C" void app_main(void)
{
    wifi_init();
    spiffs_init();     // start AP mode
      server_init();   // start web server
      mdns_config();  //sets domain for the default ip  
      Control_device();
}