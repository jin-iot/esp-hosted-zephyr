#ifndef ZEPHYR_DRIVERS_WIFI_ESP_HOSTED_ESP_HOSTED_H_
#define ZEPHYR_DRIVERS_WIFI_ESP_HOSTED_ESP_HOSTED_H_

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

#include <stdbool.h>
#include <stdint.h>

struct esph_bus_ops {
    int (*init)(const struct device *esp);
};

struct esph_bus_data {
    struct esph_bus_ops *bus_ops;
};

struct esph_config {
    bool is_spi;
    struct esph_bus_data *bus_data;
};

enum esph_state {
    ESPH_STATE_IDLE,
    ESPH_STATE_INIT,
    ESPH_STATE_READY,
    ESPH_STATE_CONNECTED,
    ESPH_STATE_DISCONNECTED,
    ESPH_STATE_ERROR
};

struct esph_data {
    const struct device *dev;
    struct net_if *net_if;
    enum esph_state state;
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
    struct k_work data_ready_work;
    struct k_work handshake_work;
};

#define ESPH_SPI_OPS (SPI_OP_MODE_MASTER | SPI_TRANSFER_MSB | SPI_WORD_SET(8))
#define ESPH_DEFINE_SPI_BUS(inst) \
        static struct esph_spi_bus esph_bus_data_##inst = { \
            .data.bus_ops = &__esph_spi_bus_ops, \
            .bus = SPI_DT_SPEC_INST_GET(inst, ESPH_SPI_OPS, 0), \
            .resetn = GPIO_DT_SPEC_INST_GET(inst, resetn_gpios), \
            .data_ready = GPIO_DT_SPEC_INST_GET(inst, data_ready_gpios), \
            .handshake = GPIO_DT_SPEC_INST_GET(inst, handshake_gpios), \
        }; \
        static struct esph_spi_data esph_data_##inst = { \
            .base = { \
                .state = ESPH_STATE_IDLE \
            }, \
        };
        
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
	ESPH_IF_TYPE_MAX,
};

struct esph_cmd_header {
    uint8_t cmd_code;
    uint8_t cmd_status;
    uint16_t len;
    uint16_t seq_num;
    uint8_t reserved1;
	uint8_t reserved2;
};

static inline int esph_bus_init(const struct device *dev) {
    const struct esph_config *config = dev->config;
    return config->bus_data->bus_ops->init(dev);
}

#endif // ZEPHYR_DRIVERS_WIFI_ESP_HOSTED_ESP_HOSTED_H_