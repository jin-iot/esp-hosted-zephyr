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

static int esph_wifi_scan(const struct device *esp,
			   struct wifi_scan_params *params,
			   scan_result_cb_t cb)
{
    return 0;
}

static int esph_wifi_connect(const struct device *esp,
			    struct wifi_connect_req_params *rq_params)
{
    int ret = 0;
    goto out;

out:
    return ret;
}

static int esph_wifi_disconnect(const struct device *esp) {
	return 0;
}

static int esph_wifi_ap_enable(const struct device *esp,
			    struct wifi_connect_req_params *rq_params)
{
    return 0;
}

static int esph_wifi_ap_disable(const struct device *esp) {
    return 0;
}

static int esph_wifi_iface_status(const struct device *esp,
                struct wifi_iface_status *status)
{
    return 0;
}

#if defined(CONFIG_NET_STATISTICS_WIFI)
static int esph_wifi_get_stats(const struct device *dev,
                struct net_stats_wifi *stats)
{
    return 0;
}
#endif // CONFIG_NET_STATISTICS_WIFI

static const struct wifi_mgmt_ops esph_mgmt_api = {
    .scan = esph_wifi_scan,
    .connect = esph_wifi_connect,
    .disconnect = esph_wifi_disconnect,
    .ap_enable = esph_wifi_ap_enable,
    .ap_disable = esph_wifi_ap_disable,
    .iface_status = esph_wifi_iface_status,
#if defined(CONFIG_NET_STATISTICS_WIFI)
    .get_stats = esph_wifi_get_stats
#endif // CONFIG_NET_STATISTICS_WIFI
};

static void esph_if_init(struct net_if *iface) {
    const struct device *dev = net_if_get_device(iface);
    struct esph_priv *esp = dev->data;
    esp->iface = iface;
}

static int esph_wifi_iface_enable(
                const struct net_if *iface,
                bool enable)
{
    return 0;
}

enum offloaded_net_if_types esph_wifi_iface_get_type() {
    return L2_OFFLOADED_NET_IF_TYPE_WIFI;
}

static const struct net_wifi_mgmt_offload esph_api = {
	.wifi_iface = {
        .iface_api = {
            .init = esph_if_init,
        },
        .enable = esph_wifi_iface_enable,
        .get_type = esph_wifi_iface_get_type,
    },
	.wifi_mgmt_api = &esph_mgmt_api,
};

static int esph_dev_init(const struct device *dev) {
    int ret;
    struct esph_priv *esp = dev->data;
    esp->esp_dev = dev;
    esp->drv_state = ESPH_DRV_STATE_DEV_INIT;

    sys_slist_init(&esp->pending_tx);

    ret = k_mutex_init(&esp->gp_mutex);
    if (ret < 0) {
        LOG_ERR("Failed to initialize mutex: %d", ret);
        goto out;
    }

    ret = k_condvar_init(&esp->gp_cond);
    if (ret < 0) {
        LOG_ERR("Failed to initialize condvar: %d", ret);
        goto out;
    }
    
    ret = esph_bus_init(esp);

    k_condvar_wait(&esp->gp_cond, &esp->gp_mutex, K_FOREVER);

out:
    return ret;
}

#define ESPH_INIT(_inst) \
        COND_CODE_1(DT_INST_ON_BUS(_inst, spi), \
            (ESPH_DEFINE_SPI_BUS(_inst)), \
            ( \
                COND_CODE_1(DT_INST_ON_BUS(_inst, sdhc), \
                    (ESPH_DEFINE_SDIO_BUS(_inst)), \
                    (BUILD_ASSERT(0, "Unsupported bus type")) \
                ) \
            ) \
        ) \
        NET_DEVICE_OFFLOAD_INIT(esp_hosted_ ## _inst, "esp-hosted" # _inst,  \
            esph_dev_init, NULL, \
            &esph_priv ## _inst, NULL, \
            CONFIG_WIFI_INIT_PRIORITY, &esph_api, NET_ETH_MTU);

DT_INST_FOREACH_STATUS_OKAY(ESPH_INIT)