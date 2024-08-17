if (CONFIG_BT_ESP_HOSTED)
message(STATUS "♥♥♥♥♥ esp-hosted HCI driver ♥♥♥♥♥")

zephyr_library_sources(hci_esp_hosted.c)

endif()