/*
 * Copyright (c) 2024 Jin (juanjin.dev@gmail.com)
 *
 * SPDX-License-Identifier: BSL-1.0
 */

#define DT_DRV_COMPAT espressif_esp_hosted
#include "esp_hosted.h"

#include <zephyr/drivers/mfd/esp_hosted.h>

#include <zephyr/net/ethernet.h>
#include <zephyr/net/net_pkt.h>
#include <zephyr/net/net_if.h>
#include <zephyr/device.h>

LOG_MODULE_REGISTER(mfd_esp_hosted, CONFIG_MFD_LOG_LEVEL);

static int esph_dev_init(const struct device *dev) {
    int ret;
    struct mfd_esp_hosted *esp = dev->data;
    esp->esp_dev = dev;

    sys_slist_init(&esp->pending_tx);
    
    ret = esph_bus_init(esp);

out:
    return ret;
}

#define ESP_HOSTED_INIT(_inst) \
        COND_CODE_1(DT_INST_ON_BUS(_inst, spi), \
            (ESPH_DEFINE_SPI_BUS(_inst)), \
            ( \
                COND_CODE_1(DT_INST_ON_BUS(_inst, sdhc), \
                    (ESPH_DEFINE_SDIO_BUS(_inst)), \
                    (BUILD_ASSERT(0, "Unsupported bus type")) \
                ) \
            ) \
        )

DT_INST_FOREACH_STATUS_OKAY(ESP_HOSTED_INIT);