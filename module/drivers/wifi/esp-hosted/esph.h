/*
 * Copyright (c) 2024 Jin (juanjin.dev@gmail.com)
 *
 * SPDX-License-Identifier: BSL-1.0
 */

#ifndef ZEPHYR_DRIVERS_WIFI_ESP_HOSTED_ESPH_H_
#define ZEPHYR_DRIVERS_WIFI_ESP_HOSTED_ESPH_H_

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/net/net_pkt.h>
#include <zephyr/net/buf.h>
#include <zephyr/net/net_if.h>
#include <zephyr/net/wifi_mgmt.h>

#include <stdbool.h>
#include <stdint.h>

enum esph_drv_state {
    ESPH_DRV_STATE_ERR = 0,
    ESPH_DRV_STATE_DEV_INIT = BIT(0),
    ESPH_DRV_STATE_IF_INIT = BIT(0),
    ESPH_DRV_STATE_IDLE = BIT(1),
    ESPH_DRV_STATE_DRDY = BIT(2),
    ESPH_DRV_STATE_HS = BIT(3),
};

enum esph_wifi_state {
    ESPH_DRV_STA_STOPPED,
	ESPH_DRV_STA_STARTED,
	ESPH_DRV_STA_CONNECTING,
	ESPH_DRV_STA_CONNECTED,
	ESPH_DRV_AP_CONNECTED,
	ESPH_DRV_AP_DISCONNECTED,
	ESPH_DRV_AP_STOPPED,
};

enum esph_rx_type {
    ESPH_RX_TYPE_ERR,
    ESPH_RX_TYPE_STA_CMD_RES,
    ESPH_RX_TYPE_AP_CMD_RES,
};

#define ESPH_DRV_STATE_DATA_CLEAR(_state) \
    (_state & ~(ESPH_DRV_STATE_DRDY | ESPH_DRV_STATE_HS))

struct esph_priv;

/**
 * @brief ESP Hosted bus-specific operations
 */
struct esph_bus_ops {
    int (*init)(struct esph_priv *);
    int (*transceive)(struct esph_priv *, void *, size_t, void *, size_t);
    int (*process)(struct esph_priv *);
};

/**
 * @brief ESP Hosted base data
 */
struct esph_priv {
    const struct device *esp_dev;
    struct net_if *iface;
    uint8_t rx_buffer[CONFIG_WIFI_ESP_HOSTED_RX_BUF_SIZE];
    struct k_condvar gp_cond;
    struct k_mutex gp_mutex;
    const struct gpio_dt_spec reset;
    const struct esph_bus_ops *bus_ops;
    sys_slist_t pending_tx;
    enum esph_drv_state drv_state;
};

#define ESPH_DEFINE_BASE(_inst, _bus_ops) \
        .base = { \
            .reset = GPIO_DT_SPEC_INST_GET(_inst, reset_gpios), \
            .bus_ops = _bus_ops, \
        }, \

#if defined(CONFIG_WIFI_ESP_HOSTED_SPI)
#include <zephyr/drivers/spi.h>

struct esph_spi_priv {
    struct esph_priv base;
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
        static struct esph_spi_priv esph_priv ## _inst = { \
            ESPH_DEFINE_BASE(_inst, &__esph_spi_bus_ops) \
            .bus = SPI_DT_SPEC_INST_GET(_inst, ESPH_SPI_OPS, 0), \
            .data_ready = GPIO_DT_SPEC_INST_GET(_inst, data_ready_gpios), \
            .handshake = GPIO_DT_SPEC_INST_GET(_inst, handshake_gpios), \
        };

#else
#define ESPH_DEFINE_SPI_BUS(inst)
#endif // CONFIG_WIFI_ESP_HOSTED_SPI

#if defined(CONFIG_WIFI_ESP_HOSTED_SDHC)
#include <zephyr/drivers/sdhc.h>
#define ESPH_DEFINE_SDIO_BUS(inst)
#else
#define ESPH_DEFINE_SDIO_BUS(inst)
#endif // CONFIG_WIFI_ESP_HOSTED_SDIO

static inline int esph_bus_init(struct esph_priv *esp) {
    return esp->bus_ops->init(esp);
}

static inline int esph_bus_transceive(struct esph_priv *esp,
                void *odata, size_t olen,
                void *idata, size_t ilen)
{
    return esp->bus_ops->
        transceive(esp, odata, olen, idata, ilen);
}

static inline int esph_proto_process(struct esph_priv *esp) {
    int __esph_proto_process(struct esph_priv *esp);
    return __esph_proto_process(esp);
}

#endif // ZEPHYR_DRIVERS_WIFI_ESP_HOSTED_ESPH_H_