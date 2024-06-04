/*
 * Copyright (c) 2024 Jin (juanjin.dev@gmail.com)
 *
 * SPDX-License-Identifier: BSL-1.0
 */

#define DT_DRV_COMPAT espressif_esp_hosted

#include "esph.h"

#include <zephyr/net/ethernet.h>
#include <zephyr/net/net_pkt.h>
#include <zephyr/net/net_if.h>
#include <zephyr/net/wifi_mgmt.h>
#include <zephyr/device.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(wifi_esp_hosted, CONFIG_WIFI_LOG_LEVEL);

static int esph_wifi_send(const struct device *esp, struct net_pkt *pkt) {
    int ret;

    goto out;
out:
    return ret;
}

static int esph_scan(const struct device *esp,
			   struct wifi_scan_params *params,
			   scan_result_cb_t cb)
{
    int ret;

    goto out;

out:
    return ret;
}

static int esph_connect(const struct device *esp,
			    struct wifi_connect_req_params *rq_params)
{
    int ret;
    struct esph_data *data = esp->data;
    
    

out:
    return ret;
}

static int esph_disconnect(const struct device *esp) {
	int ret;

    goto out;

out:
	return ret;
}

static int esph_ap_enable(const struct device *esp,
			    struct wifi_connect_req_params *rq_params)
{
    int ret;

    goto out;

out:
	return ret;
}

static int esph_ap_disable(const struct device *esp) {
    int ret;

    goto out;

out:
	return ret;
}

static int esph_iface_status(const struct device *esp,
                struct wifi_iface_status *status)
{
    int ret;

    goto out;

out:
	return ret;
}

static int esph_dev_init(const struct device *esp) {
    int ret;
    struct esph_data *data = esp->data;
    data->dev = esp;

    ret = esph_bus_init(esp);
    if (ret < 0) {
        LOG_ERR("Failed to initialize bus");
        goto out;
    }
    
out:
    return ret;
}

static const struct wifi_mgmt_ops esph_mgmt_api = {
    .scan = esph_scan,
    .connect = esph_connect,
    .disconnect = esph_disconnect,
    .ap_enable = esph_ap_enable,
    .ap_disable = esph_ap_disable,
    .iface_status = esph_iface_status,
#if defined(CONFIG_NET_STATISTICS_WIFI)
    .get_stats = esph_get_stats
#endif // CONFIG_NET_STATISTICS_WIFI
};

static void esph_iface_init(struct net_if *iface) {

}

static const struct net_wifi_mgmt_offload esph_api = {
	.wifi_iface.iface_api.init = esph_iface_init,
	.wifi_mgmt_api = &esph_mgmt_api,
};

#define ESPH_INIT(inst) \
        COND_CODE_1(DT_INST_ON_BUS(inst, spi), \
            (ESPH_DEFINE_SPI_BUS(inst)), \
            ( \
                COND_CODE_1(DT_INST_ON_BUS(inst, sdio), \
                    (ESPH_DEFINE_SDIO_BUS(inst)), \
                    (_Static_assert(0, "Unsupported bus type")) \
                ) \
            ) \
        ) \
        static struct esph_config esph_config_##inst = { \
            .is_spi = DT_INST_ON_BUS(inst, spi), \
            .bus_data = (struct esph_bus_data *)&esph_bus_data_##inst, \
        }; \
        NET_DEVICE_OFFLOAD_INIT(esp_hosted_##inst, "esp-hosted"#inst,  \
            esph_dev_init, NULL, \
            &esph_data_##inst, &esph_config_##inst, \
            CONFIG_WIFI_INIT_PRIORITY, &esph_api, NET_ETH_MTU);

DT_INST_FOREACH_STATUS_OKAY(ESPH_INIT)