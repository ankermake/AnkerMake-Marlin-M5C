/*
 * @Author       : winter
 * @Date         : 2022-05-24 14:08:20
 * @LastEditors  : winter
 * @LastEditTime : 2022-06-21 12:24:01
 * @Description  :
 */
#include "../../inc/MarlinConfig.h"
#include "../gcode.h"
#include "../../module/settings.h"

#if ENABLED(ANKER_NOZZLE_BOARD)
#include "../../feature/anker/anker_nozzle_board.h"

void GcodeSuite::M3001()
{
  memset(get_anker_nozzle_board_info()->serial_tx_buf, 0, sizeof(get_anker_nozzle_board_info()->serial_tx_buf));
  sprintf(get_anker_nozzle_board_info()->serial_tx_buf, "M3001 ");
  if (get_anker_nozzle_board_info()->serial_write(get_anker_nozzle_board_info()->serial_tx_buf) == 0)
  {
    SERIAL_ECHOPAIR("echo:M3001 V", get_anker_nozzle_board_info()->init_value, "\r\n");
  }
}

void GcodeSuite::M3002()
{
  memset(get_anker_nozzle_board_info()->serial_tx_buf, 0, sizeof(get_anker_nozzle_board_info()->serial_tx_buf));
  sprintf(get_anker_nozzle_board_info()->serial_tx_buf, "M3002 ");
  if (get_anker_nozzle_board_info()->serial_write(get_anker_nozzle_board_info()->serial_tx_buf) == 0)
  {
    SERIAL_ECHOPAIR("echo:M3002 V", get_anker_nozzle_board_info()->cur_value, "\r\n");
  }
}

// M3003 get nozzle board threshold
// M3003 V200 set nozzle board threshold
void GcodeSuite::M3003()
{
  int tmp_value = 0;
  bool is_set_threshold=false;
  bool is_set_fireproof0=false;
  bool is_set_fireproof1=false;

  if (parser.seen('V'))
  {
    get_anker_nozzle_board_info()->threshold = parser.value_int();
    is_set_threshold=true;
  }
  if (parser.seen('A'))
  {
    tmp_value=parser.value_int();
    get_anker_nozzle_board_info()->fireproof_adc0 = tmp_value;
    is_set_fireproof0=true;
  }
  if (parser.seen('B'))
  {
    tmp_value=parser.value_int();
    get_anker_nozzle_board_info()->fireproof_adc1 = tmp_value;
    is_set_fireproof1=true;
  }
  
  if((!is_set_threshold)&&(!is_set_fireproof0)&&(!is_set_fireproof1))
  {
    SERIAL_ECHOPAIR("echo:M3003 V ", get_anker_nozzle_board_info()->threshold, "\r\n");
    SERIAL_ECHOPAIR("echo:M3003 A0 ", get_anker_nozzle_board_info()->fireproof_adc0, "\r\n");
    SERIAL_ECHOPAIR("echo:M3003 A1 ", get_anker_nozzle_board_info()->fireproof_adc1, "\r\n");
  }

  if(is_set_threshold)
  {
    is_set_threshold=false;
    settings.save();
    tmp_value = get_anker_nozzle_board_info()->threshold;
    SERIAL_ECHOPAIR("echo:M3003 V", tmp_value, "\r\n");

    get_anker_nozzle_board_info()->power_reset();
    get_anker_nozzle_board_info()->serial_begin();

    memset(get_anker_nozzle_board_info()->serial_tx_buf, 0, sizeof(get_anker_nozzle_board_info()->serial_tx_buf));
    sprintf(get_anker_nozzle_board_info()->serial_tx_buf, "M3003 %d ", tmp_value);
    get_anker_nozzle_board_info()->serial_write(get_anker_nozzle_board_info()->serial_tx_buf);

   if(is_set_fireproof0||is_set_fireproof1)
   {

    memset(get_anker_nozzle_board_info()->serial_tx_buf, 0, sizeof(get_anker_nozzle_board_info()->serial_tx_buf));
    sprintf(get_anker_nozzle_board_info()->serial_tx_buf, "M3007");
    get_anker_nozzle_board_info()->serial_write(get_anker_nozzle_board_info()->serial_tx_buf);
    SERIAL_ECHO("M3007 pre:");
    SERIAL_ECHO(get_anker_nozzle_board_info()->fireproof_rx_buf);
    
    if(is_set_fireproof0)
    {
      is_set_fireproof0=false;
      tmp_value = get_anker_nozzle_board_info()->fireproof_adc0;
      SERIAL_ECHOPAIR("echo:M3008 ", tmp_value, "\r\n");
      memset(get_anker_nozzle_board_info()->serial_tx_buf, 0, sizeof(get_anker_nozzle_board_info()->serial_tx_buf));
      sprintf(get_anker_nozzle_board_info()->serial_tx_buf, "M3008 %d ", tmp_value);
      get_anker_nozzle_board_info()->serial_write(get_anker_nozzle_board_info()->serial_tx_buf);
    }

    if(is_set_fireproof1)
    {
      is_set_fireproof1=false;
      tmp_value = get_anker_nozzle_board_info()->fireproof_adc1;
      SERIAL_ECHOPAIR("echo:M3009 ", tmp_value, "\r\n");
      memset(get_anker_nozzle_board_info()->serial_tx_buf, 0, sizeof(get_anker_nozzle_board_info()->serial_tx_buf));
      sprintf(get_anker_nozzle_board_info()->serial_tx_buf, "M3009 %d ", tmp_value);
      get_anker_nozzle_board_info()->serial_write(get_anker_nozzle_board_info()->serial_tx_buf);
    }

    memset(get_anker_nozzle_board_info()->serial_tx_buf, 0, sizeof(get_anker_nozzle_board_info()->serial_tx_buf));
    sprintf(get_anker_nozzle_board_info()->serial_tx_buf, "M3007");
    get_anker_nozzle_board_info()->serial_write(get_anker_nozzle_board_info()->serial_tx_buf);
    SERIAL_ECHO("M3007 aft:");
    SERIAL_ECHO(get_anker_nozzle_board_info()->fireproof_rx_buf);
   }
    if (tmp_value != 0)
    {
      memset(get_anker_nozzle_board_info()->serial_tx_buf, 0, sizeof(get_anker_nozzle_board_info()->serial_tx_buf));
      sprintf(get_anker_nozzle_board_info()->serial_tx_buf, "M3010 ");
      get_anker_nozzle_board_info()->serial_write(get_anker_nozzle_board_info()->serial_tx_buf);

      get_anker_nozzle_board_info()->serial_end();
    }
  }

}

#endif
