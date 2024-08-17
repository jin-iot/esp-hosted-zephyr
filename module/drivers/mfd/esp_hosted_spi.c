/*
 * Copyright (c) 2024 Jin (juanjin.dev@gmail.com)
 *
 * SPDX-License-Identifier: BSL-1.0
 */

#include "esp_hosted.h"

#include <zephyr/drivers/spi.h>
#include <zephyr/logging/log.h>

struct esph_spi_buf {
    struct spi_buf buf;
    sys_snode_t node;
};

static inline struct esph_spi_priv *esph_to_spi_priv(struct esph_priv *esp) {
    return (struct esph_spi_priv *)esp;
}

static void esph_spi_handle_handshake_irq(const struct device *port,
                struct gpio_callback *cb, gpio_port_pins_t pins)
{
    int hs;
    struct esph_spi_priv *esp =
        CONTAINER_OF(cb, struct esph_spi_priv, data_ready_cb);

    hs = gpio_pin_get_dt(&esp->handshake);
    if (hs) {
        esp->base.drv_state |= ESPH_DRV_STATE_HS;
        k_work_submit(&esp->trans_work);
    } else {
        esp->base.drv_state &= ~ESPH_DRV_STATE_HS;
    }
}

static void esph_spi_handle_data_ready_irq(const struct device *port,
                struct gpio_callback *cb, gpio_port_pins_t pins)
{
    int drdy;
    struct esph_spi_priv *esp =
        CONTAINER_OF(cb, struct esph_spi_priv, handshake_cb);
    
    drdy = gpio_pin_get_dt(&esp->data_ready);
    if (drdy) {
        esp->base.drv_state |= ESPH_DRV_STATE_DRDY;
        k_work_submit(&esp->trans_work);
    } else {
        esp->base.drv_state &= ~ESPH_DRV_STATE_DRDY;
    }
}

static void esph_spi_trans_work(struct k_work *work) {
    int ret;
    struct esph_spi_priv *esp =
        CONTAINER_OF(work, struct esph_spi_priv, trans_work);
    
    ret = esph_proto_process(&esp->base);
    if (ret) {
        LOG_ERR("Failed to process protocol: %d", ret);
    }
}

static int esph_spi_bus_transceive(struct esph_spi_priv *esp,
                void *odata, size_t olen,
                void *idata, size_t ilen)
{
    int ret;
    struct spi_buf spi_tx_buf = {
        .buf = odata,
        .len = olen,
    };
    struct spi_buf spi_rx_buf = {
        .buf = idata,
        .len = ilen,
    };
    struct spi_buf_set spi_tx_set = {
        .buffers = &spi_tx_buf,
        .count = 1,
    };
    struct spi_buf_set spi_rx_set = {
        .buffers = &spi_rx_buf,
        .count = 1,
    };

    ret = spi_transceive_dt(&esp->bus, &spi_tx_set, &spi_rx_set);

    return ret;
}

static int esph_spi_config_gpios(struct esph_spi_priv *esp) {
    int ret = 0;
    struct esph_priv *base = &esp->base;

    ret |= gpio_pin_configure_dt(&esp->handshake, GPIO_INPUT | GPIO_ACTIVE_HIGH);
    ret |= gpio_pin_configure_dt(&esp->data_ready, GPIO_INPUT | GPIO_ACTIVE_HIGH);
    gpio_init_callback(&esp->data_ready_cb,
        esph_spi_handle_data_ready_irq, BIT(esp->data_ready.pin));
    ret |= gpio_pin_interrupt_configure_dt(&esp->data_ready, GPIO_INT_EDGE_RISING);
    ret |= gpio_add_callback_dt(&esp->data_ready, &esp->data_ready_cb);
    
    gpio_init_callback(&esp->handshake_cb,
        esph_spi_handle_handshake_irq, BIT(esp->handshake.pin));
    ret |= gpio_add_callback_dt(&esp->handshake, &esp->handshake_cb);
    ret |= gpio_pin_interrupt_configure_dt(&esp->handshake, GPIO_INT_EDGE_RISING);

    ret = gpio_pin_configure_dt(&base->reset,
        GPIO_OUTPUT_ACTIVE | GPIO_ACTIVE_LOW);

    k_sleep(K_MSEC(1));

    ret |= gpio_pin_set_dt(&base->reset, 0);

    return ret;
}

static int esph_spi_bus_init(struct esph_spi_priv *esp) {
    k_work_init(&esp->trans_work, esph_spi_trans_work);
    
    return esph_spi_config_gpios(esp);
}

const struct esph_bus_ops __esph_spi_bus_ops = {
    .init = (int (*)(struct esph_priv *))esph_spi_bus_init,
};