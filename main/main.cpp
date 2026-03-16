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
#include <dirent.h>

extern "C"{
  #include "mdns.h"
#include "esp_spiffs.h"
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





// new SPIFFS

void spiffs_init() {
    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs", 
        .partition_label = nullptr,
        .max_files = 5,
        .format_if_mount_failed = true
    };

    esp_err_t ret = esp_vfs_spiffs_register(&conf);
    if (ret != ESP_OK) {
        printf("Failed to mount SPIFFS\n");
        return;
    }



    DIR* dir = opendir("/spiffs");
struct dirent* ent;

while ((ent = readdir(dir)) != NULL) {
    ESP_LOGI("SPIFFS", "File: %s", ent->d_name);
}

closedir(dir);

    size_t total = 0, used = 0;
    esp_spiffs_info(nullptr, &total, &used);
    printf("SPIFFS total: %d, used: %d\n", total, used);
}


//mime typing for proper browser display 
const char* get_mime_type(const char* path) {
    if (strstr(path, ".js")) return "application/javascript";
    if (strstr(path, ".css")) return "text/css";
    if (strstr(path, ".html")) return "text/html";
    if (strstr(path, ".png")) return "image/png";
    if (strstr(path, ".jpg") || strstr(path, ".jpeg")) return "image/jpeg";
    return "text/plain";
}

// Web server handler
esp_err_t file_get_handler(httpd_req_t *req)
{
    char filepath[512];
    const char *base_path = "/spiffs";

    // If root is requested, serve index.html
    if (strcmp(req->uri, "/") == 0) {
        snprintf(filepath, sizeof(filepath), "%s/index.html", base_path);
    } 
    else {
        snprintf(filepath, sizeof(filepath), "%s%.*s",
                 base_path,
                 (int)(sizeof(filepath) - strlen(base_path) - 1),
                 req->uri);
    }

    ESP_LOGI("HTTP", "Serving file: %s", filepath);

    FILE *file = fopen(filepath, "r");
    if (!file) {
        ESP_LOGE("HTTP", "File not found");
        httpd_resp_send_404(req);
        return ESP_FAIL;
    }

    httpd_resp_set_type(req, get_mime_type(filepath));

    char chunk[1024];
    size_t read_bytes;

    while ((read_bytes = fread(chunk, 1, sizeof(chunk), file)) > 0) {
        httpd_resp_send_chunk(req, chunk, read_bytes);
    }

    fclose(file);

    httpd_resp_send_chunk(req, NULL, 0);
    return ESP_OK;
}

// URL registration (global)
httpd_uri_t root = {
    .uri = "/*",
    .method = HTTP_GET,
    .handler = file_get_handler,
    .user_ctx = NULL
};









httpd_handle_t start_webserver(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    // Enable wildcard URI matching
    config.uri_match_fn = httpd_uri_match_wildcard;

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
    wifi_init();
    spiffs_init();     // start AP mode
      server_init();   // start web server
      mdns_config();  //sets domain for the default ip  
}