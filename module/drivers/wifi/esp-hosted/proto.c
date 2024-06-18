/*
 * Copyright (c) 2024 Jin (juanjin.dev@gmail.com)
 *
 * SPDX-License-Identifier: BSL-1.0
 */

#include "esph.h"

#include <zephyr/net/net_ip.h>

/**
 * @brief Protocol header
 */

struct esph_proto_hdr {
    uint8_t if_type : 4;
#define ESPH_IF_TYPE_STA      (uint8_t)0x0
#define ESPH_IF_TYPE_AP       (uint8_t)0x1
#define ESPH_IF_TYPE_HCI      (uint8_t)0x2
#define ESPH_IF_TYPE_INTERNAL (uint8_t)0x3
#define ESPH_IF_TYPE_TEST     (uint8_t)0x4
    uint8_t if_no : 4;
    uint8_t flags;
    uint8_t pkt_type;
#define ESPH_PKT_TYPE_DATA    (uint8_t)0x00
#define ESPH_PKT_TYPE_CMD_REQ (uint8_t)0x01
#define ESPH_PKT_TYPE_CMD_RES (uint8_t)0x02
#define ESPH_PKT_TYPE_EVT     (uint8_t)0x03
#define ESPH_PKT_TYPE_EAPOL   (uint8_t)0x04
    uint8_t reserved1;
    uint16_t len;
    uint16_t offset;
    uint16_t cksm;
    uint8_t reserved2;
    union {
        uint8_t reserved3;
        uint8_t hci_pkt_type;
        uint8_t priv_pkt_type;
    };
} __packed;

/**
 * @brief Command header
 */
struct esph_proto_cmd_hdr {
    uint8_t cmd;
#define ESPH_PROTO_CMD_INIT_IF            (uint8_t)0x01
#define ESPH_PROTO_CMD_SET_MAC            (uint8_t)0x02
#define ESPH_PROTO_CMD_GET_MAC            (uint8_t)0x03
#define ESPH_PROTO_CMD_SCAN_REQ           (uint8_t)0x04
#define ESPH_PROTO_CMD_STA_CONNECT        (uint8_t)0x05
#define ESPH_PROTO_CMD_STA_DISCONNECT     (uint8_t)0x06
#define ESPH_PROTO_CMD_DEINIT_IF          (uint8_t)0x07
#define ESPH_PROTO_CMD_ADD_KEY            (uint8_t)0x08
#define ESPH_PROTO_CMD_DEL_KEY            (uint8_t)0x09
#define ESPH_PROTO_CMD_SET_DEFAULT_KEY    (uint8_t)0x0A
#define ESPH_PROTO_CMD_STA_AUTH           (uint8_t)0x0B
#define ESPH_PROTO_CMD_STA_ASSOC          (uint8_t)0x0C
#define ESPH_PROTO_CMD_SET_IP_ADDR        (uint8_t)0x0D
#define ESPH_PROTO_CMD_SET_MCAST_MAC_ADDR (uint8_t)0x0E
#define ESPH_PROTO_CMD_GET_TXPOWER        (uint8_t)0x0F
#define ESPH_PROTO_CMD_SET_TXPOWER        (uint8_t)0x10
#define ESPH_PROTO_CMD_GET_REG_DOMAIN     (uint8_t)0x11
#define ESPH_PROTO_CMD_SET_REG_DOMAIN     (uint8_t)0x12
#define ESPH_PROTO_CMD_RAW_TP_ESP_TO_HOST (uint8_t)0x13
#define ESPH_PROTO_CMD_RAW_TP_HOST_TO_ESP (uint8_t)0x14
#define ESPH_PROTO_CMD_SET_WOW_CONFIG     (uint8_t)0x15
    uint8_t status;
#define ESPH_PROTO_CMD_STATUS_PENDING     (uint8_t)0x01
#define ESPH_PROTO_CMD_STATUS_FAIL        (uint8_t)0x02
#define ESPH_PROTO_CMD_STATUS_SUCCESS     (uint8_t)0x03
#define ESPH_PROTO_CMD_STATUS_BUSY        (uint8_t)0x04
#define ESPH_PROTO_CMD_STATUS_UNSUPPORTED (uint8_t)0x05
#define ESPH_PROTO_CMD_STATUS_INVALID     (uint8_t)0x06
    uint16_t len;
    uint16_t seq_no;
    uint8_t reserved1;
	uint8_t reserved2;
} __packed;

/**
 * @brief Event header
 */
struct esph_proto_evt_hdr {
    uint8_t evt;
#define ESPH_PROTO_EVT_BOOTUP (uint8_t)1U
#define ESPH_PROTO_EVT_SCAN_RESULT (uint8_t)1U
#define ESPH_PROTO_EVT_STA_CONNECT (uint8_t)2U
#define ESPH_PROTO_EVT_STA_DISCONNECT (uint8_t)3U
#define ESPH_PROTO_EVT_AUTH_RX (uint8_t)4U
#define ESPH_PROTO_EVT_ASSOC_RX (uint8_t)5U
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

static inline void esph_proto_make_req(struct esph_proto_hdr *hdr,
                uint8_t cmd_code)
{
    memset(hdr, 0, sizeof(*hdr));

    hdr->pkt_type = ESPH_PKT_TYPE_CMD_REQ;
    hdr->if_type = ESPH_IF_TYPE_STA;
    hdr->len = sys_cpu_to_le16(sizeof(*hdr));
    hdr->offset = sys_cpu_to_le16(sizeof(*hdr));
    // hdr->cksm = sys_cpu_to_le16(esph_proto_compute_cksm(hdr));
}

void __esph_proto_make_scan(struct esph_proto_hdr *hdr) {
    memset(hdr, 0, sizeof(*hdr));
    

}