/*
 * Copyright (c) 2024 Jin (juanjin.dev@gmail.com)
 *
 * SPDX-License-Identifier: BSL-1.0
 */

#include "esph.h"

#include <zephyr/net/net_ip.h>

/**
 * @brief Command header
 */
struct esph_proto_cmd_hdr {
    uint8_t cmd_code;
    uint8_t cmd_status;
    uint16_t len;
    uint16_t seq_no;
    uint8_t reserved1;
	uint8_t reserved2;
} __packed;

/**
 * @brief Event header
 */
struct esph_proto_evt_hdr {
#define ESPH_PROTO_EVT_BOOTUP (uint8_t)1U
#define ESPH_PROTO_EVT_SCAN_RESULT (uint8_t)1U
#define ESPH_PROTO_EVT_STA_CONNECT (uint8_t)2U
#define ESPH_PROTO_EVT_STA_DISCONNECT (uint8_t)3U
#define ESPH_PROTO_EVT_AUTH_RX (uint8_t)4U
#define ESPH_PROTO_EVT_ASSOC_RX (uint8_t)5U
    uint8_t evt_code;
	uint8_t status;
	uint16_t len;
} __packed;

struct esph_proto_evt_scan {
    struct esph_proto_evt_hdr base;
	uint8_t bssid[6];
	uint8_t frame_type;
	uint8_t channel;
	uint32_t rssi;
	uint64_t tsf;
	uint16_t frame_len;
	uint8_t pad[2];
	uint8_t frame[];
} __packed;

struct esph_proto_evt_assoc {
	struct esph_proto_evt_hdr base;
	uint8_t bssid[6];
	uint8_t frame_type;
	uint8_t channel;
	uint8_t ssid[WIFI_SSID_MAX_LEN + 1];
	uint8_t pad[1];
	uint16_t frame_len;
	uint32_t rssi;
	uint64_t tsf;
	uint8_t frame[];
} __packed;

struct esph_proto_evt_bootup {
    struct esph_proto_evt_hdr base;
	uint8_t len;
	uint8_t pad[3];
	uint8_t data[];
} __packed;

struct esph_proto_evt_disconn {
    struct esph_proto_evt_hdr base;
	uint8_t bssid[6];
	uint8_t ssid[WIFI_SSID_MAX_LEN + 1];
	uint8_t reason;
} __packed;

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

enum esph_pkt_type {
	ESPH_PKT_TYPE_DATA,
	ESPH_PKT_TYPE_COMMAND_REQUEST,
	ESPH_PKT_TYPE_COMMAND_RESPONSE,
	ESPH_PKT_TYPE_EVENT,
	ESPH_PKT_TYPE_EAPOL,
};

static inline void esph_proto_make_req(struct esph_proto_hdr *hdr,
                uint8_t cmd_code)
{
    memset(hdr, 0, sizeof(*hdr));

    hdr->pkt_type = ESPH_PKT_TYPE_COMMAND_REQUEST;
    hdr->if_type = ESPH_IF_TYPE_STA;
    hdr->len = sys_cpu_to_le16(sizeof(*hdr));
    hdr->offset = sys_cpu_to_le16(sizeof(*hdr));
    // hdr->cksm = sys_cpu_to_le16(esph_proto_compute_cksm(hdr));
}

void __esph_proto_make_scan(struct esph_proto_hdr *hdr) {
    memset(hdr, 0, sizeof(*hdr));
    

}