if (CONFIG_WIFI_ESP_HOSTED)
    message(STATUS "##################### Building esp-hosted WiFi driver #####################")
    set(ZEPHYR_ESP_HOSTED_SOURCES esp-hosted.c)
    zephyr_library()
    
    if (CONFIG_WIFI_ESP_HOSTED_SPI)
        set(ZEPHYR_ESP_HOSTED_SOURCES ${ZEPHYR_ESP_HOSTED_SOURCES} esp-hosted-spi.c)
    endif()

    zephyr_library_sources(${ZEPHYR_ESP_HOSTED_SOURCES})
endif()