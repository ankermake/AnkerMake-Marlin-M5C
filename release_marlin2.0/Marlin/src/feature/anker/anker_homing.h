/*
 * @Author       : harley
 * @Date         : 2022-04-31 20:35:23
 * @LastEditors  : harley
 * @LastEditTime : 
 * @Description  :
 */
#include "../../inc/MarlinConfig.h"


#if ENABLED(ANKER_MAKE_API)
   #include "../../gcode/gcode.h"
   typedef enum
   {
      FILAMENT_PLA   = 0,
      FILAMENT_ABS   = 1,
      FILAMENT_TPU   = 2,
      FILAMENT_PETG  = 3,
      FILAMENT_PVA   = 4,
      FILAMENT_PA    = 5,
      FILAMENT_OTHER = 6,
      FILAMENT_NONE
   } FILAMENT_TYPE;

  #if ENABLED(USE_Z_SENSORLESS) 
   typedef enum
   {
      Z_AXIS_IDLE = 0,
      Z_AXIS_IS_Z1,
      Z_AXIS_IS_Z2,
   } ANKER_Z_AXIS;
  #endif

   class Anker_Homing {
    public:
     #if ENABLED(NO_MOTION_BEFORE_HOMING)
         bool no_check_all_axis;
         void anker_disable_motton_before_init();
         void anker_disable_motton_before_check();
     #endif
      bool is_home_z=false;
      bool is_clean=false;
      bool anker_z_homing_options=false;//Actions that need to be performed only when zeroing Z
      static uint8_t filament_type;
     #if ENABLED(USE_Z_SENSORLESS)
        int is_angan_homing_z_num=0;
        bool is_again_probe_homing=false;
        ANKER_Z_AXIS first_end_z_axis=Z_AXIS_IDLE;//Record the Z axis that reaches the end point first
        millis_t trigger_per_ms;
        millis_t trigger_ms;
        void set_first_end_z_axis(ANKER_Z_AXIS);
        ANKER_Z_AXIS get_first_end_z_axis();
        //void anker_dual_z_run_align();//Roughly level the two Z axes
        void set_triger_per_ms();
        void set_triger_ms();  
        void set_probe_triger_ms();
        bool is_z_top_triger();      
        bool is_anthor_z_no_triger();//The other axis is not firing  
        bool is_z_probe_no_triger();
     #endif
       bool is_center_home(); //Determine whether the center point needs to be zeroed
       void xy_triger();
       void Wiping_mouth_action(const char* TPU_Action, const char*  other1, const char*  other2);
       void after_homing_action(void);
       void after_align_action(celsius_t targetTemperature);
    };

   extern  Anker_Homing anker_homing;

#endif
