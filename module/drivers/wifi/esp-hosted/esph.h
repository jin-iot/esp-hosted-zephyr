/*
 * Copyright (c) 2024 Jin (juanjin.dev@gmail.com)
 *
 * SPDX-License-Identifier: BSL-1.0
 */

#ifndef ZEPHYR_DRIVERS_WIFI_ESP_HOSTED_ESP_HOSTED_H_
#define ZEPHYR_DRIVERS_WIFI_ESP_HOSTED_ESP_HOSTED_H_

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/net/net_pkt.h>
#include <zephyr/net/buf.h>
#include <zephyr/net/wifi_mgmt.h>

#include <stdbool.h>
#include <stdint.h>

struct esph_proto_hdr {
    uint8_t if_type : 4;
#define ESPH_IF_TYPE_STA      (uint8_t)0x0
#define ESPH_IF_TYPE_AP       (uint8_t)0x1
#define ESPH_IF_TYPE_HCI      (uint8_t)0x2
#define ESPH_IF_TYPE_INTERNAL (uint8_t)0x3
#define ESPH_IF_TYPE_TEST     (uint8_t)0x4
    uint8_t if_no : 4;
    uint8_t flags;
    uint8_t pkt_type;
#define ESPH_PKT_TYPE_DATA    (uint8_t)0x00
#define ESPH_PKT_TYPE_CMD_REQ (uint8_t)0x01
#define ESPH_PKT_TYPE_CMD_RES (uint8_t)0x02
#define ESPH_PKT_TYPE_EVT     (uint8_t)0x03
#define ESPH_PKT_TYPE_EAPOL   (uint8_t)0x04
    uint8_t reserved1;
    uint16_t len;
    uint16_t offset;
    uint16_t cksm;
    uint8_t reserved2;
    union {
        uint8_t reserved3;
        uint8_t hci_pkt_type;
        uint8_t priv_pkt_type;
    };
} __packed;

struct esph_bus_ops {
    int (*init)(const struct device *);
    int (*process)(const struct device *);
};

enum esph_drv_state {
    ESPH_DRV_STATE_INIT = 0x0000,
    ESPH_DRV_STATE_IDLE = 0x0001,
    ESPH_DRV_STATE_SCAN = 0x0002,
};

enum esph_trans_state {
    ESPH_TRANS_STATE_IDLE,
    ESPH_TRANS_STATE_INIT,
    ESPH_TRANS_STATE_DATA_READY,
};

struct esph_data {
    const struct device *dev;
    struct net_if *net_if;
    struct k_mutex gp_lock;
    sys_slist_t pending_tx;
    enum esph_drv_state drv_state;
    enum esph_trans_state trans_state;
    struct k_work recv_work;
};

struct esph_config {
    struct gpio_dt_spec reset;
    struct esph_bus_ops *bus_ops;
};

#if defined(CONFIG_WIFI_ESP_HOSTED_SPI)
#include <zephyr/drivers/spi.h>
extern struct esph_bus_ops __esph_spi_bus_ops;
struct esph_spi_config {
    struct esph_config base;
    struct spi_dt_spec bus;
    struct gpio_dt_spec data_ready;
    struct gpio_dt_spec handshake;
};

struct esph_spi_data {
    struct esph_data base;
    struct gpio_callback handshake_cb;
    struct gpio_callback data_ready_cb;
    struct k_work trans_work;
};

#define ESPH_SPI_OPS (SPI_OP_MODE_MASTER | \
                      SPI_MODE_CPHA | \
                      SPI_MODE_CPOL | \
                      SPI_TRANSFER_MSB | \
                      SPI_WORD_SET(8))
#define ESPH_DEFINE_SPI_BUS(inst) \
        static struct esph_spi_config esph_config_##inst = { \
            .base = { \
                .reset = GPIO_DT_SPEC_INST_GET(inst, reset_gpios), \
                .bus_ops = &__esph_spi_bus_ops, \
            }, \
            .bus = SPI_DT_SPEC_INST_GET(inst, ESPH_SPI_OPS, 0), \
            .data_ready = GPIO_DT_SPEC_INST_GET(inst, data_ready_gpios), \
            .handshake = GPIO_DT_SPEC_INST_GET(inst, handshake_gpios), \
        }; \
        static struct esph_spi_data esph_data_##inst;
        
#endif // CONFIG_WIFI_ESP_HOSTED_SPI

#if defined(CONFIG_WIFI_ESP_HOSTED_SDHC)
#include <zephyr/drivers/sdhc.h>
extern struct esph_bus_ops __esph_sdio_bus_ops;
struct esph_sdio_bus {
    struct esph_bus_data data;
    struct sdio_dt_spec bus;
    struct gpio_dt_spec resetn;
}
#define ESPH_SDIO_OPS (SDIO_BUS_SPEED_HS | SDIO_BUS_WIDTH_4)
#define ESPH_DEFINE_SDIO_BUS(inst) \
        static struct esph_sdio_bus esph_bus_data_##inst = { \
            .base = { \
                .reset = GPIO_DT_SPEC_INST_GET(inst, resetn_gpios), \
                .bus_ops = &__esph_spi_bus_ops, \
            }, \
            .data.dev = DEVICE_DT_GET(DT_NODELABEL(inst)), \
            .bus = SDIO_DT_SPEC_INST_GET(inst, ESPH_SDIO_OPS), \
        };
#endif // CONFIG_WIFI_ESP_HOSTED_SDIO

static inline int esph_reset(const struct device *esp) {
    int __esph_reset(const struct device *esp);
    return __esph_reset(esp);
}

static inline int esph_bus_init(const struct device *esp) {
    const struct esph_config *cfg = esp->config;
    return cfg->bus_ops->init(esp);
}

static inline int esph_bus_process(const struct device *esp) {
    const struct esph_config *cfg = esp->config;
    return cfg->bus_ops->process(esp);
}

static inline void esph_proto_make_scan(struct esph_proto_hdr *hdr) {
    void __esph_proto_make_scan(struct esph_proto_hdr *hdr);
    __esph_proto_make_scan(hdr);
}

static inline bool esph_proto_is_dummy(struct esph_proto_hdr *hdr) {
    return hdr->if_type == 0x0F && hdr->if_no == 0x0F && hdr->len == 0;
}

#endif // ZEPHYR_DRIVERS_WIFI_ESP_HOSTED_ESP_HOSTED_H_