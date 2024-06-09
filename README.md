# esp-hosted-zephyr
esp-hosted-ng driver for Zephyr

details - https://github.com/espressif/esp-hosted

## esp-hosted structure

### esp-hosted data types
 -  <table>
        <tr>
            <td colspan="3">
                <b>struct esph_proto_hdr</b>
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
            <td>Interface type</td>
            <td>0x0 or 0xF({Wakeup on FourWayHandshake}, if using SDIO)</td>
        </tr>
        <tr>
            <td>flags</td>
            <td>u8</td>
            <td>Flags</td>
            <td>any</td>
        </tr>
        <tr>
            <td>pkt_type</td>
            <td>u8</td>
            <td>Packet type</td>
        </tr>
        <tr>
            <td><del>reserved1</del></td>
            <td>u8</td>
            <td><del>Reserved</del></td>
        </tr>
        <tr>
            <td>len</td>
            <td>u16</td>
            <td>Payload length</td>
        </tr>
        <tr>
            <td>offset</td>
            <td>u16</td>
            <td>Payload offset</td>
        </tr>
        <tr>
            <td>cksm</td>
            <td>u16</td>
            <td>Checksum</td>
        </tr>
        <tr>
            <td><del>reserved2</del></td>
            <td>u8</td>
            <td><del>Reserved</del></td>
        </tr>
        <tr>
            <td>
                reserved3<br>
                hci_pkt_type<br>
                priv_pkt_type
            </td>
            <td>union (u8)</td>
            <td>Multi-purpose data</td>
        </tr>
    </table>

 -  <table>
        <tr>
            <td colspan="3">
                <b>enum esph_pkt_type</b>
            </td>
        </tr>
        <tr><td>ESPH_PKT_TYPE_DATA</td></tr>
        <tr><td>ESPH_PKT_TYPE_COMMAND_REQUEST</td></tr>
        <tr><td>ESPH_PKT_TYPE_COMMAND_RESPONSE</td></tr>
        <tr><td>ESPH_PKT_TYPE_EVENT</td></tr>
        <tr><td>ESPH_PKT_TYPE_EAPOL</td></tr>
    </table>

-   <table>
        <tr>
            <td colspan="3">
                <b>enum esph_pkt_type</b>
            </td>
        </tr>
        <tr><td>ESPH_PKT_TYPE_DATA</td></tr>
        <tr><td>ESPH_PKT_TYPE_COMMAND_REQUEST</td></tr>
        <tr><td>ESPH_PKT_TYPE_COMMAND_RESPONSE</td></tr>
        <tr><td>ESPH_PKT_TYPE_EVENT</td></tr>
        <tr><td>ESPH_PKT_TYPE_EAPOL</td></tr>
    </table>

### Sequence
1. init
    ```mermaid
    sequenceDiagram
    host->>host: setup
    host->>esp: reset pin 1 0
    esp->>esp: reboot
    esp-->>host: proto_hdr + evt_hdr + bootup_evt
    ```