# esp-hosted-zephyr
An ESP-Hosted-NG driver implementation for Zephyr

Details - https://github.com/espressif/esp-hosted

Things for people who implement IEEE 802.11 network drivers on Zephyr must read - https://docs.zephyrproject.org/latest/connectivity/networking/api/wifi.html

## esp-hosted structure

### Hardware setups

#### Common
 - reset GPIO - GPIO output/Active Low

#### SPI interface
 - SPI Mode 2 (CPOL only), default speed is 10MHz
 - data-ready GPIO - GPIO input/Active High/IRQ edge rising
 - handshake(Host TX okay) GPIO - GPIO input/Active High/IRQ edge rising

#### SDIO (SDHC) interface
 - fdsa

### esp-hosted data types
 -  <table>
        <tr>
            <td colspan="2">
                <b>struct esph_proto_hdr</b>
            </td>
            <td colspan="2">
                Main header of the esp-hosted protocol
            </td>
        </tr>
        <tr>
            <td><b>field</b></td>
            <td><b>type</b></td>
            <td><b>desc</b></td>
            <td><b>possible values</b></td>
        </tr>
        <tr>
            <td>if_type</td>
            <td>u4</td>
            <td>Interface type</td>
            <td>
                ESPH_IF_TYPE_STA<br>
                ESPH_IF_TYPE_AP<br>
                ESPH_IF_TYPE_HCI<br>
                ESPH_IF_TYPE_INTERNAL<br>
                ESPH_IF_TYPE_TEST<br>
            </td>
        </tr>
        <tr>
            <td>if_no</td>
            <td>u4</td>
            <td>Interface number</td>
            <td>0x0 or 0xF(Dummy indicator)</td>
        </tr>
        <tr>
            <td>flags</td>
            <td>u8</td>
            <td>Flags</td>
            <td>0x0 or 0xFF(Wakeup)</td>
        </tr>
        <tr>
            <td>pkt_type</td>
            <td>u8</td>
            <td>Packet type</td>
            <td>
                ESPH_PKT_TYPE_DATA<br>
                ESPH_PKT_TYPE_CMD_REQ<br>
                ESPH_PKT_TYPE_CMD_RES<br>
                ESPH_PKT_TYPE_EVT<br>
                ESPH_PKT_TYPE_EAPOL
            </td>
        </tr>
        <tr>
            <td><del>reserved1</del></td>
            <td>u8</td>
            <td><del>Reserved</del></td>
            <td>None</td>
        </tr>
        <tr>
            <td>len</td>
            <td>le16</td>
            <td>Payload length</td>
            <td>0 ~ le16 max</td>
        </tr>
        <tr>
            <td>offset</td>
            <td>le16</td>
            <td>Payload offset(?) I don't know yet</td>
            <td>Mostly size of this type</td>
        </tr>
        <tr>
            <td>cksm</td>
            <td>le16</td>
            <td>Checksum</td>
            <td>Sum of the whole payload buffer</td>
        </tr>
        <tr>
            <td><del>reserved2</del></td>
            <td>u8</td>
            <td><del>Reserved</del></td>
            <td>None</td>
        </tr>
        <tr>
            <td>
                reserved3<br>
                hci_pkt_type<br>
                priv_pkt_type
            </td>
            <td>union (u8)</td>
            <td>Multi-purpose data</td>
            <td>Not used yet</td>
        </tr>
    </table>

     -  <table>
            <tr>
                <td colspan="2">
                    <b>struct esph_proto_cmd_hdr</b>
                </td>
                <td colspan="2">
                    payload header for ESPH_PKT_TYPE_CMD_REQ and ESPH_PKT_TYPE_CMD_RES
                </td>
            </tr>
            <tr>
                <td><b>field</b></td>
                <td><b>type</b></td>
                <td><b>desc</b></td>
                <td><b>possible values</b></td>
            </tr>
            <tr>
                <td>cmd</td>
                <td>u8</td>
                <td>Command code</td>
                <td>
                    ESPH_PROTO_CMD_INIT_IF<br>
                    ESPH_PROTO_CMD_SET_MAC<br>
                    ESPH_PROTO_CMD_GET_MAC<br>
                    ESPH_PROTO_CMD_SCAN_REQ<br>
                    ESPH_PROTO_CMD_STA_CONNECT<br>
                    ESPH_PROTO_CMD_STA_DISCONNECT<br>
                    ESPH_PROTO_CMD_DEINIT_IF<br>
                    ESPH_PROTO_CMD_ADD_KEY<br>
                    ESPH_PROTO_CMD_DEL_KEY<br>
                    ESPH_PROTO_CMD_SET_DEFAULT_KEY<br>
                    ESPH_PROTO_CMD_STA_AUTH<br>
                    ESPH_PROTO_CMD_STA_ASSOC<br>
                    ESPH_PROTO_CMD_SET_IP_ADDR<br>
                    ESPH_PROTO_CMD_SET_MCAST_MAC_ADDR<br>
                    ESPH_PROTO_CMD_GET_TXPOWER<br>
                    ESPH_PROTO_CMD_SET_TXPOWER<br>
                    ESPH_PROTO_CMD_GET_REG_DOMAIN<br>
                    ESPH_PROTO_CMD_SET_REG_DOMAIN<br>
                    ESPH_PROTO_CMD_RAW_TP_ESP_TO_HOST<br>
                    ESPH_PROTO_CMD_RAW_TP_HOST_TO_ESP<br>
                    ESPH_PROTO_CMD_SET_WOW_CONFIG
                </td>
            </tr>
            <tr>
                <td>status</td>
                <td>u8</td>
                <td>Status code</td>
                <td>
                    ESPH_PROTO_CMD_STATUS_PENDING<br>
                    ESPH_PROTO_CMD_STATUS_FAIL<br>
                    ESPH_PROTO_CMD_STATUS_SUCCESS<br>
                    ESPH_PROTO_CMD_STATUS_BUSY<br>
                    ESPH_PROTO_CMD_STATUS_UNSUPPORTED<br>
                    ESPH_PROTO_CMD_STATUS_INVALID<br>
                </td>
            </tr>
            <tr>
                <td>len</td>
                <td>le16</td>
                <td>Command payload length</td>
                <td>0 ~ le16 max</td>
            </tr>
            <tr>
                <td>seq_no</td>
                <td>le16</td>
                <td>Sequence number</td>
                <td>0 ~ le16 max</td>
            </tr>
            <tr>
                <td><del>reserved1</del></td>
                <td>u8</td>
                <td><del>Reserved</del></td>
                <td>None</td>
            </tr>
            <tr>
                <td><del>reserved2</del></td>
                <td>u8</td>
                <td><del>Reserved</del></td>
                <td>None</td>
            </tr>
        </table>

     -  <table>
            <tr>
                <td colspan="2">
                    <b>struct esph_proto_evt_hdr</b>
                </td>
                <td colspan="2">
                    payload header for ESPH_PKT_TYPE_EVT
                </td>
            </tr>
            <tr>
                <td><b>field</b></td>
                <td><b>type</b></td>
                <td><b>desc</b></td>
                <td><b>possible values</b></td>
            </tr>
            <tr>
                <td>evt</td>
                <td>u8</td>
                <td>Event code</td>
                <td>
                    ESPH_PROTO_EVT_BOOTUP<br>
                    ESPH_PROTO_EVT_SCAN_RESULT<br>
                    ESPH_PROTO_EVT_STA_CONNECT<br>
                    ESPH_PROTO_EVT_STA_DISCONNECT<br>
                    ESPH_PROTO_EVT_AUTH_RX<br>
                    ESPH_PROTO_EVT_ASSOC_RX
                </td>
            </tr>
            <tr>
                <td>status</td>
                <td>u8</td>
                <td>Error status</td>
                <td>
                    0: No error,
                    else: Error
                </td>
            </tr>
            <tr>
                <td>len</td>
                <td>Event data length</td>
                <td>le16</td>
                <td>0 ~ le16 max</td>
            </tr>
        </table>

         -  <table>
                <tr>
                    <td colspan="2">struct esph_proto_evt_bootup</td>
                    <td colspan="2">payload for ESPH_PROTO_EVT_BOOTUP</td>
                </tr>
                <tr>
                    <td><b>field</b></td>
                    <td><b>type</b></td>
                    <td><b>desc</b></td>
                    <td><b>possible values</b></td>
                </tr>
            </table>

### Sequence
1. device init
    ```mermaid
    sequenceDiagram
    host->>host: reset pin setup
    host->>esp: reset pin 1 0
    host-->>host: sleep 300ms
    esp->>esp: reboot
    esp-->>host: proto_hdr(ESPH_IF_TYPE_INTERNAL + ) + evt_hdr + bootup_evt
    ```

2. netif init
    ```mermaid
    sequenceDiagram
    host->>esp: proto_hdr(ESPH_IF_TYPE_STA + ESPH_PKT_TYPE_CMD_REQ) + cmd_hdr(ESPH_PROTO_CMD_INIT_IF)
    esp->>esp: STA init
    esp-->>host: proto_hdr(ESPH_IF_TYPE_STA + ESPH_PKT_TYPE_CMD_RES) + cmd_hdr(ESPH_PROTO_CMD_INIT_IF)
    ```