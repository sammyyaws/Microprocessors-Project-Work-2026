#include "spiffs.h"
#include <dirent.h>
#include "esp_log.h"
extern "C"{
  #include "mdns.h"
#include "esp_spiffs.h"
}




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


