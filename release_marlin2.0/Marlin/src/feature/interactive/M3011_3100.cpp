#include "../../inc/ANKER_Config.h"

#if ADAPT_DETACHED_NOZZLE

#include "../../gcode/gcode.h"
#include "../../inc/MarlinConfig.h"
#include "../../module/settings.h"
#include "../anker/anker_z_offset.h"
#include "M3011_3100.h"
#include <string.h>

#include "protocol.h"

#include "uart_nozzle_rx.h"
#include "uart_nozzle_tx.h"
#include <stdint.h>

// hw/sw version get, M3011 S1/S0
void GcodeSuite::M3011(void)
{
    uint8_t n = 0xFF;
    uint8_t data[32];
    uint32_t addr = 0x0800D000;

    if (parser.seenval('S'))
        n = parser.value_ushort();

    switch (n)
    {
    case 0: // get marlin boot version
        if (strstr((char *)addr, "ANKER"))
        {
            memcpy(data, (uint8_t *)addr, 23);
        }
        else
        {
            memcpy(data, (uint8_t *)"ANKER_V8110_BOOT_V0.0.0", 23);
        }
        data[23] = '\n';

        MYSERIAL2.send(data, 24);
        // MYSERIAL1.printLine("%s", data);
        break;

    case 1: // get nozzle hw&sw verion
        uart_nozzle_tx_hwsw_ver_get();
        MYSERIAL1.printLine("echo:M3011 uart_nozzle_tx_sw_ver_get\n");
        break;
    }
}

void rgb_led_ctrl(void)
{
    uint8_t n;

    if (parser.seenval('M'))
    {
        n = parser.value_ushort();
        uart_nozzle_tx_single_data(GCP_CMD_26_LOGO_LED, n);
        MYSERIAL2.printLine("echo:M3012 M%d\n", n);
    }
    else if (parser.seenval('W'))
    {
        n = parser.value_ushort();
        uart_nozzle_tx_single_data(GCP_CMD_27_DOT_LED, n);
        MYSERIAL2.printLine("echo:M3012 W%d\n", n);
    }
    else if (parser.seenval('C'))
    {
        n = parser.value_ushort();
        uart_nozzle_tx_single_data(GCP_CMD_F3_LED_TEST, n);
        MYSERIAL2.printLine("echo:M3012 C%d\n", n);
    }
    else if (parser.seenval('S'))
    {
        n = parser.value_ushort();
        uart_nozzle_tx_single_data(GCP_CMD_F0_LED_IO_CTRL, n);
        MYSERIAL2.printLine("echo:M3012 S%d\n", n);
    }
}

// rgb led control, M3012 S1/S0
void GcodeSuite::M3012(void)
{
    rgb_led_ctrl();
}

// sn w/r, M3013 W0123456789abcdef / M3013 R0
void GcodeSuite::M3013(void)
{
    uint8_t n = 0;
    uint8_t *ptr;

    ptr = (uint8_t *)strchr(parser.command_ptr, 'W');
    if (ptr != 0)
    {
        ptr += 1;
        uart_nozzle_tx_multi_data(GCP_CMD_29_SN_WRITE, ptr, 16);
        MYSERIAL1.printLine("echo:M3013 W%s\n", ptr);
        return;
    }

    ptr = (uint8_t *)strchr(parser.command_ptr, 'R');
    if (ptr != 0)
    {
        ptr += 1;
        n = ptr[0] - '0';
        uart_nozzle_tx_single_data(GCP_CMD_30_SN_READ, n);
        MYSERIAL1.printLine("echo:M3013 R%d\n", n);
    }
}

// material status get, M3014 Gx
void GcodeSuite::M3014(void)
{
    uint8_t n = 0;
    uint8_t *ptr;

    ptr = (uint8_t *)strchr(parser.command_ptr, 'G');
    if (ptr != 0)
    {
        ptr += 1;
        n = ptr[0] - '0';
        uart_nozzle_tx_multi_data(GCP_CMD_23_NO_MS, &n, 1);
        MYSERIAL1.printLine("echo:M3014 G%s\n", ptr);
        return;
    }
}

// heater fan io ctrl, M3015 Sx
void GcodeSuite::M3015(void)
{
    uint8_t n = 0;
    uint8_t *ptr;

    ptr = (uint8_t *)strchr(parser.command_ptr, 'S');
    if (ptr != 0)
    {
        ptr += 1;
        n = ptr[0] - '0';
        uart_nozzle_tx_multi_data(GCP_CMD_F4_HEATER_FAN_IO_CTRL, &n, 1);
        MYSERIAL1.printLine("echo:M3015 S%s\n", ptr);
        return;
    }
}

void GcodeSuite::M3020()
{
    if (parser.seen('V'))
    {
        const uint16_t leveing_value = parser.value_int();
        uart_nozzle_tx_probe_leveling_val_set(leveing_value);
        MYSERIAL2.printLine("echo:M3020 V%d\r\n", leveing_value);
        if(leveing_value !=  anker_probe_set.leveing_value){
            anker_probe_set.leveing_value = leveing_value;
            settings.save();
        }
        return;
    }
    uart_nozzle_tx_probe_leveling_val_get();
}

#endif
