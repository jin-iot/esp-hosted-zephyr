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
#include <zephyr/net/wifi_mgmt.h>

#include <stdbool.h>
#include <stdint.h>

struct esph_bus_trans_header {
    uint8_t if_type : 4;
    uint8_t if_no : 4;
    uint8_t flags;
    uint8_t pkt_type;
    uint8_t reserved1;
    uint16_t len;
    uint16_t offset;
    uint16_t cksm;
    uint16_t reserved2;

    uint8_t *payload;
};

struct esph_bus_trans_data {
    struct esph_bus_trans_header header;
    uint8_t payload[1500];
};

struct esph_bus_ops {
    int (*init)(const struct device *);
    int (*send)(const struct device *, struct net_pkt *);
    int (*scan)(const struct device *, scan_result_cb_t);
};

struct esph_bus_data {
    struct esph_bus_ops *bus_ops;
};

struct esph_config {
    bool is_spi;
    struct esph_bus_data *bus_data;
};

enum esph_drv_state {
    ESPH_DRV_STATE_INIT,
    ESPH_DRV_STATE_READY,
    ESPH_DRV_STATE_ERROR,
};

enum esph_trans_state {
    ESPH_TRANS_STATE_IDLE,
    ESPH_TRANS_STATE_INIT,
    ESPH_TRANS_STATE_DATA_READY,
};

struct esph_data {
    const struct device *dev;
    struct net_if *net_if;
    uint8_t bus_buf[1600];
    struct esph_bus_trans_data trans_data;
    enum esph_drv_state drv_state;
    enum esph_trans_state trans_state;
    struct k_work recv_work;
};

#if defined(CONFIG_WIFI_ESP_HOSTED_SPI)
#include <zephyr/drivers/spi.h>
extern struct esph_bus_ops __esph_spi_bus_ops;
struct esph_spi_bus {
    struct esph_bus_data data;
    struct spi_dt_spec bus;
    struct gpio_dt_spec resetn;
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
        static struct esph_spi_bus esph_bus_data_##inst = { \
            .data.bus_ops = &__esph_spi_bus_ops, \
            .bus = SPI_DT_SPEC_INST_GET(inst, ESPH_SPI_OPS, 0), \
            .resetn = GPIO_DT_SPEC_INST_GET(inst, resetn_gpios), \
            .data_ready = GPIO_DT_SPEC_INST_GET(inst, data_ready_gpios), \
            .handshake = GPIO_DT_SPEC_INST_GET(inst, handshake_gpios), \
        }; \
        static struct esph_spi_data esph_data_##inst;
        
#endif // CONFIG_WIFI_ESP_HOSTED_SPI

#if defined(CONFIG_WIFI_ESP_HOSTED_SDIO)
#include <zephyr/drivers/sdio.h>
extern struct esph_bus_ops __esph_sdio_bus_ops;
struct esph_sdio_bus {
    struct esph_bus_data data;
    struct sdio_dt_spec bus;
    struct gpio_dt_spec resetn;
}
#define ESPH_SDIO_OPS (SDIO_BUS_SPEED_HS | SDIO_BUS_WIDTH_4)
#define ESPH_DEFINE_SPI_BUS(inst) \
        static struct esph_sdio_bus esph_bus_data_##inst = { \
            .data.dev = DEVICE_DT_GET(DT_NODELABEL(inst)), \
            .data.bus_ops = &__esph_sdio_bus_ops, \
            .bus = SDIO_DT_SPEC_INST_GET(inst, ESPH_SDIO_OPS), \
            .resetn = GPIO_DT_SPEC_INST_GET(inst, resetn_gpios), \
        };
#endif // CONFIG_WIFI_ESP_HOSTED_SDIO

enum esph_cmd_code {
	ESPH_CMD_CODE_INIT_INTERFACE = 1,
	ESPH_CMD_CODE_SET_MAC,
	ESPH_CMD_CODE_GET_MAC,
	ESPH_CMD_CODE_SCAN_REQUEST,
	ESPH_CMD_CODE_STA_CONNECT,
	ESPH_CMD_CODE_STA_DISCONNECT,
	ESPH_CMD_CODE_DEINIT_INTERFACE,
	ESPH_CMD_CODE_ADD_KEY,
	ESPH_CMD_CODE_DEL_KEY,
	ESPH_CMD_CODE_SET_DEFAULT_KEY,
	ESPH_CMD_CODE_STA_AUTH,
	ESPH_CMD_CODE_STA_ASSOC,
	ESPH_CMD_CODE_SET_IP_ADDR,
	ESPH_CMD_CODE_SET_MCAST_MAC_ADDR,
	ESPH_CMD_CODE_GET_TXPOWER,
	ESPH_CMD_CODE_SET_TXPOWER,
	ESPH_CMD_CODE_GET_REG_DOMAIN,
	ESPH_CMD_CODE_SET_REG_DOMAIN,
	ESPH_CMD_CODE_RAW_TP_ESP_TO_HOST,
	ESPH_CMD_CODE_RAW_TP_HOST_TO_ESP,
	ESPH_CMD_CODE_SET_WOW_CONFIG,
	ESPH_CMD_CODE_MAX,
};

enum esph_if_type {
	ESPH_IF_TYPE_STA,
	ESPH_IF_TYPE_AP,
	ESPH_IF_TYPE_HCI,
	ESPH_IF_TYPE_INTERNAL,
	ESPH_IF_TYPE_TEST,
};

enum esph_pkt_type {
	ESPH_PKT_TYPE_DATA,
	ESPH_PKT_TYPE_COMMAND_REQUEST,
	ESPH_PKT_TYPE_COMMAND_RESPONSE,
	ESPH_PKT_TYPE_EVENT,
	ESPH_PKT_TYPE_EAPOL,
};

static inline int esph_bus_init(const struct device *esp) {
    const struct esph_config *config = esp->config;
    return config->bus_data->bus_ops->init(esp);
}

static inline int esph_bus_scan(const struct device *esp,
                    scan_result_cb_t cb)
{
    const struct esph_config *config = esp->config;
    return config->bus_data->bus_ops->scan(esp, cb);
}

static inline int esph_bus_send(const struct device *esp, struct net_pkt *pkt) {
    const struct esph_config *config = esp->config;
    return config->bus_data->bus_ops->send(esp, pkt);
}

void __esph_proto_make_scan(struct esph_bus_trans_header *hdr);
static inline void esph_proto_make_scan(struct esph_bus_trans_header *hdr) {
    __esph_proto_make_scan(hdr);
}

#endif // ZEPHYR_DRIVERS_WIFI_ESP_HOSTED_ESP_HOSTED_H_