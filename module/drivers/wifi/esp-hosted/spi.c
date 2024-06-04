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
    struct esph_spi_data *bus = CONTAINER_OF(cb, struct esph_spi_data, data_ready_cb);
    k_work_submit(&bus->trans_work);
}

static void esph_spi_handle_data_ready_irq(const struct device *port,
                struct gpio_callback *cb, gpio_port_pins_t pins)
{
    struct esph_spi_data *bus = CONTAINER_OF(cb, struct esph_spi_data, data_ready_cb);
    k_work_submit(&bus->trans_work);
}

static int esph_spi_reset(const struct device *esp) {
    int ret;
    const struct esph_config *config = esp->config;
    const struct esph_spi_bus *bus = (const struct esph_spi_bus *)config->bus_data;

    ret = gpio_pin_set_dt(&bus->resetn, 1);
    if (ret < 0) {
        goto out;
    }

    ret = gpio_pin_set_dt(&bus->resetn, 0);
    if (ret < 0) {
        goto out;
    }

    k_sleep(K_MSEC(300));

out:
    return ret;
}

static int esph_spi_read_header(const struct device *esp,
                struct esph_bus_trans_header *header)
{
    const struct esph_config *config = esp->config;
    const struct esph_spi_bus *bus = (const struct esph_spi_bus *)config->bus_data;
    struct esph_spi_data *data = esp->data;
    const struct spi_dt_spec *spi = &bus->bus;
    struct spi_buf rx_buf = {
        .buf = ((struct esph_data *)data)->bus_buf,
        .len = sizeof(((struct esph_data *)data)->bus_buf)
    };
    struct spi_buf_set rx = {
        .buffers = &rx_buf,
        .count = 1
    };

    return spi_read_dt(spi, &rx);
}

static void esph_spi_trans_work(struct k_work *work) {
    int ret;
    int handshake;
    int data_ready;
    struct esph_spi_data *data =
        CONTAINER_OF(work, struct esph_spi_data, trans_work);
    const struct device *esp = data->base.dev;
    const struct esph_config *config = esp->config;
    const struct esph_spi_bus *bus =
        (const struct esph_spi_bus *)config->bus_data;

    handshake = gpio_pin_get_dt(&bus->handshake);
    data_ready = gpio_pin_get_dt(&bus->data_ready);

    if (handshake == 1) {

    }
}

static int esph_spi_config_gpios(const struct device *esp) {
    int ret = 0;
    const struct esph_config *config = esp->config;
    struct esph_spi_data *data = (struct esph_spi_data *)esp->data;
    const struct esph_spi_bus *bus =
        (const struct esph_spi_bus *)config->bus_data;

    if (bus->data_ready.port == NULL ||
        bus->handshake.port == NULL ||
        bus->resetn.port == NULL)
    {
        ret = -ENODEV;
        goto out;
    } 

    ret |= gpio_pin_configure_dt(&bus->data_ready, GPIO_INPUT);
    ret |= gpio_pin_interrupt_configure_dt(&bus->data_ready, GPIO_INT_EDGE_RISING);
    gpio_init_callback(&data->data_ready_cb,
        esph_spi_handle_data_ready_irq, BIT(bus->data_ready.pin));
    ret |= gpio_add_callback(bus->data_ready.port, &data->data_ready_cb);

    ret |= gpio_pin_configure_dt(&bus->handshake, GPIO_OUTPUT_ACTIVE | GPIO_ACTIVE_LOW);
    ret |= gpio_pin_interrupt_configure_dt(&bus->handshake, GPIO_INT_EDGE_RISING);
    gpio_init_callback(&data->handshake_cb,
        esph_spi_handle_handshake_irq, BIT(bus->handshake.pin));
    ret |= gpio_add_callback(bus->handshake.port, &data->handshake_cb);

    ret |= gpio_pin_configure_dt(&bus->resetn, GPIO_OUTPUT_ACTIVE | GPIO_ACTIVE_LOW);

out:
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

    ret = esph_spi_reset(esp);

out:
    return ret;
}

struct esph_bus_ops __esph_spi_bus_ops = {
    .init = esph_spi_bus_init

};