#define DT_DRV_COMPAT espressif_hci_esp_hosted

#include <zephyr/kernel.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(hci_esp_hosted);

struct hci_esp_hosted {
    const struct device *bus;

};