idf_component_register(
    SRCS "webserver.cpp" "spiffs.cpp" "wifi.cpp" "main.cpp" "Control_device.cpp"
    INCLUDE_DIRS "."
    REQUIRES json esp_driver_gpio  mdns esp_http_server esp_netif nvs_flash esp_wifi esp_event
    PRIV_REQUIRES spiffs
)
