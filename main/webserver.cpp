 #include <cstring>   // strcmp, strlen
#include <cstdio>    // fopen, fread, fwrite
#include <cstdlib>   // malloc, free
#include <string>    // std::string
#include <esp_log.h> // ESP_LOGI, ESP_LOGE
#include <dirent.h>
#include "cJSON.h"
#include <esp_http_server.h>
#include "webserver.h"





//mime typing for proper browser display 
const char* get_mime_type(const char* path) {
    if (strstr(path, ".js")) return "application/javascript";
    if (strstr(path, ".css")) return "text/css";
    if (strstr(path, ".html")) return "text/html";
    if (strstr(path, ".png")) return "image/png";
    if (strstr(path, ".jpg") || strstr(path, ".jpeg")) return "image/jpeg";
    return "text/plain";
}



//this is the cors  configuration 
void add_cors_headers(httpd_req_t *req) {
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Headers", "Content-Type");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Methods", "GET, POST, OPTIONS");
}

//cors device options handler
esp_err_t options_handler(httpd_req_t *req)
{
    add_cors_headers(req);

    httpd_resp_set_status(req, "204 No Content"); 
      httpd_resp_set_type(req, "text/plain");  
    httpd_resp_send(req, NULL, 0);

    return ESP_OK;
}
//cors uri registrations
httpd_uri_t options = {
    .uri = "/*",
    .method = HTTP_OPTIONS,
    .handler = options_handler,
    .user_ctx = NULL
};



// Web server handler for rendereing the dahoboard(the html data)
esp_err_t file_get_handler(httpd_req_t *req)
{
    add_cors_headers(req);
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

//end of file serving handler




// URL registration (global)
httpd_uri_t root = {
    .uri = "/*",
    .method = HTTP_GET,
    .handler = file_get_handler,
    .user_ctx = NULL
};






// this is the GET file handler for the devices added

esp_err_t get_devices_handler(httpd_req_t *req) {
    add_cors_headers(req);
    FILE *file = fopen("/spiffs/devices.json", "r");
    if (!file) {
        httpd_resp_send(req, "[]", HTTPD_RESP_USE_STRLEN);
        return ESP_OK;
    }

    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *buf = (char*) malloc(size + 1);
if (!buf) {
    fclose(file);
    httpd_resp_send_500(req);
    return ESP_FAIL;
}
   size_t read = fread(buf, 1, size, file);
buf[read] = '\0';
    fclose(file);
   
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, buf, read);
    free(buf);
    return ESP_OK;
}

// this is the POST handler for the devices added

esp_err_t post_devices_handler(httpd_req_t *req) {
    add_cors_headers(req);
    char buf[2048];

    int ret = httpd_req_recv(req, buf, sizeof(buf) - 1);
    if (ret <= 0) {
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }
    buf[ret] = '\0';

    // Parse incoming JSON
    cJSON *new_device = cJSON_Parse(buf);
    if (!new_device) {
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    // Extract fields from UI
    cJSON *name_item = cJSON_GetObjectItem(new_device, "deviceName");
    cJSON *onTime_item = cJSON_GetObjectItem(new_device, "onTime");
    cJSON *offTime_item = cJSON_GetObjectItem(new_device, "offTime");

    cJSON *pin_item  = cJSON_GetObjectItem(new_device, "pin");

    if (!cJSON_IsString(name_item) || !cJSON_IsString(onTime_item) || !cJSON_IsString(offTime_item) || !cJSON_IsString(pin_item)) {
        cJSON_Delete(new_device);
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    const char *device_name = name_item->valuestring;
    const char *on_time = onTime_item->valuestring;
    const char *off_time = offTime_item->valuestring;
    const char *device_pin  = pin_item->valuestring;

    // Read existing file
    FILE *file = fopen("/spiffs/devices.json", "r");
    cJSON *root;

    if (file) {
        fseek(file, 0, SEEK_END);
        size_t size = ftell(file);
        fseek(file, 0, SEEK_SET);

        char *file_buf = (char*) malloc(size + 1);
        if (!file_buf) {
            fclose(file);
            cJSON_Delete(new_device);
            httpd_resp_send_500(req);
            return ESP_FAIL;
        }

        fread(file_buf, 1, size, file);
        file_buf[size] = '\0';
        fclose(file);

        root = cJSON_Parse(file_buf);
        free(file_buf);

        if (!root) {
            root = cJSON_CreateArray();
        }
    } else {
        root = cJSON_CreateArray();
    }

    // Ensure root is an array
    if (!cJSON_IsArray(root)) {
        cJSON_Delete(root);
        root = cJSON_CreateArray();
    }

    // Compute max_id
   
    int max_id = 0;
    int array_size = cJSON_GetArraySize(root);

    for (int i = 0; i < array_size; i++) {
        cJSON *device = cJSON_GetArrayItem(root, i);
        cJSON *id_item = cJSON_GetObjectItem(device, "id");

        if (cJSON_IsNumber(id_item)) {
            if (id_item->valueint > max_id) {
                max_id = id_item->valueint;
            }
        }
    }

    int new_id = max_id + 1;

   
    // Create new device object
 
    cJSON *device_obj = cJSON_CreateObject();
    cJSON_AddNumberToObject(device_obj, "id", new_id);
    cJSON_AddStringToObject(device_obj, "name", device_name);
    cJSON_AddStringToObject(device_obj, "onTime", on_time);
    cJSON_AddStringToObject(device_obj, "offTime", off_time);
    cJSON_AddStringToObject(device_obj, "pin", device_pin);

    // Add to array
    cJSON_AddItemToArray(root, device_obj);

    // Convert back to string
    char *json_string = cJSON_PrintUnformatted(root);

    // Save to file
    file = fopen("/spiffs/devices.json", "w");
    if (file) {
        fwrite(json_string, 1, strlen(json_string), file);
        fclose(file);
    }

    // Respond
   
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, json_string, HTTPD_RESP_USE_STRLEN);

    // Cleanup
    cJSON_free(json_string);
    cJSON_Delete(root);
    cJSON_Delete(new_device);

    return ESP_OK;
}
 
//url  definitions
httpd_uri_t devices_get = {
    .uri = "/devices",
    .method = HTTP_GET,
    .handler = get_devices_handler,
    .user_ctx = NULL
};

httpd_uri_t devices_post = {
    .uri = "/devices",
    .method = HTTP_POST,
    .handler = post_devices_handler,
    .user_ctx = NULL
};


httpd_handle_t start_webserver(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
config.stack_size = 8192;  
    // Enable wildcard URI matching
    config.uri_match_fn = httpd_uri_match_wildcard;

    httpd_handle_t server = NULL;

    if (httpd_start(&server, &config) == ESP_OK)
    {
        // Register API FIRST
httpd_register_uri_handler(server, &options);
httpd_register_uri_handler(server, &devices_get);
httpd_register_uri_handler(server, &devices_post);

// wildcard file server
httpd_register_uri_handler(server, &root);
ESP_LOGI("SERVER", "successfully started HTTP server");
    }
    else
    {
        ESP_LOGE("SERVER", "Failed to start HTTP server");
    }

   



return server;



}
