/*
 * @Author       : harley
 * @Date         : 2022-04-27 20:35:23
 * @LastEditors  : harley
 * @LastEditTime : 
 * @Description  :
 */
#include "../../inc/MarlinConfig.h"

#if ENABLED(ANKER_ANLIGN)
  #include "anker_align.h"
  #include "../../module/stepper.h"
  #include "../../module/probe.h"
  #include "../../gcode/gcode.h"
  #include "../../module/temperature.h"
  #include "anker_z_offset.h"
  #include "../../module/endstops.h"

  #include "../../feature/anker/anker_homing.h"

  #if ADAPT_DETACHED_NOZZLE
  #include "../interactive/uart_nozzle_tx.h"
  #endif

  Anker_Align anker_align;

    xy_pos_t Anker_Align::xy[NUM_Z_STEPPER_DRIVERS];
    void Anker_Align:: init(void)
    {
      anker_align.xy[0].x=PROBING_MARGIN;
      anker_align.xy[0].y=Y_CENTER;
      anker_align.xy[1].x=X_BED_SIZE-PROBING_MARGIN;
      anker_align.xy[1].y=Y_CENTER;
    }
    void Anker_Align:: add_z_value(float value,uint8_t choose)
    {
       stepper.set_separate_multi_axis(true);
       // Lock all steppers except one
       stepper.set_all_z_lock(true, choose);
       // Do a move to correct part of the misalignment for the current stepper
       do_blocking_move_to_z(current_position.z+value);
 
       // Back to normal stepper operations
       stepper.set_all_z_lock(false);
       stepper.set_separate_multi_axis(false);   
    }
    void Anker_Align:: add_z1_value(float value)
    {
       anker_align.z1_value+=value;
       anker_align.add_z_value(value,0);
    }
    void Anker_Align:: add_z2_value(float value)
    {
       anker_align.z2_value+=value;
       anker_align.add_z_value(value,1);  
    }
    void Anker_Align:: add_z1_value_no_save(float value)
    {
       anker_align.add_z_value(value,0); 
    }
    void Anker_Align:: add_z2_value_no_save(float value)
    {
       anker_align.add_z_value(value,1);
    }
    void Anker_Align::run_align(void)
    {
       float value=0;
       value=anker_align.z1_value;
       anker_align.z1_value=0;
       anker_align.add_z1_value(value);  
       value=anker_align.z2_value;
       anker_align.z2_value=0;
       anker_align.add_z2_value(value);
    }
    void Anker_Align::reset(void)
    {
       anker_align.z1_value=0;
       anker_align.z2_value=0;
       anker_align.run_align();
    }

    void Anker_Align::auto_align(void)
    {
     uint16_t num=0;
     const ProbePtRaise raise_after =  PROBE_PT_RAISE;

     #if ENABLED(Z_MULTI_ENDSTOPS)
      const float z2_endstop_adj = endstops.z2_endstop_adj;
      endstops.z2_endstop_adj = 0.0f;
     #endif

     anker_align.init();
     #ifdef  ALIGN_PER_RESET
      anker_align.reset();
     #endif
     anker_align.g36_running_flag = true;

     const celsius_t targetTemperature = thermalManager.degTargetHotend(0);  // Reducing temperature during the process of wiping mouth

     gcode.process_subcommands_now_P(PSTR("G28"));

     anker_probe_set.delay=800;
     for(num=0;num<ANLIGN_NUM;num++)
     {
       do_blocking_move_to_z(current_position.z+ANLIGN_RISE);
       #if ADAPT_DETACHED_NOZZLE
         uart_nozzle_tx_point_type(POINT_G36, num);
       #endif
       const float z1 = probe.probe_at_point(anker_align.xy[0], raise_after, 0, true, false);
       do_blocking_move_to_z(current_position.z+ANLIGN_RISE);
       safe_delay(600);
       const float z2 = probe.probe_at_point(anker_align.xy[1], raise_after, 0, true, false);
       float rise_z=0;
         rise_z=(SCREW_DISTANCE/(anker_align.xy[1].x-anker_align.xy[0].x))*ABS(z1-z2);
         SERIAL_ECHO(" z1:");
         SERIAL_ECHO(z1);
         SERIAL_ECHO(" \r\n");
         SERIAL_ECHO(" z2:");
         SERIAL_ECHO(z2);
         SERIAL_ECHO(" \r\n");

       if(ABS(z1-z2)>ANLIGN_MAX_VALUE || ANKER_OVERPRESSURE_TRIGGER())
       {
         ANKER_CLOSED_OVERPRESSURE_TRIGGER();
         TERN_(ADAPT_DETACHED_NOZZLE, uart_nozzle_tx_notify_error());
         SERIAL_ECHO("ok\r\n");
         SERIAL_ERROR_MSG("Adjustment range over 1.5mm!!\r\n");
         SERIAL_ERROR_MSG(STR_ERR_PROBING_FAILED);
         endstops.z2_endstop_adj = z2_endstop_adj;
         anker_align.g36_running_flag = false;
         kill();
       }
       if(ABS(z1-z2)<=ANLIGN_ALLOWED)
       {
         SERIAL_ECHO("echo:anlign ok!\r\n");
         SERIAL_ECHO(" Z1_value:");
         SERIAL_ECHO(anker_align.z1_value);
         SERIAL_ECHO(" Z2_value:");
         SERIAL_ECHO(anker_align.z2_value);
         SERIAL_ECHO("\r\n");
         break;
       }
       else if(z1>z2)
       {
         anker_align.add_z2_value(rise_z);
       }
       else if(z2>z1)
       {
         anker_align.add_z1_value(rise_z);
       }
    
         anker_align.xy[0].x=PROBING_MARGIN;
         anker_align.xy[0].y+=1.5;
         anker_align.xy[1].x=X_BED_SIZE-PROBING_MARGIN;
         anker_align.xy[1].y+=1.5;

         if(num==(ANLIGN_NUM-1))
         {
           if(ABS(z1-z2)>ANLIGN_ALLOWED)
           {
               SERIAL_ECHO("echo:Please check the Z-axis limit!\r\n");
               anker_align.reset();
           }
         }

         if(anker_align.g36_running_flag == false)
         {
            SERIAL_ECHO("echo:anker_align stop!\r\n");
            break;
         }

     }
     anker_probe_set.delay=LEVEING_PROBE_DELAY;
     gcode.process_subcommands_now_P(PSTR("G2001\n"));
     anker_align.g36_running_flag = false;
     TERN_(USE_Z_SENSORLESS, anker_homing.is_again_probe_homing = false);
     TERN_(Z_MULTI_ENDSTOPS, endstops.z2_endstop_adj = z2_endstop_adj);

     TERN_(ANKER_MAKE_API, anker_homing.after_align_action(targetTemperature));
     TERN_(ANKER_MAKE_API, anker_homing.filament_type = FILAMENT_PLA); // reset filament to PLA

    }


#endif






