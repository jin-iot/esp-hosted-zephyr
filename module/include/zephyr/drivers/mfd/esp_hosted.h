#ifndef ZEPHYR_INCLUDE_DRIVERS_MFD_ESP_HOSTED_H_
#define ZEPHYR_INCLUDE_DRIVERS_MFD_ESP_HOSTED_H_

#include <stdbool.h>

struct esph_priv;

bool mfd_esph_tx_is_ready(const struct esph_priv *priv);

#endif /* ZEPHYR_INCLUDE_DRIVERS_MFD_ESP_HOSTED_H_ */