
#include <nvs_flash.h>
#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_netif.h>
#include <esp_log.h>
#include <cstring>
#include <wifi.h>



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
    wifi_config.ap.max_connection = 10;
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