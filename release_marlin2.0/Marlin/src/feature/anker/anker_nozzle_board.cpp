/*
 * @Author       : winter
 * @Date         : 2022-05-12 20:30:21
 * @LastEditors  : winter
 * @LastEditTime : 2022-07-08 17:21:39
 * @Description  :
 */
#include "anker_nozzle_board.h"
#include "../../gcode/gcode.h"
#include "../../module/planner.h"

#if ENABLED(ANKER_MAKE_API)

#if ENABLED(ANKER_NOZZLE_BOARD)
   
   
#define FACTOR (0x107 & 0xFF)
static unsigned char crc8(unsigned char *pdat, unsigned int len)
{
    unsigned char j;
    unsigned char crc = 0x00;
    while (len--)
    {
        crc ^= (*pdat++);
        for (j = 8; j > 0; j--)
        {
            crc <<= 1;
            if (crc & 0x80)
            {
                crc ^= FACTOR;
            }
        }
    }

    return crc;
}

static void anker_nozzle_board_power_reset(void)
{
#ifdef NOZZLE_BOARD_PWR_PIN
    OUT_WRITE(NOZZLE_BOARD_PWR_PIN, !NOZZLE_BOARD_PWR_STATE);
    _delay_ms(10);
    OUT_WRITE(NOZZLE_BOARD_PWR_PIN, NOZZLE_BOARD_PWR_STATE);
    _delay_ms(10);
#endif
}

static void anker_nozzle_board_serial_begin(void)
{
    anker_nozzle_board_info_t *p_info = get_anker_nozzle_board_info();

#ifdef SERIAL_PORT_3
#ifndef BAUDRATE_3
#define BAUDRATE_3 BAUDRATE
#endif
    if (p_info->serial_flag == ANKER_NOZZLE_BOARD_SERIAL_TXRX_CLOSE)
    {
        MYSERIAL3.begin(BAUDRATE_3);
        millis_t serial_connect_timeout = millis() + 1000UL;
        while (!MYSERIAL3.connected() && PENDING(millis(), serial_connect_timeout))
        {
        }
        p_info->serial_flag = ANKER_NOZZLE_BOARD_SERIAL_TXRX_OPEN;
    }
#endif
}

static void anker_nozzle_board_serial_end(void)
{
    anker_nozzle_board_info_t *p_info = get_anker_nozzle_board_info();

#ifdef SERIAL_PORT_3
    if (p_info->serial_flag == ANKER_NOZZLE_BOARD_SERIAL_TXRX_OPEN)
    {
        MYSERIAL3.end();
        p_info->serial_flag = ANKER_NOZZLE_BOARD_SERIAL_TXRX_CLOSE;
    }
#endif

#if ENABLED(PROVE_CONTROL)
    OUT_WRITE(PROVE_CONTROL_PIN, !PROVE_CONTROL_STATE);
#endif
}

static int anker_nozzle_board_serial_read(void)
{
    char *p;
    anker_nozzle_board_info_t *p_info = get_anker_nozzle_board_info();

#ifdef SERIAL_PORT_3
    millis_t serial_read_timeout = millis() + 1000UL;
    while (PENDING(millis(), serial_read_timeout))
    {
        if (MYSERIAL3.available() > 0)
        {
            const int c = MYSERIAL3.read();
            const char serial_char = (char)c;

            if (ISEOL(serial_char))
            {
                if (p_info->serial_rx_count > 0)
                {
                    // MYSERIAL1.printf(" --- serial_read : %s ---\r\n", p_info->serial_rx_buf);

                    if ((p = strstr(p_info->serial_rx_buf, "M3001")) != NULL)
                    {
                        sscanf(p, "M3001 %d", &(p_info->init_value));
                        return 0;
                    }
                    else if ((p = strstr(p_info->serial_rx_buf, "M3002")) != NULL)
                    {
                        sscanf(p, "M3002 %d", &(p_info->cur_value));
                        return 0;
                    }
                    else if ((p = strstr(p_info->serial_rx_buf, "M3003 OK")) != NULL)
                    {
                        return 0;
                    }
                    else if ((p = strstr(p_info->serial_rx_buf, "M3008 OK")) != NULL)
                    {
                        return 0;
                    }
                    else if ((p = strstr(p_info->serial_rx_buf, "M3009 OK")) != NULL)
                    {
                        return 0;
                    }
                    else if ((p = strstr(p_info->serial_rx_buf, "M3010 OK")) != NULL)
                    {
                        return 0;
                    }
                    else if ((p = strstr(p_info->serial_rx_buf, "M3007")) != NULL)
                    {
                        memset(p_info->fireproof_rx_buf,0,sizeof(p_info->fireproof_rx_buf));
                        strcpy(p_info->fireproof_rx_buf,p_info->serial_rx_buf);
                        return 0;
                    }
                }
                p_info->serial_rx_count = 0;
                memset(p_info->serial_rx_buf, 0, sizeof(p_info->serial_rx_buf));
            }
            else
            {
                p_info->serial_rx_buf[p_info->serial_rx_count++] = serial_char;
            }
        }
    }
#endif

    return -1;
}

static int anker_nozzle_board_serial_write(char *cmd_buf)
{
    uint8_t count = 0;
    char tmp_crc_buf[8] = {0};
    unsigned char tmp_crc8 = 0;
    anker_nozzle_board_info_t *p_info = get_anker_nozzle_board_info();

#ifdef SERIAL_PORT_3
    while (count++ < 3)
    {
        watchdog_refresh();

        p_info->serial_rx_count = 0;
        memset(p_info->serial_rx_buf, 0, sizeof(p_info->serial_rx_buf));

        tmp_crc8 = crc8((unsigned char *)cmd_buf, strlen(cmd_buf));
        sprintf(tmp_crc_buf, "*%d", tmp_crc8);
        MYSERIAL3.printf("%s%s\n", cmd_buf, tmp_crc_buf);
        // MYSERIAL1.printf(" --- serial_write : %s%s ---\r\n", cmd_buf, tmp_crc_buf);

        if (p_info->serial_read() == 0)
        {
            return 0;
        }
    }
#endif
    // MYSERIAL1.printf("--- %s failed ---\r\n", cmd_buf);
    return -1;
}

// sprintf(tmp_buf, "%s ", "M3004");
// sprintf(tmp_buf, "%s %d ", "M3005", nums);
// sprintf(tmp_buf, "%s %d %d %d ", "M3006", num, adc, pwm);

void anker_nozzle_board_init(void)
{
    anker_nozzle_board_info_t *p_info = get_anker_nozzle_board_info();

    p_info->power_reset = anker_nozzle_board_power_reset;
    p_info->serial_begin = anker_nozzle_board_serial_begin;
    p_info->serial_end = anker_nozzle_board_serial_end;
    p_info->serial_read = anker_nozzle_board_serial_read;
    p_info->serial_write = anker_nozzle_board_serial_write;

    p_info->power_reset();
    p_info->serial_begin();

    memset(p_info->serial_tx_buf, 0, sizeof(p_info->serial_tx_buf));
    sprintf(p_info->serial_tx_buf, "M3003 %d ", p_info->threshold);

    if (p_info->serial_write(p_info->serial_tx_buf) == 0)
    {      
        memset(p_info->serial_tx_buf, 0, sizeof(p_info->serial_tx_buf));
        sprintf(p_info->serial_tx_buf, "M3008 %d ", p_info->fireproof_adc0);    
    }
    if (p_info->serial_write(p_info->serial_tx_buf) == 0)
    {      
        memset(p_info->serial_tx_buf, 0, sizeof(p_info->serial_tx_buf));
        sprintf(p_info->serial_tx_buf, "M3009 %d ", p_info->fireproof_adc1);    
    }
    if (p_info->serial_write(p_info->serial_tx_buf) == 0)
    {
        memset(p_info->serial_tx_buf, 0, sizeof(p_info->serial_tx_buf));
        sprintf(p_info->serial_tx_buf, "M3010 ");
        p_info->serial_write(p_info->serial_tx_buf);
    }

    p_info->serial_end();
}

anker_nozzle_board_info_t *get_anker_nozzle_board_info(void)
{
    static anker_nozzle_board_info_t anker_nozzle_board_info = {0};
    return &anker_nozzle_board_info;
}

void set_anker_z_sensorless_probe_value(int value)
{
  char str[100]="";
   get_anker_nozzle_board_info()->is_z_sensorless=true;
   get_anker_nozzle_board_info()->z_sensorless_threshols = get_anker_nozzle_board_info()->threshold ;
   get_anker_nozzle_board_info()->threshold=value;
   sprintf(str,"M3003 V%d\n",get_anker_nozzle_board_info()->threshold);
   gcode.process_subcommands_now_P(str);
}

void reset_anker_z_sensorless_probe_value()
{
   char str[100]="";
   get_anker_nozzle_board_info()->is_z_sensorless=false;
   get_anker_nozzle_board_info()->threshold=get_anker_nozzle_board_info()->z_sensorless_threshols;
   sprintf(str,"M3003 V%d\n",get_anker_nozzle_board_info()->threshold);
   gcode.process_subcommands_now_P(str);
}

#endif /* ANKER_NOZZLE_BOARD_THRESHOLD */

#endif /* ANKER_MAKE_API */
