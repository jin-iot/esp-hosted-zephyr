/*
 * Copyright (c) 2024 Jin (juanjin.dev@gmail.com)
 *
 * SPDX-License-Identifier: BSL-1.0
 */

#include "esph.h"

#include <zephyr/drivers/spi.h>

#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(wifi_esp_hosted);

static void esph_spi_handle_handshake_irq(const struct device *port,
                struct gpio_callback *cb, gpio_port_pins_t pins)
{
    struct esph_spi_data *data =
        CONTAINER_OF(cb, struct esph_spi_data, data_ready_cb);
    k_work_submit(&data->trans_work);
}

static void esph_spi_handle_data_ready_irq(const struct device *port,
                struct gpio_callback *cb, gpio_port_pins_t pins)
{
    struct esph_spi_data *data =
        CONTAINER_OF(cb, struct esph_spi_data, data_ready_cb);
    k_work_submit(&data->trans_work);
}

static int esph_spi_read_data(const struct device *esp,
                        struct esph_proto_hdr *header)
{
    // return spi_read_dt(spi, &rx);
    return 0;
}

static int esph_spi_process(const struct device *esp) {
    int handshake;
    int data_ready;
    int ret = -EAGAIN;

    const struct esph_spi_config *cfg = esp->config;

    handshake = gpio_pin_get_dt(&cfg->handshake);
    data_ready = gpio_pin_get_dt(&cfg->data_ready);

    if (handshake == 1) {

    }

    return ret;
}

static void esph_spi_trans_work(struct k_work *work) {
    int ret;
    struct esph_spi_data *data =
        CONTAINER_OF(work, struct esph_spi_data, trans_work);
    const struct device *esp = data->base.dev;
    ret = esph_spi_process(esp);
    if (ret < 0) {
        LOG_WARN("Failed to process SPI data: %d", ret);
    }
}

static int esph_spi_config_gpios(const struct device *esp) {
    int ret = 0;
    const struct esph_spi_config *cfg = esp->config;
    struct esph_spi_data *data = esp->data;

    ret |= gpio_pin_configure_dt(&cfg->data_ready, GPIO_INPUT);
    ret |= gpio_pin_interrupt_configure_dt(&cfg->data_ready, GPIO_INT_EDGE_RISING);
    gpio_init_callback(&data->data_ready_cb,
        esph_spi_handle_data_ready_irq, BIT(cfg->data_ready.pin));
    ret |= gpio_add_callback_dt(&cfg->data_ready, &data->data_ready_cb);

    ret |= gpio_pin_configure_dt(&cfg->handshake, GPIO_OUTPUT_ACTIVE | GPIO_ACTIVE_LOW);
    ret |= gpio_pin_interrupt_configure_dt(&cfg->handshake, GPIO_INT_EDGE_RISING);
    gpio_init_callback(&data->handshake_cb,
        esph_spi_handle_handshake_irq, BIT(cfg->handshake.pin));
    ret |= gpio_add_callback_dt(&cfg->handshake, &data->handshake_cb);

    return ret;
}

static int esph_spi_bus_init(const struct device *esp) {
    int ret;
    struct esph_spi_data *data = (struct esph_spi_data *)esp->data;

    k_work_init(&data->trans_work, esph_spi_trans_work);

    ret = esph_spi_config_gpios(esp);
    if (ret < 0) {
        goto out;
    }

    ret = esph_reset(esp);

out:
    return ret;
}

struct esph_bus_ops __esph_spi_bus_ops = {
    .init = esph_spi_bus_init,
    .process = esph_spi_process,
};