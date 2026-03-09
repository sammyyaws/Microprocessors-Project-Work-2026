#include <iostream>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <nvs_flash.h>
#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_netif.h>
#include <esp_log.h>
#include <cstring>
#include <esp_http_server.h>
extern "C"{
  #include "mdns.h"
}


extern "C" void app_main();

void wifi_init( void){

    //nvs initialization for storing wifi data, checks is the nvs partitions are full and checks if an update is needed
esp_err_t ret=nvs_flash_init();
    if (ret== ESP_ERR_NVS_NO_FREE_PAGES || ret ==  ESP_ERR_NVS_NEW_VERSION_FOUND){
      ESP_ERROR_CHECK(nvs_flash_erase());
      ESP_ERROR_CHECK(nvs_flash_init());
    }
    //enable newtwork interface  ///create an event loop
     
ESP_ERROR_CHECK(esp_netif_init());
ESP_ERROR_CHECK(esp_event_loop_create_default());
esp_netif_create_default_wifi_ap();     
   

        ///wifi initialization config
wifi_init_config_t cfg= WIFI_INIT_CONFIG_DEFAULT();
ESP_ERROR_CHECK(esp_wifi_init(&cfg));

///wifi configuration
wifi_config_t wifi_config={
};
    strcpy((char*)wifi_config.ap.ssid, "ap_test");
    strcpy((char*)wifi_config.ap.password, "12345678");

    wifi_config.ap.ssid_len = strlen("ap_test");
    wifi_config.ap.max_connection = 2;
    wifi_config.ap.authmode = WIFI_AUTH_WPA_WPA2_PSK;

  //SET WIFI MODE
ESP_ERROR_CHECK(esp_wifi_set_mode( WIFI_MODE_AP));
ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP,& wifi_config));
ESP_ERROR_CHECK( esp_wifi_start());
//printing the ip address and the other data
esp_netif_ip_info_t ip_info;
esp_netif_t* netif=esp_netif_get_handle_from_ifkey("WIFI_AP_DEF")  ;
if (netif == NULL) {
    ESP_LOGW("WIFI_AP", "Default AP netif handle not found");
}
ESP_ERROR_CHECK(esp_netif_get_ip_info(netif,&ip_info));
ESP_LOGI("WIFI_AP","IP Address:" IPSTR,IP2STR(&ip_info.ip));
ESP_LOGI("WIFI_AP","Subnet mask:" IPSTR,IP2STR(&ip_info.netmask));
ESP_LOGI("WIFI_AP","GateWay:" IPSTR,IP2STR(&ip_info.gw));


};




// Web server handler
esp_err_t root_get_handler(httpd_req_t *req)
{
    const char* resp =
        "<html><body>"
        "<h1>SERVER IS WORKING</h1>"
        "<p>You are connected to the ESP32 AP!</p>"
        "</body></html>";

    httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

// URL registration (global)
httpd_uri_t root = {
    .uri = "/",
    .method = HTTP_GET,
    .handler = root_get_handler,
    .user_ctx = NULL
};

// Start server
httpd_handle_t start_webserver(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t server = NULL;

    if (httpd_start(&server, &config) == ESP_OK)
    {
        httpd_register_uri_handler(server, &root);
        ESP_LOGI("SERVER", "HTTP server started");
    }
    else
    {
        ESP_LOGE("SERVER", "Failed to start HTTP server");
    }

    return server;
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




// **************--- Main app ---***********//
extern "C" void app_main(void)
{
    wifi_init();     // start AP mode
      server_init();   // start web server
      mdns_config();  //sets domain for the default ip  
}