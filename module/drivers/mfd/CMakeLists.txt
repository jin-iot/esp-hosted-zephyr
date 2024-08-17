if (CONFIG_MFD_ESP_HOSTED)
    message(STATUS "♥♥♥♥♥ esp-hosted MFD driver ♥♥♥♥♥")
    zephyr_library()
    set(ZEPHYR_MFD_ESP_HOSTED_SOURCES
        esp_hosted.c
        esp_hosted_proto.c)
    
    if (CONFIG_MFD_ESP_HOSTED_SPI)
        set(ZEPHYR_ESP_HOSTED_SOURCES
            ${ZEPHYR_MFD_ESP_HOSTED_SOURCES}
            esp_hosted_spi.c)
    endif()

    if(CONFIG_MFD_ESP_HOSTED_SDHC)
        set(ZEPHYR_MFD_ESP_HOSTED_SOURCES
            ${ZEPHYR_MFD_ESP_HOSTED_SOURCES}
            esp_hosted_sdhc.c)
    endif()

    zephyr_library_sources(${ZEPHYR_MFD_ESP_HOSTED_SOURCES})
endif()