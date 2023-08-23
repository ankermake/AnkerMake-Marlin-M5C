/*
 * @Author         : winter
 * @Date           : 2022-08-23 11:42:20
 * @LastEditors    : winter
 * @LastEditTime   : 2022-08-25 14:43:59
 * @Description    :
 */
#include "../../inc/MarlinConfig.h"
#include "../../HAL/STM32/fastio.h"
#include "../../HAL/shared/Delay.h"

#if ENABLED(ANKER_MAKE_API)


#if ENABLED(ANKER_Z_OFFSET_FUNC)
#ifndef __ANKER_Z_OFFSET_H__
#define __ANKER_Z_OFFSET_H__

typedef enum
{
	CS1237_INVALID_SIGNAL = 0,
	CS1237_VALID_SIGNAL,
} CS1237_SIGNAL_STATUS;

class Anker_Zoffset
{
public:
    static xy_pos_t xy[2];
	bool cs1237_start_convert=false;
	static float  save_probe_offset_z;
	float value[5];
	bool log=false;
	bool loop_read_flag=false;
	void init(void);
	void deal(void);
	void loop_read(void);
	void cs1237_enable(void);
	void cs1237_disable(void);
	int32_t cs1237_threshold_get(void);
	void cs1237_threshold_set(int32_t threshold);
	void run();
	void update_offset(void);
	void reset_init(void);
public:
	CS1237_SIGNAL_STATUS cs1237_signal_status;
};

extern void cs1237_init_value();

extern Anker_Zoffset anker_z_offset;

#endif /* __ANKER_Z_OFFSET_H__ */

#endif /* ANKER_Z_OFFSET_FUNC */

#if ENABLED(ANKER_PROBE_SET)

	#define HOMING_PROBE_VALUE   650
	#define LEVEING_PROBE_VALUE  600//400
	#define LEVEING_PROBE_DELAY  650
	#define PROBE_READ_NUM_DELAY 500

    class Anker_Probe_set
	{
	public:
	   static uint16_t homing_value;
	   static uint16_t leveing_value;
	   static uint16_t delay;
	   static uint8_t run_step;
	   static bool auto_run_flag;
       static xy_pos_t xy[5];
	   static bool point_test_flag;
       void probe_start(uint16_t value);
	   void reset_value();
	   void report_value();
	   void get_probe_value();
	   void show_adc_value(bool show_adc);
	   void pid_autotune(uint16_t temp, uint16_t ncyclesc);
	   void run();
	   void home_delay();
	   void auto_run_send_start_info();
	   void auto_run();
	   void point_test_ready();
	   void point_test(uint8_t point);
	   void point_test_idle();
	};
extern Anker_Probe_set anker_probe_set;
#endif/* ANKER_PROBE_SET */


#endif /* ANKER_MAKE_API */
