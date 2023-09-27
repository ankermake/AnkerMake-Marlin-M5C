/*
 * @Author       : Anan
 * @Date         : 2023-06-09 09:25:24
 * @LastEditors  : Anan
 * @LastEditTime : 2023-06-17 16:28:00
 * @Description  : Dynamically tracking the motion planning situation
 */

#include "../../inc/MarlinConfig.h"
#if ENABLED(ANKER_MOTION_TRACKING)
#include "anker_motion_tracking.h"
#include "../../core/serial.h"
#include "../../module/planner.h"
#include "../../module/stepper.h"

#ifdef DEBUG_TP172
  #define TP172_INIT()    pinMode(DEBUG_TP172, OUTPUT)
  #define TP172_TOGGLE()  TOGGLE(DEBUG_TP172)
  #define TP172_HIGH()    WRITE(DEBUG_TP172,HIGH)
  #define TP172_LOW()     WRITE(DEBUG_TP172,LOW)
  #define TP173_INIT()    pinMode(DEBUG_TP173, OUTPUT)
  #define TP173_TOGGLE()  TOGGLE(DEBUG_TP173)
  #define TP173_HIGH()    WRITE(DEBUG_TP173,HIGH)
  #define TP173_LOW()     WRITE(DEBUG_TP173,LOW)
#endif

enum TRACK_MODE{ NONE_TRACK, POS_TRACK, PLR_TRACK};

#pragma pack(push, 1) 
typedef struct
{
    // Fields used by the motion planner to manage acceleration
    float nominal_speed_sqr,                  // The nominal speed for this block in (mm/sec)^2
          entry_speed_sqr,                    // Entry speed at previous-current junction in (mm/sec)^2
          max_entry_speed_sqr,                // Maximum allowable junction entry speed in (mm/sec)^2
          millimeters,                        // The total travel of this block in mm
          acceleration;                       // acceleration mm/sec^2
    uint32_t la_advance_rate;               // The rate at which steps are added whilst accelerating
    uint16_t max_adv_steps,                 // Max advance steps to get cruising speed pressure
            final_adv_steps;               // Advance steps for exit speed pressure
    la_block_bits_t la_current;
    uint32_t nominal_rate,                    // The nominal step rate for this block in step_events/sec
            initial_rate,                    // The jerk-adjusted step rate at start of block
            final_rate,                      // The minimal rate at exit
            acceleration_steps_per_s2;       // acceleration steps/sec^2
    uint32_t accelerate_until,                // The index of the step event on which to stop acceleration
            step_event_count,
            decelerate_after;                // The index of the step event on which to start decelerating
}planner_track_t;
#pragma pack(pop) 

typedef struct
{
  #define POS_LEN 100 // Cannot exceed 512
  millis_t  ms[POS_LEN];
  xyze_long_t pos[POS_LEN];
}pos_track_t;

typedef struct
{
  #define PLR_LEN 32 // Cannot exceed 512
  union{
    pos_track_t p;
    planner_track_t l[PLR_LEN];
  };
  volatile uint8_t head, tail; // Cannot exceed 512
  volatile uint8_t count;
  uint32_t ms_tick;
  TRACK_MODE mode;
}motion_track_t; // motion data cache receiving structure.

static motion_track_t m_track ={
  .head = 0,
  .tail = 0,
  .count = 0,
  .mode = NONE_TRACK,
};

#pragma pack(push, 1) 
typedef struct
{
  uint8_t header0;
  uint8_t header1;
  uint8_t count;
  uint8_t checksum;
  union ms_t
  {
    millis_t  ms;
    uint8_t content[sizeof(ms)];
  }m;
  union pos_t
  {
    xyze_long_t pos;
    uint8_t content[sizeof(pos)];
  }c;
} motion_pack_t; // Data packing structure.
#pragma pack(pop) 

#pragma pack(push, 1) 
typedef struct
{
  uint8_t header0;
  uint8_t header1;
  uint8_t count;
  union pos_t
  {
    planner_track_t plr;
    uint8_t content[sizeof(plr)];
  }c;
  uint8_t checksum;
} plr_pack_t; // Data packing structure.
#pragma pack(pop) 

static motion_pack_t m_pack;
static plr_pack_t p_pack;


Motion_Track motion_track;


/**
  * @brief  Motion monitoring timer interrupt, currently set to 1kHz interrupt.
  * @param  None
  * @retval None
  */
HAL_MOTION_TRACK_ISR() { // Consume time = 750ns 
  TP173_HIGH();
  HAL_timer_isr_prologue(MOTION_TRACK_TIMER_NUM);
  if(m_track.count < POS_LEN-1){
    m_track.p.pos[m_track.head].x =  stepper.position(X_AXIS);//16909060;//
    m_track.p.pos[m_track.head].y =  stepper.position(Y_AXIS);//84281096;//
    m_track.p.pos[m_track.head].z =  stepper.position(Z_AXIS);//151653132;//
    m_track.p.pos[m_track.head].e =  stepper.position(E_AXIS);//270544960;//
    m_track.ms_tick++;//0xf0f0f0f0;//
    m_track.p.ms[m_track.head] = m_track.ms_tick;
    m_track.head = (m_track.head + 1) % POS_LEN;
    m_track.count++;
  }
  HAL_timer_isr_epilogue(MOTION_TRACK_TIMER_NUM);
  TP173_LOW();
}

void Motion_Track::block_info_record(const void* ptr) { // 800NS
    //TP173_HIGH();
    if(PLR_TRACK == m_track.mode && m_track.count < PLR_LEN-1){// Trigger and close block information record by G0_1 W<active>
      const block_t* currentbBlock = (const block_t*)ptr; // To convert a `void*` pointer to a `const block_t*` pointer
      planner_track_t* pbuff = &(m_track.l[m_track.head]);
      pbuff->nominal_speed_sqr = currentbBlock->nominal_speed_sqr;
      pbuff->entry_speed_sqr = currentbBlock->entry_speed_sqr;
      pbuff->max_entry_speed_sqr = currentbBlock->max_entry_speed_sqr;
      pbuff->millimeters = currentbBlock->millimeters;
      pbuff->acceleration = currentbBlock->acceleration;
      pbuff->la_advance_rate = currentbBlock->la_advance_rate;
      pbuff->max_adv_steps = currentbBlock->max_adv_steps;
      pbuff->final_adv_steps = currentbBlock->final_adv_steps;
      pbuff->la_current = currentbBlock->la_segment;
      pbuff->nominal_rate = currentbBlock->nominal_rate;
      pbuff->initial_rate = currentbBlock->initial_rate;
      pbuff->final_rate = currentbBlock->final_rate;
      pbuff->acceleration_steps_per_s2 = currentbBlock->acceleration_steps_per_s2;
      pbuff->accelerate_until = currentbBlock->accelerate_until;
      pbuff->step_event_count = currentbBlock->step_event_count;
      pbuff->decelerate_after = currentbBlock->decelerate_after;
      m_track.head = (m_track.head + 1) % PLR_LEN;
      m_track.ms_tick++;
      m_track.count++;
    }
    //TP173_LOW();
}


/**
  * @brief  Timer start/stop switch
  * @param  None
  * @retval None
  */
void Motion_Track::contorl(uint8_t switch_flag)
{
  switch (m_track.mode)
  {
    case POS_TRACK:
      if(true == switch_flag){
        HAL_timer_start(MOTION_TRACK_TIMER_NUM, 1000);
        ENABLE_MOTION_TRACK_INTERRUPT();
      }
      else
        DISABLE_MOTION_TRACK_INTERRUPT();
      break;

    case PLR_TRACK:
      DISABLE_MOTION_TRACK_INTERRUPT(); 
      break;

    default:
      DISABLE_MOTION_TRACK_INTERRUPT(); 
      break;
  }

}

/**
  * @brief  Switching data acquisition modes.
  * @param  None
  * @retval None
  */
void Motion_Track::mode(uint8_t mode)
{
  switch (mode)
  {
    case POS_TRACK:
      HAL_timer_start(MOTION_TRACK_TIMER_NUM, 1000);
      ENABLE_MOTION_TRACK_INTERRUPT();
      m_track.mode = POS_TRACK;
      break;

    case PLR_TRACK:
      m_track.mode = PLR_TRACK;
      DISABLE_MOTION_TRACK_INTERRUPT(); 
      break;

    default:
      m_track.mode = NONE_TRACK;
      DISABLE_MOTION_TRACK_INTERRUPT(); 
      break;
  }
}


/**
  * @brief  Pack the data in m_track into the msg structure, add a checksum, and send it out through the serial port
  * @param  None
  * @retval None
  */
void position_msg_pack(motion_pack_t *msg, xyze_long_t pos, millis_t ms)
{
  uint16_t i;
  uint8_t sum  = 0;
  msg->header0 = 0xAA;
  msg->header1 = 0x55;
  
  msg->c.pos = pos;
  msg->count++;
  msg->m.ms = ms;
  sum += msg->header0;
  sum += msg->header1;
  sum += msg->count;
  for (i = 0; i < sizeof(millis_t); i++)
      sum += msg->m.content[i];
  for (i = 0; i < sizeof(xyze_long_t); i++)
      sum += msg->c.content[i];

  msg->checksum = sum;
  MYSERIAL1.send((uint8_t *)msg, sizeof(motion_pack_t)); 
}
/**
  * @brief  Pack the data in m_track into the msg structure, add a checksum, and send it out through the serial port
  * @param  None
  * @retval None
  */
void planner_msg_pack(plr_pack_t *msg, const planner_track_t plr)
{
  uint16_t i;
  uint8_t sum  = 0;
  msg->header0 = 0xAA;
  msg->header1 = 0x55;
  
  msg->c.plr = plr;
  msg->count++;
  sum += msg->header0;
  sum += msg->header1;
  sum += msg->count;
  for (i = 0; i < sizeof(planner_track_t); i++)
      sum += msg->c.content[i];

  msg->checksum = sum;
  MYSERIAL1.send((uint8_t *)msg, sizeof(plr_pack_t)); 
}

/**
  * @brief  init
  * @param  None
  * @retval None
  */
void Motion_Track::init(void)
{
  
  TP172_INIT();
  TP173_INIT();

  switch (m_track.mode)
  {
    case POS_TRACK:
      HAL_timer_start(MOTION_TRACK_TIMER_NUM, 1000);
      //ENABLE_MOTION_TRACK_INTERRUPT(); 
      DISABLE_MOTION_TRACK_INTERRUPT();
      break;

    case PLR_TRACK:
      //HAL_timer_start(MOTION_TRACK_TIMER_NUM, 1000);
      DISABLE_MOTION_TRACK_INTERRUPT(); 
      break;

    default:
      break;
  }
}




/**
  * @brief  Send the cached data received in the timer to the PC at a scheduled time
  * @param  None
  * @retval None
  */
void Motion_Track::msg_sending(void)
{
  uint8_t Number_of_runs = 5; // Maximum number of sends per run
  if(m_track.count && Number_of_runs){
    TP172_HIGH();
    switch (m_track.mode)
    {
      case POS_TRACK: // Consume time = 5US 
        position_msg_pack(&m_pack, m_track.p.pos[m_track.tail], m_track.p.ms[m_track.tail]);
        m_track.tail = (m_track.tail + 1) % POS_LEN;
        break;

      case PLR_TRACK:// 5.5US
        planner_msg_pack(&p_pack, m_track.l[m_track.tail]);
        m_track.tail = (m_track.tail + 1) % PLR_LEN;
        break;

      default:
        break;
    }

    m_track.count--;
    Number_of_runs--;
    TP172_LOW();
  }
}

#endif
