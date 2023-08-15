/*
 * @Author         : winter
 * @Date           : 2022-08-23 11:42:50
 * @LastEditors    : winter
 * @LastEditTime   : 2022-08-25 15:10:45
 * @Description    :
 */
// #include "sys.h"
#include "anker_z_offset.h"

#if ENABLED(ANKER_MAKE_API)

#if ENABLED(ANKER_Z_OFFSET_FUNC) 

#include "../../module/stepper.h"
#include "../../module/probe.h"
#include "../../feature/bedlevel/bedlevel.h"
#include "../../module/probe.h"
#include "../../module/temperature.h"
#include "../../gcode/gcode.h"

#define CS1237_SDA_INPUT SET_INPUT(CS1237_SDA_PIN)
#define CS1237_SDA_READ READ(CS1237_SDA_PIN)
#define CS1237_SCL_H OUT_WRITE(CS1237_SCL_PIN, HIGH)
#define CS1237_SCL_L OUT_WRITE(CS1237_SCL_PIN, LOW)
#define CS1237_SDA_H OUT_WRITE(CS1237_SDA_PIN, HIGH)
#define CS1237_SDA_L OUT_WRITE(CS1237_SDA_PIN, LOW)
#define CS1237_PWR_CTRL_ENABLE OUT_WRITE(CS1237_PWR_CTRL_PIN, LOW)
#define CS1237_PWR_CTRL_DISABLE OUT_WRITE(CS1237_PWR_CTRL_PIN, HIGH)

#define ValBit(VAR, POS) (VAR & (1 << POS))
#define SetBit(VAR, POS) (VAR |= (1 << POS))
#define ClrBit(VAR, POS) (VAR &= ((1 << POS) ^ 255))

typedef enum
{
    CS1237_REFO_ON = 0,
    CS1237_REFO_OFF = 1,
} CS1237_REFO;

typedef enum
{
    CS1237_SPEED_10HZ = 0,
    CS1237_SPEED_40HZ = 1,
    CS1237_SPEED_640HZ = 2,
    CS1237_SPEED_1280HZ = 3
} CS1237_SPEED;

typedef enum
{
    C1237_PGA_1 = 0,
    C1237_PGA_2 = 1,
    C1237_PGA_64 = 2,
    C1237_PGA_128 = 3
} CS1237_PGA;

typedef enum
{
    C1237_CHN_CHA = 0, // adc channel
    C1237_CHN_RST = 1, // cheap reserved
    C1237_CHN_TEM = 2, // temperature test
    C1237_CHN_INS = 3  // internal short
} CS1237_CHN;

typedef enum
{
    CS1237_READ_CONFIG_CMD = 0x56,
    CS1237_WRITE_CONFIG_CMD = 0x65,
    CS1237_CONFIG_RESET_VALUE = 0x0C,
    CS1237_TIMEOUT = 550, // ms
    CS1237_DATA_BUF_SIZE = 3,
    CS1237_DATA_BUF_HEAD = 1,
    CS1237_DATA_BUF_TAIL = 1,
    GET_INIT_VALUE_TIMES = 2,
    CS1237_DEFAULT_THRESHOLD = 100,

} CS1237_INFO_ENUM;

typedef struct
{
    uint8_t config;
    int32_t init_value;
    int32_t cur_value;
    int32_t average_value;
    uint8_t average_flag;
    uint8_t init_status;
    int32_t threshold;
    uint8_t deal_flag;
    uint8_t average_deal_step;

} cs1237_info_t;

cs1237_info_t cs1237_info = {0};
Anker_Zoffset anker_z_offset;
xy_pos_t Anker_Zoffset::xy[2];
float Anker_Zoffset::save_probe_offset_z=0;


static void cs1237_delay_1us(void)
{
    __NOP();
}

static void cs1237_delay_1ms(void)
{
    uint16_t t = 30000;
    while (t--)
    {
        __NOP();
    }
}

static void cs1237_delay_ms(__IO uint16_t ms)
{
    //_delay_ms(ms);
  //  safe_delay(ms);
    while (ms--)
    {
        cs1237_delay_1ms();
    }
}

static void cs1237_gpio_init(void)
{
    CS1237_PWR_CTRL_ENABLE;
    CS1237_SCL_H;
    CS1237_SDA_H;
}

static uint8_t cs1237_read_sda_pin(void)
{
    if (CS1237_SDA_READ > 0)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

static void cs1237_one_clock(void)
{
    CS1237_SCL_H;
    cs1237_delay_1us();
    CS1237_SCL_L;
    cs1237_delay_1us();
}

static void cs1237_write_config(uint8_t data)
{
    uint8_t i = 0;
    uint8_t write_cmd = 0;
    uint16_t count = 0;

    CS1237_SDA_H;
    CS1237_SDA_INPUT;
    CS1237_SCL_L;
    while (cs1237_read_sda_pin() == 1)
    {
        cs1237_delay_ms(1);
        count++;
        if (count > CS1237_TIMEOUT)
        {
            CS1237_SDA_H;
            CS1237_SCL_H;
            return;
        }
    }
    for (i = 0; i < 29; i++) // 1 - 29
    {
        cs1237_one_clock();
    }
    // CS1237_SDA_H;
    write_cmd = CS1237_WRITE_CONFIG_CMD;
    for (i = 0; i < 7; i++) // 30 - 36
    {
        if (write_cmd & 0x40)
        {
            CS1237_SDA_H;
        }
        else
        {
            CS1237_SDA_L;
        }
        cs1237_one_clock();
        write_cmd <<= 1;
    }
    // CS1237_SDA_H;
    cs1237_one_clock();     // 37
    for (i = 0; i < 8; i++) // 38 - 45
    {
        if (data & 0x80)
            CS1237_SDA_H; // OUT = 1
        else
            CS1237_SDA_L;
        data <<= 1;
        cs1237_one_clock();
    }
    CS1237_SDA_H;       // OUT = 1
    cs1237_one_clock(); // 46
}

static uint8_t cs1237_read_config(void)
{
    uint8_t i = 0;
    uint8_t read_cmd = 0, data = 0;
    uint16_t count = 0;

    CS1237_SDA_H;
    CS1237_SDA_INPUT;
    CS1237_SCL_L;
    while (cs1237_read_sda_pin() == 1)
    {
        cs1237_delay_ms(1);
        count++;
        if (count > CS1237_TIMEOUT)
        {
            CS1237_SDA_H;
            CS1237_SCL_H;
            return 0;
        }
    }
    for (i = 0; i < 29; i++) // 1 - 29
    {
        cs1237_one_clock();
    }
    CS1237_SDA_H;
    read_cmd = CS1237_READ_CONFIG_CMD;
    for (i = 0; i < 7; i++) // 30 - 36
    {
        if (read_cmd & 0x40)
        {
            CS1237_SDA_H;
        }
        else
        {
            CS1237_SDA_L;
        }
        cs1237_one_clock();
        read_cmd <<= 1;
    }
    CS1237_SDA_INPUT;
    cs1237_one_clock();     // 37
    for (i = 0; i < 8; i++) // 38 - 45
    {
        data <<= 1;
        cs1237_one_clock();
        if (cs1237_read_sda_pin() == 1)
        {
            data++;
        }
    }
    CS1237_SDA_H;       // OUT = 1
    cs1237_one_clock(); // 46

    return data;
}

static int32_t cs1237_read_result(void)
{
    uint8_t i = 0;
    uint32_t data = 0;
    int32_t ret_data = 0;
    uint16_t count = 0;

    CS1237_SDA_H;
    CS1237_SDA_INPUT;
    CS1237_SCL_L;
    while (cs1237_read_sda_pin() == 1)
    {
        cs1237_delay_ms(1);
        count++;
        if (count > CS1237_TIMEOUT)
        {
            CS1237_SCL_H;
            CS1237_SDA_H;
            return 0;
        }
    }
    data = 0;
    for (i = 0; i < 24; i++)
    {
        data <<= 1;
        cs1237_one_clock();
        if (cs1237_read_sda_pin() == 1)
            data++;
    }
    cs1237_one_clock();
    cs1237_one_clock();
    cs1237_one_clock();
    CS1237_SDA_H;

    if (data & 0x800000)
    {
        data -= 1;
        data = ~data;
        data &= 0x7FFFFF;
        ret_data = data;
        ret_data *= -1;
    }
    else
    {
        ret_data = data;
    }

    ret_data /= 32;

    return ret_data;
}

static void cs1237_get_average(void)
{
    static int32_t tmp_buf[CS1237_DATA_BUF_SIZE] = {0};
    static uint8_t tmp_count = 0;
    uint8_t i = 0, j = 0;
    int32_t tmp_value = 0;

    switch (cs1237_info.average_deal_step)
    {
    case 0:
    {
        tmp_count = 0;
        cs1237_info.average_flag = 0;
        memset(tmp_buf, 0, sizeof(tmp_buf));
        cs1237_info.average_deal_step++;
        break;
    }
    case 1:
    {
        tmp_buf[tmp_count++] = (cs1237_read_result());
        //cs1237_delay_ms(5);
        if (tmp_count >= CS1237_DATA_BUF_SIZE)
        {
            cs1237_info.average_deal_step++;
        }
        break;
    }
    case 2:
    {
        for (i = 0; i < CS1237_DATA_BUF_SIZE - 1; i++)
        {
            for (j = i + 1; j < CS1237_DATA_BUF_SIZE; j++)
            {
                if (tmp_buf[i] > tmp_buf[j])
                {
                    tmp_value = tmp_buf[i];
                    tmp_buf[i] = tmp_buf[j];
                    tmp_buf[j] = tmp_value;
                }
            }
        }

        tmp_value = 0;
        for (i = CS1237_DATA_BUF_HEAD; i < CS1237_DATA_BUF_SIZE - CS1237_DATA_BUF_TAIL; i++)
        {
            tmp_value += tmp_buf[i];
        }

        cs1237_info.average_value = tmp_value / (CS1237_DATA_BUF_SIZE - CS1237_DATA_BUF_HEAD - CS1237_DATA_BUF_TAIL);

        cs1237_info.average_flag = 1;
        cs1237_info.average_deal_step = 0;
        break;
    }

    default:
    {
        cs1237_info.average_deal_step = 0;
        break;
    }
    }

}

static void cs1237_init(bool refo_off, CS1237_SPEED speed_sel, CS1237_PGA pga_sel, CS1237_CHN ch_sel)
{
    uint8_t data = 0;// i = 0;
    #if ENABLED(USE_WATCHDOG)
    HAL_watchdog_refresh();
    #endif
        cs1237_gpio_init();
        if (refo_off)
        {
            SetBit(data, 6);
        }
        else
        {
            ClrBit(data, 6);
        }

        switch (speed_sel)
        {
        case CS1237_SPEED_10HZ:
        {
            ClrBit(data, 5);
            ClrBit(data, 4);
            break;
        }
        case CS1237_SPEED_40HZ:
        {
            ClrBit(data, 5);
            SetBit(data, 4);
            break;
        }
        case CS1237_SPEED_640HZ:
        {
            SetBit(data, 5);
            ClrBit(data, 4);
            break;
        }
        case CS1237_SPEED_1280HZ:
        {
            SetBit(data, 5);
            SetBit(data, 4);
            break;
        }

        default:
        {
            ClrBit(data, 5);
            ClrBit(data, 4);
            break;
        }
        }

        switch (pga_sel)
        {
        case C1237_PGA_1:
        {
            ClrBit(data, 3);
            ClrBit(data, 2);
            break;
        }
        case C1237_PGA_2:
        {
            ClrBit(data, 3);
            SetBit(data, 2);
            break;
        }
        case C1237_PGA_64:
        {
            SetBit(data, 3);
            ClrBit(data, 2);
            break;
        }
        case C1237_PGA_128:
        {
            SetBit(data, 3);
            SetBit(data, 2);
            break;
        }

        default:
        {
            SetBit(data, 3);
            SetBit(data, 2);
            break;
        }
        }

        switch (ch_sel)
        {
        case C1237_CHN_CHA:
        {
            ClrBit(data, 1);
            ClrBit(data, 0);
            break;
        }
        case C1237_CHN_RST:
        {
            ClrBit(data, 1);
            SetBit(data, 0);
            break;
        }
        case C1237_CHN_TEM:
        {
            SetBit(data, 1);
            ClrBit(data, 0);
            break;
        }
        case C1237_CHN_INS:
        {
            SetBit(data, 1);
            SetBit(data, 0);
            break;
        }

        default:
        {
            ClrBit(data, 1);
            ClrBit(data, 0);
            break;
        }
        }


        cs1237_delay_ms(100);
        #if ENABLED(USE_WATCHDOG)
        HAL_watchdog_refresh();
        #endif
        cs1237_write_config(data);

        MYSERIAL1.printf("\ncs1237_write_config : data = %#x\n", data);
        MYSERIAL2.printf("\ncs1237_write_config : data = %#x\n", data);

        cs1237_delay_ms(100);
        cs1237_info.config = 0;
        #if ENABLED(USE_WATCHDOG)
        HAL_watchdog_refresh();
        #endif
        cs1237_info.config = cs1237_read_config();
        MYSERIAL1.printf("cs1237_read_config : config = %#x\n", cs1237_info.config);
        MYSERIAL2.printf("cs1237_read_config : config = %#x\n", cs1237_info.config);
        
        // if(cs1237_info.config!=data)
        // {
        //     SERIAL_ECHO("ok\r\n");
        //     SERIAL_ERROR_MSG("Pressure sensor initialization failed!!\r\n");
        //     SERIAL_ERROR_MSG(STR_ERR_PROBING_FAILED);
        //     kill();
        // }
        if (cs1237_info.threshold == 0)
        {
            cs1237_info.threshold = CS1237_DEFAULT_THRESHOLD;
        }
}

void cs1237_init_value()
{
    if (cs1237_info.init_status == 0)
    {
        if (cs1237_info.threshold == 0)
        {
            cs1237_info.threshold = CS1237_DEFAULT_THRESHOLD;
        }
        cs1237_info.average_deal_step = 0;
             while (cs1237_info.average_flag == 0)
             {
                #if ENABLED(USE_WATCHDOG)
                HAL_watchdog_refresh();
                #endif
                cs1237_get_average();
            }
            cs1237_info.average_flag = 0;
            cs1237_info.init_value = cs1237_info.average_value;
            cs1237_info.cur_value = cs1237_info.average_value;
            SERIAL_ECHO(" cs1237_info.init_value:");
            SERIAL_ECHO((int32_t)(cs1237_info.init_value));
            SERIAL_ECHO(" \r\n");
        cs1237_info.init_status = 1;
    }
        #if ENABLED(USE_WATCHDOG)
        HAL_watchdog_refresh();
        #endif
}

        // MYSERIAL1.printf("cs1237_info.config = %#x\r\n", (int32_t)cs1237_info.config);
        // MYSERIAL1.printf("cs1237_info.init_value = %d\r\n", (int32_t)cs1237_info.init_value);
        // MYSERIAL1.printf("value:%d\r\n", (int32_t)cs1237_info.cur_value);
        // MYSERIAL1.printf("threshold:%d\r\n", (int32_t)cs1237_info.threshold);

static void cs1237_deal(void)
{
    //static uint32_t ms=0;
   // int32_t temp=0;
    if(cs1237_info.deal_flag != ENABLE)
        return;

    cs1237_get_average();
    // temp=cs1237_read_result();

    // if((temp<=(cs1237_info.init_value+cs1237_info.threshold+200))&&((temp>=(cs1237_info.init_value-cs1237_info.threshold-200))))
    //   cs1237_info.cur_value=temp;
    // {
    // }

    //cs1237_info.cur_value =cs1237_read_result();
      
    // if(millis()>=(ms+20))
    // {
    //    ms=millis();
    //    SERIAL_ECHO(" probe_value:");
    //    SERIAL_ECHO(cs1237_info.cur_value);
    //    SERIAL_ECHO(" \r\n");
    // }

    if (cs1237_info.average_flag == 1)
    {
        cs1237_info.average_flag = 0;
        if((cs1237_info.average_value<=(cs1237_info.init_value+cs1237_info.threshold+200))&&((cs1237_info.average_value>=(cs1237_info.init_value-cs1237_info.threshold-200))))
        {
         cs1237_info.cur_value = cs1237_info.average_value;
        }
    }

    if(anker_z_offset.log)
    {
        SERIAL_ECHO(" 2-probe_value:");
        SERIAL_ECHO(cs1237_info.cur_value);
        SERIAL_ECHO(" \r\n");
    }

    if (cs1237_info.cur_value >= cs1237_info.init_value)
    {
        if (cs1237_info.cur_value - cs1237_info.init_value > cs1237_info.threshold)
        {
            stepper.endstop_triggered(_AXIS(Z));
            anker_z_offset.cs1237_start_convert=false;
            anker_z_offset.cs1237_disable();
            anker_z_offset.cs1237_signal_status = CS1237_VALID_SIGNAL;
            SERIAL_ECHO(" probe more:");
            SERIAL_ECHO((int32_t)(cs1237_info.cur_value - cs1237_info.init_value));
            SERIAL_ECHO(" \r\n");
        }
        else
        {
            anker_z_offset.cs1237_signal_status = CS1237_INVALID_SIGNAL;
        }
    }
    else
    {
        if (cs1237_info.init_value - cs1237_info.cur_value > cs1237_info.threshold)
        {
            stepper.endstop_triggered(_AXIS(Z));
            anker_z_offset.cs1237_start_convert=false;
            anker_z_offset.cs1237_disable();
            anker_z_offset.cs1237_signal_status = CS1237_VALID_SIGNAL;
            SERIAL_ECHO(" probe less:");
            SERIAL_ECHO((int32_t)(cs1237_info.init_value - cs1237_info.cur_value));
            SERIAL_ECHO(" \r\n");
        }
        else
        {
            anker_z_offset.cs1237_signal_status = CS1237_INVALID_SIGNAL;
        }
    }
}

void Anker_Zoffset::init(void)
{
     anker_z_offset.xy[0].x=110;
     anker_z_offset.xy[0].y=210.3;//233-21.7
     anker_z_offset.xy[1].x=110;
     anker_z_offset.xy[1].y=233;//110+123

     cs1237_threshold_set(200);
     CS1237_PWR_CTRL_DISABLE;
     safe_delay(100);
     CS1237_PWR_CTRL_ENABLE;
     cs1237_disable();
     cs1237_info.init_status = 0;
     //CS1237_PWR_CTRL_DISABLE;
     cs1237_init(CS1237_REFO_ON, CS1237_SPEED_40HZ, C1237_PGA_128, C1237_CHN_CHA);
}

void Anker_Zoffset::deal(void)
{
    cs1237_deal();
}

void Anker_Zoffset::loop_read(void)
{
    if(anker_z_offset.loop_read_flag)
    {
      cs1237_get_average();

    if (cs1237_info.average_flag == 1)
        {
            cs1237_info.average_flag = 0;
            // if((cs1237_info.average_value<=(cs1237_info.init_value+cs1237_info.threshold+200))&&((cs1237_info.average_value>=(cs1237_info.init_value-cs1237_info.threshold-200))))
            // {
            cs1237_info.cur_value = cs1237_info.average_value;
            // }
        }

        SERIAL_ECHO(" probe_value:");
        SERIAL_ECHO(cs1237_info.cur_value);
        SERIAL_ECHO(" \r\n");
    }
}

void Anker_Zoffset::cs1237_enable(void)
{
    cs1237_info.deal_flag = ENABLE;
    cs1237_info.init_status = 0;
    anker_z_offset.cs1237_signal_status = CS1237_INVALID_SIGNAL;
}

void Anker_Zoffset::cs1237_disable(void)
{
    cs1237_info.deal_flag = DISABLE;
    anker_z_offset.cs1237_signal_status = CS1237_INVALID_SIGNAL;
}

int32_t Anker_Zoffset::cs1237_threshold_get(void)
{
    return cs1237_info.threshold;
}

void Anker_Zoffset::cs1237_threshold_set(int32_t threshold)
{
    cs1237_info.threshold = threshold;
}

void Anker_Zoffset::reset_init()
{
     CS1237_PWR_CTRL_DISABLE;
     safe_delay(100);
     CS1237_PWR_CTRL_ENABLE;
     cs1237_disable();
     cs1237_info.init_status = 0;
     cs1237_init(CS1237_REFO_ON, CS1237_SPEED_40HZ, C1237_PGA_128, C1237_CHN_CHA);
}

#define ANKER_Z_OFFSET_ERROR 0.04

int anker_del_max_min(float *a, int n )
{
    int maxi, mini,i,j;
    maxi=mini=0;
    for(i = 1;i<n; i++)
        if(a[maxi]<a[i]) maxi=i;
        else if(a[mini]>a[i])mini=i;
    for(i=j=0;i<n;i++)
        if(i!=maxi&&i!=mini)
            a[j++]=a[i];
    return j;
}

void Anker_Zoffset::run()
{
  // Start with current offsets and modify
  static int16_t save_hot_temp=0,save_bed_temp=0;
  float probe_z1=0,probe_z2=0;

   for(uint8_t i=0;i<5;i++)
   {
  
    save_hot_temp=thermalManager.degTargetHotend(0);
    save_bed_temp=thermalManager.degTargetBed();
    
    anker_z_offset.cs1237_start_convert=false;
    // SERIAL_ECHO(" anker_z_offset.cs1237_start_convert=:");
    // SERIAL_ECHO(anker_z_offset.cs1237_start_convert);
    // SERIAL_ECHO(" \r\n");

    probe_z1 = probe.anker_z_ofset_probe_at_point(anker_z_offset.xy[0], PROBE_PT_NONE, 0, true, false, false);
    SERIAL_ECHO(" probe_z1:");
    SERIAL_ECHO(probe_z1);
    SERIAL_ECHO(" \r\n");
   
    probe_z2 = probe.anker_z_ofset_probe_at_point(anker_z_offset.xy[1], PROBE_PT_NONE, 0, true, false, true);
    SERIAL_ECHO(" probe_z2:");
    SERIAL_ECHO(probe_z2);
    SERIAL_ECHO(" \r\n");
    
    anker_z_offset.value[i]=probe_z2-probe_z1;
    
    do_blocking_move_to_z(current_position.z+5);

    thermalManager.setTargetHotend(save_hot_temp,0);
    thermalManager.setTargetBed(save_bed_temp);
    
    } 
    //gcode.process_subcommands_now_P(PSTR("M109 S0\n"));

    anker_del_max_min(anker_z_offset.value,5);
    
    save_probe_offset_z=(anker_z_offset.value[0]+anker_z_offset.value[1]+anker_z_offset.value[2])/3+ANKER_Z_OFFSET_ERROR;
    
    SERIAL_ECHO(" 16_save_probe_offset_z:");
    SERIAL_ECHO(save_probe_offset_z);
    SERIAL_ECHO(" \r\n");

    //cs1237_init(CS1237_REFO_ON, CS1237_SPEED_40HZ, C1237_PGA_128, C1237_CHN_CHA);
}

void Anker_Zoffset::update_offset(void)
{
    probe.offset.z=save_probe_offset_z;
    gcode.process_subcommands_now_P(PSTR("M851\n"));
}

#endif /* ANKER_Z_OFFSET_FUNC */


#if ENABLED(ANKER_PROBE_SET)
#include "../interactive/uart_nozzle_rx.h"
#include "../../gcode/gcode.h"
#include "../../module/probe.h"
    Anker_Probe_set anker_probe_set;
    uint16_t Anker_Probe_set::homing_value=HOMING_PROBE_VALUE;
    uint16_t Anker_Probe_set::leveing_value=LEVEING_PROBE_VALUE;
    uint16_t Anker_Probe_set::delay=LEVEING_PROBE_DELAY;
    uint8_t Anker_Probe_set::run_step=0;
    bool Anker_Probe_set::auto_run_flag=false;
    bool Anker_Probe_set::point_test_flag=false;
    xy_pos_t Anker_Probe_set::xy[5];

    void Anker_Probe_set::probe_start(uint16_t value)
    {
       #if ADAPT_DETACHED_NOZZLE
       safe_delay(anker_probe_set.delay);
       uart_nozzle_tx_probe_val(value);
       TERN_(ANKER_PROBE_SET, safe_delay(anker_probe_set.delay));
       SERIAL_ECHO("probe_start\r\n");
       #else
       MYSERIAL1.printf("M2012 S%d\n",value);
       #endif
    }
    void Anker_Probe_set::reset_value()
    {
       anker_probe_set.homing_value=HOMING_PROBE_VALUE;
       anker_probe_set.leveing_value=LEVEING_PROBE_VALUE;
    }
    void Anker_Probe_set::report_value()
    {
        SERIAL_ECHO("anker probe homing value: ");
        SERIAL_ECHO(anker_probe_set.homing_value);
        SERIAL_ECHO("\r\nanker probe leveing value: ");
        SERIAL_ECHO(anker_probe_set.leveing_value);
        SERIAL_ECHO("\r\n");
    }
    
    void Anker_Probe_set::get_probe_value()
    {
       uart_nozzle_tx_probe_val_get();
    }

    void Anker_Probe_set::show_adc_value(bool show_adc)
    {
        if(show_adc)
        {
         uart_nozzle_tx_show_adc_value_on();
        }
        else
        {
         uart_nozzle_tx_show_adc_value_off(); 
        }
    }
    
    void Anker_Probe_set::pid_autotune(uint16_t temp, uint16_t ncyclesc)
    {
       uart_nozzle_tx_pid_autotune(temp,ncyclesc);
       SERIAL_ECHO("PID AUTOTUNE START \r\n");
    }

    void Anker_Probe_set::run()
    {

       switch (anker_probe_set.run_step)
       {
       case 0:
          gcode.process_subcommands_now_P(PSTR("M104 S230\nM109 S230\n"));
          gcode.process_subcommands_now_P(PSTR("G28\n"));
          gcode.process_subcommands_now_P(PSTR("G1 Z80 F900\n"));
          gcode.process_subcommands_now_P(PSTR("G1 X110 Y110 F6000\n"));
          anker_probe_set.run_step=1;
        break;
        case 1:  
          gcode.process_subcommands_now_P(PSTR("G2001\n"));          
        break;       
       default:
        break;
       }
    }

    void Anker_Probe_set::home_delay()
    {
       safe_delay(PROBE_READ_NUM_DELAY);
    }

    void Anker_Probe_set::auto_run()
    {
        gcode.process_subcommands_now_P(PSTR("M104 S230\nM109 S230\n")); 
        anker_probe_set.auto_run_flag=true;
        gcode.process_subcommands_now_P(PSTR("G28\n"));
    }

    void Anker_Probe_set::auto_run_send_start_info()
    {
       safe_delay(anker_probe_set.delay);
       uart_nozzle_tx_auto_offset_start();
       TERN_(ANKER_PROBE_SET, safe_delay(anker_probe_set.delay));
       SERIAL_ECHO("auto_run_send_start_info\r\n");
    }

    void Anker_Probe_set::point_test_ready()
    {
        anker_probe_set.xy[0].x=0;
        anker_probe_set.xy[0].y=110;
        anker_probe_set.xy[1].x=55;
        anker_probe_set.xy[1].y=110;
        anker_probe_set.xy[2].x=110;
        anker_probe_set.xy[2].y=110;
        anker_probe_set.xy[3].x=165;
        anker_probe_set.xy[3].y=110;
        anker_probe_set.xy[4].x=220;
        anker_probe_set.xy[4].y=110;   

        gcode.process_subcommands_now_P(PSTR("M104 S230\nM109 S230\n"));
        gcode.process_subcommands_now_P(PSTR("G28\n"));
        gcode.process_subcommands_now_P(PSTR("G1 Z80 F900\n"));
        gcode.process_subcommands_now_P(PSTR("G1 X110 Y110 F6000\n"));   
        anker_probe_set.point_test_flag=true;
    }
    
    void Anker_Probe_set::point_test(uint8_t point)
    {
       probe.probe_at_point(anker_probe_set.xy[point], PROBE_PT_RAISE, 0, true, false);      
    }

    void Anker_Probe_set::point_test_idle()
    {
        safe_delay(2500);
        SERIAL_ECHO("echo:ready to read\r\n");
        safe_delay(1000);
    }
#endif /* ANKER_PROBE_SET */

#endif /* ANKER_MAKE_API */

