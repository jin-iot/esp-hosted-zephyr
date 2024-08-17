#define DT_DRV_COMPAT espressif_esp_hosted_wifi

#include <zephyr/net/wifi_mgmt.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(wifi_esp_hosted);

struct esp_hosted_wifi {
    const struct device *bus;
    const struct device *wdev;
    struct ethernet_config cfg;
    struct net_if *iface;
};

static int esp_hosted_wm_scan(const struct device *esp,
			   struct wifi_scan_params *params,
			   scan_result_cb_t cb)
{
    return 0;
}

static int esp_hosted_wm_connect(const struct device *esp,
			    struct wifi_connect_req_params *rq_params)
{
    return 0;
}

static int esp_hosted_wm_disconnect(const struct device *esp) {
	return 0;
}

static int esp_hosted_wm_ap_enable(const struct device *esp,
			    struct wifi_connect_req_params *rq_params)
{
    return 0;
}

static int esp_hosted_wm_ap_disable(const struct device *esp) {
    return 0;
}

static int esp_hosted_wm_status(const struct device *esp,
                struct wifi_iface_status *status)
{
    return 0;
}

static const struct wifi_mgmt_ops esp_hosted_wm_ops = {
    .scan = esp_hosted_wm_scan,
    .connect = esp_hosted_wm_connect,
    .disconnect = esp_hosted_wm_disconnect,
    .ap_enable = esp_hosted_wm_ap_enable,
    .ap_disable = esp_hosted_wm_ap_disable,
    .iface_status = esp_hosted_wm_status,
#if defined(CONFIG_NET_STATISTICS_WIFI)
    .get_stats = esp_hosted_wm_get_stats
#endif // CONFIG_NET_STATISTICS_WIFI
};

static void esp_hosted_if_init(struct net_if *iface) {
    const struct device *dev = net_if_get_device(iface);
    struct esp_hosted_wifi *wifi = dev->data;
    wifi->iface = iface;
}

static enum ethernet_hw_caps esp_hosted_if_get_caps(
    const struct device *net_dev)
{
    return (enum ethernet_hw_caps)0;
}

static int esp_hosted_if_get_config(const struct device *net_dev,
    enum ethernet_config_type type, struct ethernet_config *config)
{
    int ret;

out:
    return ret;
}

static int esp_hosted_if_set_config(const struct device *net_dev,
    enum ethernet_config_type type, const struct ethernet_config *config)
{
    int ret;

out:
    return ret;
}

static int esp_hosted_if_send(const struct device *dev,
                            struct net_pkt *pkt)
{
    int ret;

    goto out;
out:
    return ret;
}

static int esp_hosted_if_start(const struct device *dev)
{
    int ret;

    goto out;
out:
    return ret;
}

static int esp_hosted_if_stop(const struct device *dev)
{
    int ret;

    goto out;
out:
    return ret;
}

static const struct net_wifi_mgmt_offload esp_hosted_wifi_api = {
	.wifi_iface = {
        .iface_api = {
            .init = esp_hosted_if_init,
        },
        .get_capabilities = esp_hosted_if_get_caps,
        .get_config = esp_hosted_if_get_config,
        .set_config = esp_hosted_if_set_config,
        .send = esp_hosted_if_send,
        .start = esp_hosted_if_start,
        .stop = esp_hosted_if_stop,
    },
	.wifi_mgmt_api = &esp_hosted_wm_ops,
};

static int esp_hosted_wifi_init(const struct device *wifi_dev) {
    struct esp_hosted_wifi *wifi = wifi_dev->data;
    wifi->wdev = wifi_dev;

    if (wifi->bus == NULL || device_is_ready(wifi->bus) == false) {
        LOG_ERR("Bus device not ready");
        return -ENODEV;
    }

    return 0;
}

#define ESP_HOSTED_WIFI_INIT(_inst) \
        COND_CODE_1(DT_INST_ON_BUS(_inst, esp_hosted), \
            ( \
                static struct esp_hosted_wifi esp_hosted_wifi ## _inst = { \
                    .bus = DEVICE_DT_GET(DT_INST_BUS(_inst)), \
                }; \
                NET_DEVICE_OFFLOAD_INIT(esp_hosted_wifi ## _inst, "esp-hosted-wifi" # _inst, \
                    esp_hosted_wifi_init, NULL, \
                    &esp_hosted_wifi ## _inst, NULL, \
                    CONFIG_WIFI_INIT_PRIORITY, &esp_hosted_wifi_api, NET_ETH_MTU); \
            ), \
            ( \
                BUILD_ASSERT("Unsupported bus type"); \
            ) \
        )

DT_INST_FOREACH_STATUS_OKAY(ESP_HOSTED_WIFI_INIT)