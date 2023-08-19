/*
 * @Author       : winter
 * @Date         : 2022-05-12 20:32:54
 * @LastEditors  : winter
 * @LastEditTime : 2022-06-20 20:37:13
 * @Description  :
 */
#include "../../inc/MarlinConfig.h"

#if ENABLED(ANKER_MAKE_API)

#if ENABLED(ANKER_NOZZLE_BOARD)

#ifndef __ANKER_NOZZLE_BOARD_H__
#define __ANKER_NOZZLE_BOARD_H__
  #include "../../gcode/gcode.h"
#define ANKER_NOZZLE_BOARD_INIT_VALUE_CMD_STR "M3001"
#define ANKER_NOZZLE_BOARD_CUR_VALUE_CMD_STR "M3002"
#define ANKER_NOZZLE_BOARD_THRESHOLD_CMD_STR "M3003"

typedef enum
{
    ANKER_NOZZLE_BOARD_SERIAL_TXRX_CLOSE = 0,
    ANKER_NOZZLE_BOARD_SERIAL_TXRX_OPEN = 1,
} anker_nozzle_board_serial_state;

typedef struct
{
    char serial_tx_buf[64];
    int threshold;
    int z_sensorless_threshols;
    bool is_z_sensorless=false;
    char serial_rx_buf[64];
    int serial_rx_count;
    int serial_rx_len;
    int serial_flag;
    int init_value;
    int cur_value;
    int fireproof_adc0;
    int fireproof_adc1;
    char fireproof_rx_buf[64];

    void (*power_reset)(void);
    void (*serial_begin)(void);
    void (*serial_end)(void);
    int (*serial_read)(void);
    int (*serial_write)(char *cmd);

} anker_nozzle_board_info_t;

void anker_nozzle_board_init(void);
void set_anker_z_sensorless_probe_value(int value);
void reset_anker_z_sensorless_probe_value();

anker_nozzle_board_info_t *get_anker_nozzle_board_info(void);

#endif /* __ANKER_NOZZLE_BOARD_H__ */

#endif /* ANKER_NOZZLE_BOARD */

#endif /* ANKER_MAKE_API */
