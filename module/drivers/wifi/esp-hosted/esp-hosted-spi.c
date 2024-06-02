#include "esp-hosted.h"

#include <zephyr/drivers/spi.h>

static void esph_spi_handle_handshake_irq(const struct device *port,
                struct gpio_callback *cb, gpio_port_pins_t pins)
{
    struct esph_spi_data *bus = CONTAINER_OF(cb, struct esph_spi_data, handshake_cb);
    k_work_submit(&bus->handshake_work);
}

static void esph_spi_handle_data_ready_irq(const struct device *port,
                struct gpio_callback *cb, gpio_port_pins_t pins)
{
    struct esph_spi_data *bus = CONTAINER_OF(cb, struct esph_spi_data, data_ready_cb);
    k_work_submit(&bus->data_ready_work);
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

    k_sleep(K_MSEC(100));

out:
    return ret;
}

static void esph_spi_handshake_work(struct k_work *work) {

}

static void esph_spi_data_ready_work(struct k_work *work) {
    int ret;
    int handshake;
    int data_ready;
    struct esph_spi_data *data = CONTAINER_OF(work, struct esph_spi_data, data_ready_work);
    const struct device *dev = data->base.dev;
    const struct esph_config *config = dev->config;
    const struct esph_spi_bus *bus = (const struct esph_spi_bus *)config->bus_data;

    handshake = gpio_pin_get_dt(&bus->handshake);
    data_ready = gpio_pin_get_dt(&bus->data_ready);
    
}

static int esph_spi_config_gpios(const struct device *esp) {
    int ret = 0;
    const struct esph_config *config = esp->config;
    struct esph_spi_data *data = (struct esph_spi_data *)esp->data;
    const struct esph_spi_bus *bus = (const struct esph_spi_bus *)config->bus_data;

    if (bus->data_ready.port == NULL || bus->handshake.port == NULL || bus->resetn.port == NULL) {
        ret = -ENODEV;
        goto out;
    } 

    ret |= gpio_pin_configure_dt(&bus->data_ready, GPIO_INPUT);
    ret |= gpio_pin_interrupt_configure_dt(&bus->data_ready, GPIO_INT_EDGE_FALLING);
    gpio_init_callback(&data->data_ready_cb,
        esph_spi_handle_data_ready_irq, BIT(bus->data_ready.pin));

    ret |= gpio_pin_configure_dt(&bus->handshake, GPIO_OUTPUT_ACTIVE | GPIO_ACTIVE_LOW);
    ret |= gpio_pin_interrupt_configure_dt(&bus->handshake, GPIO_INT_EDGE_FALLING);
    gpio_init_callback(&data->handshake_cb,
        esph_spi_handle_handshake_irq, BIT(bus->handshake.pin));

    ret |= gpio_pin_configure_dt(&bus->resetn, GPIO_OUTPUT_ACTIVE | GPIO_ACTIVE_LOW);
    
    if (ret < 0) {
        goto out;
    }

out:
    return ret;
}

static int esph_spi_bus_init(const struct device *esp) {
    int ret;
    struct esph_spi_data *data = (struct esph_spi_data *)esp->data;

    k_work_init(&data->data_ready_work, esph_spi_data_ready_work);
    k_work_init(&data->handshake_work, esph_spi_handshake_work);

    ret = esph_spi_config_gpios(esp);
    if (ret < 0) {
        goto out;
    }

    ret = esph_spi_reset(esp);
    if (ret < 0) {
        goto out;
    }

out:
    return ret;
}

struct esph_bus_ops __esph_spi_bus_ops = {
    .init = esph_spi_bus_init
};