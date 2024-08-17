if (CONFIG_WIFI_ESP_HOSTED)
message(STATUS "♥♥♥♥♥ esp-hosted WIFI driver ♥♥♥♥♥")

zephyr_library_sources(esp_hosted_wifi.c)

endif()