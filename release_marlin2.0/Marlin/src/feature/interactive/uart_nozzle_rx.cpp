#include "../../inc/ANKER_Config.h"

#if ADAPT_DETACHED_NOZZLE
#include "../../inc/MarlinConfig.h"
#include "../../module/temperature.h"
#include "../anker/anker_z_offset.h"
#include "clock.h"

#include "../../module/endstops.h"
#include "../../module/planner.h"
#include "../../feature/anker/anker_homing.h"

extern "C"
{

#include "uart_nozzle_rx.h"

#include <stdint.h>

#include "gcp.h"
#include "gcp_parser.h"
#include "protocol.h"
#include "soft_timer.h"
#include "uart.h"
#include "oci.h"

    typedef struct
    {
        int32_t init_value;
        int32_t cur_value;
    } cs1237_test_t;

    cs1237_test_t cs1237_test;

    typedef struct
    {
        uint8_t setup;
        uint32_t sys_err;
        uint16_t rst_flg;
        uint16_t adc_raw;
        uint16_t adc_ave;
        uint16_t temp_now;
        uint16_t temp_tg;
        uint16_t pidout;
        int16_t probe_thres;
        uint32_t oci_ttl_cnt;
    } uart_nozzle_info_t;

    static uart_nozzle_info_t uart_nozzle_info;

#define PACK_LE_32(p) (*(p + 3) << 24 | *(p + 2) << 16 | *(p + 1) << 8 | *(p))
#define PACK_BE_32(p) (*(p + 3) | *(p + 2) << 8 | *(p + 1) << 16 | *(p) << 24)
#define PACK_LE_16(p) (*(p + 1) << 8 | *(p))
#define PACK_BE_16(p) (*(p + 1) | *(p) << 8)

    typedef struct
    {
        soft_timer_t soft_timer;
        gcp_parser_t parser;
        uint8_t uart_buffer[1056];
        uint8_t rx_buffer[1056];

        uint32_t tx_cnt;
        uint32_t rx_cnt;

        volatile uint32_t start_ms;

        uint8_t mos_err_flg;
        uint32_t mos_err_start_ms;
    } uart_nozzle_rx_t;

    static uart_nozzle_rx_t uart_nozzle_rx;

    nozzle_t nozzle;
    static uint8_t nozzle_rst_cnt;

    static void uart_nozzle_tx_temperature_polling_callback(void)
    {
        uint8_t content[8];
        uint8_t buf[64];
        gcp_msg_t *gcp_msg = (gcp_msg_t *)&buf[0];
        uint16_t len;

        content[0] = 1;
        content[1] = thermalManager.degTargetHotend(0) & 0xFF;
        content[2] = thermalManager.degTargetHotend(0) >> 8;
        content[3] = thermalManager.fan_speed[0] & 0xFF;
        content[4] = thermalManager.fan_speed[0] >> 8;
        len = gcp_msg_pack(gcp_msg,
                           GCP_TYPE_ACK,
                           GCP_MODULE_01_MARLIN,
                           GCP_MODULE_02_NOZZLE,
                           GCP_CMD_20_MSG_GET,
                           content,
                           5);

        MYSERIAL3.send((uint8_t *)gcp_msg, len);

        uart_nozzle_rx.tx_cnt += 1;
    }

#define SYS_ERR_03_OCI_MOS_INPUT_L     0x03
#define SYS_ERR_04_OCI_MOS_INPUT_H     0x04
#define SYS_ERR_05_HEATER_FAN_ACTIVE   0x05
#define SYS_ERR_06_HEATER_FAN_INACTIVE 0x06
#define SYS_ERR_07_MODEL_FAN_ACTIVE    0x07
#define SYS_ERR_08_MODEL_FAN_INACTIVE  0x08
#define SYS_ERR_10_ADC_RANGE_EXCEED    0x10
#define SYS_ERR_11_TEMP_MAX_LIMIT      0x11
#define SYS_ERR_12_TEMP_MIN_LIMIT      0x12
#define SYS_ERR_13_20S_HEAT_TIMEOUT    0x13
#define SYS_ERR_14_60S_HEAT_TIMEOUT    0x14
#define SYS_ERR_15_TEMP_DROPPED        0x15
#define SYS_ERR_16_TEMP_RUNAWAY        0x16
#define SYS_ERR_17_TEMP_SLIDE_WINDOW   0x17
#define SYS_ERR_18_TEMP_SEGMENT1       0x18
#define SYS_ERR_19_TEMP_SEGMENT2       0x19
#define SYS_ERR_1A_TEMP_SHOCK          0x1A

    void sys_err_parse(uint32_t err)
    {
        if ((err >> SYS_ERR_04_OCI_MOS_INPUT_H) & 0x01)
        {
            MYSERIAL2.printLine("Error:Demage:1 hotend\n");
        }
        if ((err >> SYS_ERR_05_HEATER_FAN_ACTIVE) & 0x01)
        {
            MYSERIAL2.printLine("Error:Heater Fan feedback\n");
        }
        if ((err >> SYS_ERR_06_HEATER_FAN_INACTIVE) & 0x01)
        {
            MYSERIAL2.printLine("Error:Heater Fan feedback\n");
        }
        if ((err >> SYS_ERR_07_MODEL_FAN_ACTIVE) & 0x01)
        {
            MYSERIAL2.printLine("Error:Model Fan feedback\n");
        }
        if ((err >> SYS_ERR_08_MODEL_FAN_INACTIVE) & 0x01)
        {
            MYSERIAL2.printLine("Error:Model Fan feedback\n");
        }
        if ((err >> SYS_ERR_10_ADC_RANGE_EXCEED) & 0x01)
        {
            MYSERIAL2.printLine("heater adc range exceed\n");
        }
        if ((err >> SYS_ERR_11_TEMP_MAX_LIMIT) & 0x01)
        {
            MYSERIAL2.printLine("heater temperature is more than max limit\n");
            thermalManager.max_temp_error((heater_id_t)0);
        }
        if ((err >> SYS_ERR_12_TEMP_MIN_LIMIT) & 0x01)
        {
            MYSERIAL2.printLine("heater temperature is less than max limit\n");
            thermalManager.min_temp_error((heater_id_t)0);
        }
        if ((err >> SYS_ERR_13_20S_HEAT_TIMEOUT) & 0x01)
        {
            MYSERIAL2.printLine("heater temperature 20s timeout\n");
            thermalManager.heater_temp_error((heater_id_t)0);
        }

        if ((err >> SYS_ERR_14_60S_HEAT_TIMEOUT) & 0x01)
        {
            MYSERIAL2.printLine("heater temperature 60s timeout\n");
            thermalManager.heater_temp_error((heater_id_t)0);
        }
        if ((err >> SYS_ERR_15_TEMP_DROPPED) & 0x01)
        {
            MYSERIAL2.printLine("heater temperature dropped 5C\n");
            thermalManager.heater_temp_error((heater_id_t)0);
        }
        if ((err >> SYS_ERR_16_TEMP_RUNAWAY) & 0x01)
        {
            MYSERIAL2.printLine("heater temperature runaway\n");
            thermalManager.heater_temp_runaway((heater_id_t)0);
        }
        if ((err >> SYS_ERR_17_TEMP_SLIDE_WINDOW) & 0x01)
        {
            MYSERIAL2.printLine("heater temperature slide window\n");
            thermalManager.heater_temp_error((heater_id_t)0);
        }
        if ((err >> SYS_ERR_18_TEMP_SEGMENT1) & 0x01)
        {
            MYSERIAL2.printLine("heater temperature segment 1\n");
            thermalManager.heater_temp_error((heater_id_t)0);
        }
        if ((err >> SYS_ERR_19_TEMP_SEGMENT2) & 0x01)
        {
            MYSERIAL2.printLine("heater temperature segment 2\n");
            thermalManager.heater_temp_error((heater_id_t)0);
        }
        if ((err >> SYS_ERR_1A_TEMP_SHOCK) & 0x01)
        {
            MYSERIAL2.printLine("heater temperature shock\n");
            thermalManager.heater_temp_runaway((heater_id_t)0);
        }
    }

    // rx from uart6, nozzle->marlin
    static void uart_nozzle_rx_callback(gcp_msg_t *gcp_msg)
    {
        uint8_t data[64];
        uint8_t len = 0;
        char str[20];

        if (gcp_msg->src_module != GCP_MODULE_02_NOZZLE)
            return;

        if (gcp_msg->dst_module != GCP_MODULE_01_MARLIN)
            return;

        nozzle_rst_cnt = 0;
        uart_nozzle_rx.start_ms = getCurrentMillis();
        memset(data, 0, sizeof(data));
        switch (gcp_msg->cmd)
        {
          case GCP_CMD_20_MSG_GET:
            uint16_t t;

            t = (gcp_msg->content[2] << 8) | gcp_msg->content[1];
            thermalManager.setCurrentHotend(t, 0);
            uart_nozzle_rx.rx_cnt += 1;
            break;

          case GCP_CMD_22_LOG_UPLOAD:
            uart_nozzle_info.sys_err = PACK_LE_32(&gcp_msg->content[0]);
            uart_nozzle_info.rst_flg = PACK_LE_16(&gcp_msg->content[4]);
            uart_nozzle_info.adc_raw = PACK_LE_16(&gcp_msg->content[6]);
            uart_nozzle_info.adc_ave = PACK_LE_16(&gcp_msg->content[8]);
            uart_nozzle_info.temp_now = PACK_LE_16(&gcp_msg->content[10]);
            uart_nozzle_info.temp_tg = PACK_LE_16(&gcp_msg->content[12]);
            uart_nozzle_info.pidout = PACK_LE_16(&gcp_msg->content[14]);
            uart_nozzle_info.probe_thres = PACK_LE_16(&gcp_msg->content[16]);
            uart_nozzle_info.oci_ttl_cnt = PACK_LE_32(&gcp_msg->content[18]);
            uart_nozzle_info.setup = 1;
            break;

          case GCP_CMD_23_NO_MS:
            nozzle.material_sta = gcp_msg->content[0];
            len = sprintf((char *)data, "MS:%d\n", nozzle.material_sta);
            MYSERIAL2.send(data, len);

            MYSERIAL1.printLine("echo:%s\n", data);
            break;

          case GCP_CMD_24_HWSW_VER:
            nozzle.hw_version[0] = gcp_msg->content[0];
            nozzle.hw_version[1] = gcp_msg->content[1];
            nozzle.hw_version[2] = gcp_msg->content[2];
            nozzle.sw_version[0] = gcp_msg->content[3];
            nozzle.sw_version[1] = gcp_msg->content[4];
            nozzle.sw_version[2] = gcp_msg->content[5];

            len = sprintf((char *)data, "ANKER_V8110_HW_V%d.%d.%d_SW_V%d.%d.%d\n",
                          nozzle.hw_version[0], nozzle.hw_version[1], nozzle.hw_version[2],
                          nozzle.sw_version[0], nozzle.sw_version[1], nozzle.sw_version[2]);
            MYSERIAL2.send(data, len);

            MYSERIAL1.printLine("%s", data);
            break;

          case GCP_CMD_28_ENV_T_ALARM:
            OUT_WRITE(EN_24V_HEAT, LOW);
            if (gcp_msg->content[0] == 1)
            {
                if (uart_nozzle_rx.mos_err_flg == 0)
                {
                    uart_nozzle_rx.mos_err_flg = 1;
                    uart_nozzle_rx.mos_err_start_ms = getCurrentMillis();
                }
            }
            MYSERIAL2.printLine("nozzle power off, type = %d\n", gcp_msg->content[0]);
            break;

          case GCP_CMD_29_SN_WRITE:
            memcpy(str, "ok\n", 4);
            MYSERIAL2.send((uint8_t *)str, 4);

            MYSERIAL1.printLine("SN write ack\n");
            break;

          case GCP_CMD_30_SN_READ:
            memcpy(str, "SN:", 4);
            strncat(str, (char *)&gcp_msg->content[0], 16);
            str[19] = '\n';
            MYSERIAL2.send((uint8_t *)str, sizeof(str));

            MYSERIAL1.printLine("SN read ack, %s\n", str);
            break;

          case GCP_CMD_34_LEVELING_VAL_READ:
            anker_probe_set.leveing_value = PACK_LE_16(&gcp_msg->content[0]);
            MYSERIAL2.printLine("echo:M3020 V%d\r\n", anker_probe_set.leveing_value);
            break;

          case GCP_CMD_45_M3001_DEAL:
            cs1237_test.init_value = PACK_LE_32(&gcp_msg->content[0]);
            SERIAL_ECHOPAIR("echo:M3001 V", cs1237_test.init_value, "\r\n");
            break;

          case GCP_CMD_46_M3002_DEAL:
            cs1237_test.cur_value = PACK_LE_32(&gcp_msg->content[0]);
            SERIAL_ECHOPAIR("echo:M3002 V", cs1237_test.cur_value, "\r\n");
            break;

          case GCP_CMD_47_DEBUG_LOG:
            MYSERIAL2.printLine("echo: nozzle debug log = %d\n", gcp_msg->content[0]);
            break;

          case GCP_CMD_48_POINT_TYPE: // Notify the nozzle board of the position under different commands.
            MYSERIAL2.printLine("echo: nozzle tpye= %d, point= %d\n", gcp_msg->content[1], gcp_msg->content[0]);
            break;

          case GCP_CMD_49_PRODUCTION_MODE: // Switch between production test mode and normal mode.
            MYSERIAL2.printLine("echo: production mode= %d, parm= %d\n", gcp_msg->content[1], gcp_msg->content[0]);
            break;

          case GCP_CMD_4A_OVERPRESSURE: // Detecting and checking parameters related to overpressure 
            {
                uint8_t overpressure = gcp_msg->content[0];
                int32_t initial      = PACK_LE_32(&gcp_msg->content[1]);
                int32_t Raw          = PACK_LE_32(&gcp_msg->content[5]);
                int32_t filter       = PACK_LE_32(&gcp_msg->content[9]);
                uint8_t err_count    = gcp_msg->content[13];
                uint8_t HPF_count    = gcp_msg->content[14];
                uint8_t area0_count    = gcp_msg->content[15];
                uint8_t area1_count    = gcp_msg->content[16];
                MYSERIAL2.printLine("echo: %s-point= %d, type= %d, adc= %d %d %d, count= %d %d %d %d\n", POINT_TYPE_STRING, POINT_TYPE_POSITION, overpressure, initial, Raw, filter, err_count, HPF_count, area0_count, area1_count);
                #if ENABLED(ANKER_OVERPRESSURE_REPORT)
                    if(ANKER_TEST_MODE() && (overpressure == TRIGGER_OVERPRESSURE))
                    {
                        production_mode.overpressure_trigger = OVERPRESSURE_TRIGGER_OPEN;
                        if (endstops.z_probe_enabled)
                        {
                            planner.endstop_triggered(_AXIS(Z));
                            anker_homing.trigger_ms = anker_homing.trigger_per_ms + ANTHER_TIME_ANTHOR_Z_MAX_LIMIT+100;
                        }
                    }
                    else
                    {
                        production_mode.overpressure_trigger = OVERPRESSURE_TRIGGER_CLOSED;
                    }
                #endif
            }
            break;

          default:
            break;
        }
    }

    static void uart_nozzle_rx_disconnect_check(void)
    {
        uint32_t t_now = getCurrentMillis();
        int32_t gap = t_now - uart_nozzle_rx.start_ms;

        if (fatal_err)
            return;

        if (gap > 5000)
        {
            nozzle_rst_cnt += 1;
            if (nozzle_rst_cnt == 3)
            {
                thermalManager.setCurrentHotend(0, 0);
                MYSERIAL2.printLine("Err:disconnect with nozzle, nozzle power off, %d,%d\n", t_now, uart_nozzle_rx.start_ms);
            }
            if (nozzle_rst_cnt > 2)
            {
                OUT_WRITE(EN_24V_HEAT, LOW);
            }
            else
            {
                MYSERIAL2.printLine("Err:disconnect with nozzle, nozzle reset, %d,%d\n", t_now, uart_nozzle_rx.start_ms);
                OUT_WRITE(EN_24V_HEAT, LOW);
                _delay_ms(100);
                OUT_WRITE(EN_24V_HEAT, HIGH);
            }
            uart_nozzle_rx.start_ms = t_now;
        }
    }

    static void uart_nozzle_packet_parse_isr(uint8_t *buf, uint16_t len)
    {
        uint16_t index;

        for (index = 0; index < len; index++)
        {
            gcp_parser_fsm_process(&uart_nozzle_rx.parser, buf[index]);
        }
    }

    void latch_clear(void)
    {
        OUT_WRITE(PD10, LOW);
        _delay_ms(1);
        OUT_WRITE(PD10, HIGH);
    }

    void uart_nozzle_init(void)
    {
        gcp_parser_init(&uart_nozzle_rx.parser,
                        uart_nozzle_rx.rx_buffer,
                        10,
                        getCurrentMillis,
                        uart_nozzle_rx_callback);

        soft_timer_init(&uart_nozzle_rx.soft_timer,
                        100,
                        SOFT_TIMER_REPEAT_FOREVER,
                        uart_nozzle_tx_temperature_polling_callback);

        soft_timer_start(&uart_nozzle_rx.soft_timer);

        uart_rx_isr_callback[5] = uart_nozzle_packet_parse_isr;

        uart_nozzle_rx.start_ms = getCurrentMillis();

        MYSERIAL2.printLine("marlin reset\n");
    }

    static void uart_nozzle_info_echo(void)
    {
        if (uart_nozzle_info.setup == 1)
        {   
            static uint16_t last_temp_now = uart_nozzle_info.temp_now;

            uart_nozzle_info.setup = 0;
            if(!WITHIN(uart_nozzle_info.temp_now, last_temp_now - 5, last_temp_now + 5))
            {// Upload data once the temperature fluctuates more than 5℃.
                if(uart_nozzle_info.temp_now < 5)
                    last_temp_now = 5;// To prevent situations where no alarm is reported for a long time, upload data once the temperature remains below 5℃ for 5 seconds.
                else
                    last_temp_now = uart_nozzle_info.temp_now;
                MYSERIAL2.printLine("sys_err = 0x%x, rst_flg = 0x%x, adc_raw = %d, adc_ave = %d, ", uart_nozzle_info.sys_err, uart_nozzle_info.rst_flg, uart_nozzle_info.adc_raw, uart_nozzle_info.adc_ave);
                MYSERIAL2.printLine("temp_now = %d, temp_tg = %d, pidout = %d, probe_thres = %d, ", uart_nozzle_info.temp_now, uart_nozzle_info.temp_tg, uart_nozzle_info.pidout, uart_nozzle_info.probe_thres);
                MYSERIAL2.printLine("tx_cnt = %d, rx_cnt = %d, oci ttl cnt = %d\n", uart_nozzle_rx.tx_cnt, uart_nozzle_rx.rx_cnt, uart_nozzle_info.oci_ttl_cnt);
            }

            sys_err_parse(uart_nozzle_info.sys_err);
        }
    }

    void hotend_mos_err_polling(void)
    {
        if (uart_nozzle_rx.mos_err_flg == 0)
            return;

        if (int(getCurrentMillis() - uart_nozzle_rx.mos_err_start_ms) > 5500)
        {
            MYSERIAL2.printLine("Error:Demage:1 hotend\n");
            uart_nozzle_rx.mos_err_flg = 0;
        }
    }

    void uart_nozzle_polling(void)
    {
        soft_timer_loop();

        uart_nozzle_rx_disconnect_check();

        hotend_mos_err_polling();

        uart_nozzle_info_echo();
    }
}

#endif
