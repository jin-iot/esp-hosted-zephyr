/*
 * Copyright (c) 2024 Jin (juanjin.dev@gmail.com)
 *
 * SPDX-License-Identifier: BSL-1.0
 */

#include "esph.h"


static void esph_proto_set_cksm(struct esph_bus_trans_header *hdr)
{
	uint16_t cksm = 0;

	for (unsigned idx = 0; idx < hdr->len; ++idx) {
		cksm += hdr->payload[idx];
	}

	hdr->cksm = cksm;
}

static inline void esph_proto_make_req(struct esph_bus_trans_header *hdr,
                uint8_t pkt_type)
{
    memset(hdr, 0, sizeof(*hdr));
    
    hdr->pkt_type = ESPH_PKT_TYPE_COMMAND_REQUEST;
    hdr->pkt_type = pkt_type;
}

void __esph_proto_make_scan(struct esph_bus_trans_header *hdr) {
    memset(hdr, 0, sizeof(*hdr));
    hdr->pkt_type = ESPH_PKT_TYPE_COMMAND_REQUEST;

}