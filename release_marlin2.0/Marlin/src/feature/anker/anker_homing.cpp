/*
 * @Author       : harley
 * @Date         : 2022-04-31 20:35:23
 * @LastEditors  : harley
 * @LastEditTime : 
 * @Description  :
 */
#include "anker_homing.h"
#include "../bedlevel/bedlevel.h"
#if ENABLED(EVT_HOMING_5X)

#include "../../gcode/gcode.h"
#include "../../module/temperature.h"
#include "../../MarlinCore.h"

  Anker_Homing anker_homing;
  uint8_t Anker_Homing::filament_type = FILAMENT_PLA;

    #if ENABLED(NO_MOTION_BEFORE_HOMING)
     void Anker_Homing:: anker_disable_motton_before_init()
       {
         pinMode(DISABLED_MOTION_BEFORE_PIN, INPUT_PULLUP);
       }
      void Anker_Homing:: anker_disable_motton_before_check()
       {
         if(READ(DISABLED_MOTION_BEFORE_PIN)==DISABLED_MOTION_BEFORE_STATE) 
         {
           anker_homing.no_check_all_axis=true;
         } 
         else
         {
           anker_homing.no_check_all_axis=false;
         }
       }
    #endif 

    #if ENABLED(USE_Z_SENSORLESS)
    #include "../../module/motion.h"
    #include "../../module/probe.h"
    #include "anker_align.h"
    #include "anker_nozzle_board.h"

        void Anker_Homing:: set_first_end_z_axis(ANKER_Z_AXIS z)
        {
          first_end_z_axis=z;
          //SERIAL_ECHO(" first_end_z_axis:");
          //SERIAL_ECHO(z);
          //SERIAL_ECHO("\r\n");
        }
        ANKER_Z_AXIS Anker_Homing:: get_first_end_z_axis()
        {
          return first_end_z_axis;
        }
        #define FIRST_END_Z1_POS_X              235
        #define FIRST_END_Z1_POS_Y              237  
        #define FIRST_END_Z2_POS_X              0
        #define FIRST_END_Z2_POS_Y              237 
        #define DUAL_Z_NOZZLE_BELOW_BED_MM      4     //The height of the nozzle below the heated bed when the double Z is locked and reset to zero
        #define DUAL_Z_NOZZLE_BELOW_BED_STEP_MM 0.1
        #define HOMING_RUN_ALIGN_MM             20
        #define HOMING_RUN_ALIGN_STEP_MM        0.1
        // void Anker_Homing:: anker_dual_z_run_align()
        // {
        //    bool run_align=true;
        //    xyz_pos_t first_z1_pos = { FIRST_END_Z1_POS_X, FIRST_END_Z1_POS_Y, current_position.z };
        //    xyz_pos_t first_z2_pos = { FIRST_END_Z2_POS_X, FIRST_END_Z2_POS_Y, current_position.z };
        //   if(first_end_z_axis==Z_AXIS_IS_Z1)
        //   {
        //     do_blocking_move_to(first_z1_pos, feedRate_t(XY_PROBE_FEEDRATE_MM_S)); 
        //   }
        //   else if(first_end_z_axis==Z_AXIS_IS_Z2)
        //   {
        //     do_blocking_move_to(first_z2_pos, feedRate_t(XY_PROBE_FEEDRATE_MM_S));              
        //   }

        //    set_anker_z_sensorless_probe_value(800);
        //   #if ENABLED(PROVE_CONTROL)
        //     digitalWrite(PROVE_CONTROL_PIN, !PROVE_CONTROL_STATE);
        //     probe.anker_level_set_probing_paused(true,ANKER_LEVEING_DELAY_BEFORE_PROBING);
        //     digitalWrite(PROVE_CONTROL_PIN, PROVE_CONTROL_STATE);
        //   #endif
        //   for(u_int16_t i=0;i<(DUAL_Z_NOZZLE_BELOW_BED_MM/DUAL_Z_NOZZLE_BELOW_BED_STEP_MM);i++)
        //   {
        //      do_blocking_move_to_z(current_position.z-DUAL_Z_NOZZLE_BELOW_BED_STEP_MM);
        //      if(PROBE_TRIGGERED())
        //      {
        //        SERIAL_ECHO(" probe first triggered!\r\n");
        //        run_align=false;
        //        break;
        //      }
        //   }
        //   if(run_align)
        //   {
        //       run_align=false;
        //       if(first_end_z_axis==Z_AXIS_IS_Z1)
        //       {
        //         for(u_int16_t i=0;i<(HOMING_RUN_ALIGN_MM/HOMING_RUN_ALIGN_STEP_MM);i++)
        //         {
        //           anker_align.add_z2_value(-HOMING_RUN_ALIGN_STEP_MM);
        //           if(PROBE_TRIGGERED())
        //           {
        //             SERIAL_ECHO("right probe triggered!\r\n");
        //             break;
        //           }
        //         }
        //       }
        //       else if(first_end_z_axis==Z_AXIS_IS_Z2)
        //       {
        //         for(u_int16_t i=0;i<(HOMING_RUN_ALIGN_MM/HOMING_RUN_ALIGN_STEP_MM);i++)
        //         {
        //           anker_align.add_z1_value(-HOMING_RUN_ALIGN_STEP_MM);
        //           if(PROBE_TRIGGERED())
        //           {
        //             SERIAL_ECHO("lift probe triggered!\r\n");
        //             break;
        //           }
        //         }                
        //       }
        //   }
           
            
        //   #if ENABLED(PROVE_CONTROL)
        //     digitalWrite(PROVE_CONTROL_PIN, !PROVE_CONTROL_STATE);
        //   #endif
        //   reset_anker_z_sensorless_probe_value();
        // }

        void Anker_Homing:: set_triger_per_ms()
        {
          anker_homing.trigger_per_ms=millis();
        }
        void Anker_Homing:: set_triger_ms()
        {
          anker_homing.trigger_ms=millis();
          SERIAL_ECHO(" trigger!!\r\n");
        }
        bool Anker_Homing:: is_z_top_triger()
        {
          SERIAL_ECHO(" trigger_per_ms:");
          SERIAL_ECHO(anker_homing.trigger_per_ms);
          SERIAL_ECHO("\r\n");
          SERIAL_ECHO(" trigger_ms:");
          SERIAL_ECHO(anker_homing.trigger_ms);
          SERIAL_ECHO("\r\n");
          if(anker_homing.trigger_ms<(anker_homing.trigger_per_ms+ANTHER_TIME_Z_MIN_LIMIT))
          return true;
          else return false;
        }
        bool Anker_Homing:: is_anthor_z_no_triger()
        {
          SERIAL_ECHO(" anthor_z trigger_per_ms:");
          SERIAL_ECHO(anker_homing.trigger_per_ms);
          SERIAL_ECHO("\r\n");
          SERIAL_ECHO(" anthor_z trigger_ms:");
          SERIAL_ECHO(anker_homing.trigger_ms);
          SERIAL_ECHO("\r\n");
          if(anker_homing.trigger_ms>(anker_homing.trigger_per_ms+ANTHER_TIME_ANTHOR_Z_MAX_LIMIT))
          return true;
          else return false;
        }
        void Anker_Homing:: set_probe_triger_ms()
        {
          anker_homing.trigger_ms=millis();
          SERIAL_ECHO(" probe trigger!!\r\n");
        }
        bool Anker_Homing:: is_z_probe_no_triger()//If the probe does not trigger for a long time, reset Z to zero
        {
          if(millis()>(anker_homing.trigger_per_ms+ANKER_PROBE_TIMEOUT))
          return true;
          else return false;         
        }
        #endif
        bool Anker_Homing::is_center_home() //Determine whether the center point needs to be zeroed
        {
          #if ENABLED(USE_Z_SENSORLESS)
           return anker_homing.anker_z_homing_options&&!anker_homing.is_again_probe_homing;
          #else
           return anker_homing.anker_z_homing_options;
          #endif
        }
        void Anker_Homing:: xy_triger()
        {
          SERIAL_ECHO(" xy trigger!!\r\n");
        }

        void Anker_Homing::Wiping_mouth_action(const char* TPU_Action, const char*  other1, const char*  other2)
        {
          gcode.set_relative_mode(false);
          if(FILAMENT_TPU == anker_homing.filament_type){
            gcode.process_subcommands_now_P(TPU_Action);
          }else{
            gcode.process_subcommands_now_P(other1);
            gcode.process_subcommands_now_P(other2);
          }
        }


        void Anker_Homing::after_homing_action(void)
        {
          if(is_center_home())
          {
              if(is_clean)
              {
                if(get_e_is_absolute())
                {
                  Wiping_mouth_action(ANKER_TPU_HOMING_SCRIPT_ABSOLUTE0, ANKER_HOMING_SCRIPT_ABSOLUTE0, ANKER_HOMING_SCRIPT_ABSOLUTE1);
                }
                else
                {
                  Wiping_mouth_action(ANKER_TPU_HOMING_SCRIPT_NO_ABSOLUTE0, ANKER_HOMING_SCRIPT_NO_ABSOLUTE0, ANKER_HOMING_SCRIPT_NO_ABSOLUTE1);
                  gcode.set_e_relative();
                }
                
                #if ENABLED(WS1_HOMING_5X)
                  WS1_do_z_clearance(ANKER_Z_AFTER_HOMING,true);
                #endif
              }
              anker_leve_pause=true;
              anker_z_homing_options=false;
              gcode.process_subcommands_now_P(PSTR("G2001\n"));
          }

          #if ENABLED(USE_Z_SENSORLESS)
            if(is_again_probe_homing)//Requires repeated zeroing of Z
            {
              is_again_probe_homing=false;
              is_angan_homing_z_num++;
              if(is_angan_homing_z_num>=ANKER_Z_AGAIN_HOMING_NUM)  
              {
                MYSERIAL2.printLine("Error:Homing Error Z_AXIS\r\n");
                kill(GET_TEXT(MSG_KILL_HOMING_FAILED));
              }
              gcode.process_subcommands_now_P(PSTR("G28 Z\n"));
            }
            else
            {
              is_angan_homing_z_num=0;
            }
          #endif
        }

        void Anker_Homing::after_align_action(celsius_t targetTemperature)
        {
          char pcmd[128]="";
          if(FILAMENT_TPU == anker_homing.filament_type && targetTemperature > 0 && true == anker_homing.is_clean){
            sprintf(pcmd,"G1 X-10 Y200 F12000\nM109 S%d\nG92 E0\nG1 X-5 F600\nG1 Z0.2\nG1 Y150 E12\nG0 Y145\nG1 E11 F1800\nG92 E0\nG1 Z2\nG0 F12000\n", targetTemperature);
            MYSERIAL2.printLine("echo: Return to the set temperature(%d)\r\n",targetTemperature);
            gcode.process_subcommands_now_P(pcmd); // Restoring temperature after the process of wiping mouth
          }
        }
#endif