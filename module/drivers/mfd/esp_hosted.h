/*
 * Copyright (c) 2024 Jin (juanjin.dev@gmail.com)
 *
 * SPDX-License-Identifier: BSL-1.0
 */

#ifndef ZEPHYR_DRIVERS_MFD_ESP_HOSTED_ESPH_H_
#define ZEPHYR_DRIVERS_MFD_ESP_HOSTED_ESPH_H_

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/net/net_pkt.h>
#include <zephyr/net/buf.h>
#include <zephyr/net/net_if.h>
#include <zephyr/net/wifi_mgmt.h>
#include <zephyr/logging/log.h>

#include <stdbool.h>
#include <stdint.h>

LOG_MODULE_DECLARE(mfd_esp_hosted);

enum mfd_esp_hosted_state {
    MFD_ESP_HOSTED_STATE_UNINIT,
	MFD_ESP_HOSTED_STATE_STA_STARTED = BIT(0),
	MFD_ESP_HOSTED_STATE_STA_CONNECTING = BIT(1),
	MFD_ESP_HOSTED_STATE_STA_CONNECTED = BIT(2),
    MFD_ESP_HOSTED_STATE_SCAN_REQUESTED = BIT(3),
	MFD_ESP_HOSTED_STATE_AP_CONNECTED = BIT(4),
};

struct mfd_esp_hosted_bus_ops;

/**
 * @brief ESP Hosted base data
 */
struct mfd_esp_hosted {
    const struct device *esp_dev;
    const struct gpio_dt_spec reset;
    const struct mfd_esp_hosted_bus_ops *bus_ops;
    sys_slist_t pending_tx;
    enum mfd_esp_hosted_state state;
};

struct mfd_esp_hosted_bus_ops {
    int (*init)(struct mfd_esp_hosted *esp);
};

#define ESPH_DEFINE_BASE(_inst, _bus_ops) \
        .base = { \
            .reset = GPIO_DT_SPEC_INST_GET(_inst, reset_gpios), \
            .bus_ops = _bus_ops, \
        }, \

#if defined(CONFIG_MFD_ESP_HOSTED_SPI)
#include <zephyr/drivers/spi.h>

/**
 * @brief ESP Hosted SPI bus-specific data
 */
struct esph_spi_priv {
    struct mfd_esp_hosted base;
    struct spi_dt_spec bus;
    const struct gpio_dt_spec data_ready;
    const struct gpio_dt_spec handshake;
    struct gpio_callback handshake_cb;
    struct gpio_callback data_ready_cb;
    struct k_work trans_work;
};
extern const struct esph_bus_ops __esph_spi_bus_ops;

#define ESPH_SPI_OPS (SPI_OP_MODE_MASTER | \
                      SPI_MODE_CPOL | \
                      SPI_TRANSFER_MSB | \
                      SPI_WORD_SET(8))
#define ESPH_DEFINE_SPI_BUS(_inst) \
        static struct esph_spi_priv mfd_esp_hosted ## _inst = { \
            ESPH_DEFINE_BASE(_inst, &__esph_spi_bus_ops) \
            .bus = SPI_DT_SPEC_INST_GET(_inst, ESPH_SPI_OPS, 0), \
            .data_ready = GPIO_DT_SPEC_INST_GET(_inst, data_ready_gpios), \
            .handshake = GPIO_DT_SPEC_INST_GET(_inst, handshake_gpios), \
        };

#else
#define ESPH_DEFINE_SPI_BUS(inst)
#endif /* CONFIG_MFD_ESP_HOSTED_SPI */

#if defined(CONFIG_MFD_ESP_HOSTED_SDHC)
#include <zephyr/drivers/sdhc.h>
#define ESPH_DEFINE_SDIO_BUS(inst)
#else
#define ESPH_DEFINE_SDIO_BUS(inst)
#endif // CONFIG_WIFI_ESP_HOSTED_SDIO

static inline int esph_bus_init(struct mfd_esp_hosted *esp) {
    return esp->bus_ops->init(esp);
}

#endif /* ZEPHYR_DRIVERS_MFD_ESP_HOSTED_ESPH_H_ */