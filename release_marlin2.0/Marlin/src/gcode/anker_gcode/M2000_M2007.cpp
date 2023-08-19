#include "../gcode.h"
#include "../../inc/MarlinConfig.h"
#include "../queue.h"           // for getting the command port
#include "../../module/planner.h"
#include "../../feature/anker/anker_homing.h"
#include "../../feature/anker/anker_z_sensorless.h"
#include "../../feature/anker/board_configure.h"
#include "../../module/motion.h"
#include "../../module/stepper.h"
#include "../../module/endstops.h"

#if ENABLED(PROVE_CONTROL)
    
void GcodeSuite::M2000() {
    if (parser.seen('S'))
    {
       if(parser.value_bool())
       {
        digitalWrite(PROVE_CONTROL_PIN,PROVE_CONTROL_STATE);
        SERIAL_ECHOLNPGM("PROVE_CONTROL_PIN:OPEN!"); 
       }
       else
       {
        digitalWrite(PROVE_CONTROL_PIN,!PROVE_CONTROL_STATE);
        SERIAL_ECHOLNPGM("PROVE_CONTROL_PIN:CLOSE!"); 
       }
    }
    else
    {
       if(READ(PROVE_CONTROL_PIN)==PROVE_CONTROL_STATE)
       {
         SERIAL_ECHOLNPGM("PROVE_CONTROL is OPEN!");  
       }
       else 
       {
        SERIAL_ECHOLNPGM("PROVE_CONTROL is CLOSE!");  
       }
    }
}
#endif


#if ENABLED(READ_BUFFER_SIZE)
   void GcodeSuite::M2001()
   {
     SERIAL_ECHO("BLOCK_BUFFER_SIZE: ");
     SERIAL_ECHO(BLOCK_BUFFER_SIZE);
     SERIAL_ECHO("\r\nMAX_CMD_SIZE: ");
     SERIAL_ECHO(MAX_CMD_SIZE);
     SERIAL_ECHO("\r\nBUFSIZE: ");
     SERIAL_ECHO(BUFSIZE);
     SERIAL_ECHO("\r\nSLOWDOWN_DIVISOR: ");
     SERIAL_ECHO(SLOWDOWN_DIVISOR);
     #ifdef XY_FREQUENCY_LIMIT
      SERIAL_ECHO("\r\nXY_FREQUENCY_LIMIT: ");
      SERIAL_ECHO(planner.xy_freq_limit_hz);
      SERIAL_ECHO("\r\nXY_FREQUENCY_MIN_PERCENT: ");
      SERIAL_ECHO(planner.xy_freq_min_speed_factor);
     #endif
     SERIAL_ECHO("\r\n");  
     SERIAL_ECHO("\r\n");     
   }
#endif

#if ENABLED(BOARD_CONFIGURE)
   void GcodeSuite::M2002()
   {
      SERIAL_ECHO("Board chek \r\n");
      SERIAL_ECHO("adc1: ");
      SERIAL_ECHO(board_configure.adc1);
      SERIAL_ECHO("\r\n");
      SERIAL_ECHO("adc2: ");
      SERIAL_ECHO(board_configure.adc2);
      SERIAL_ECHO("\r\n");
   }
#endif

#if ENABLED(USE_Z_SENSORLESS)

   void GcodeSuite::M2003()
   {
      if (parser.seen('Z'))
      {
         int16_t value = parser.value_int();
         use_z_sensorless.set_z1_value(value);
         #ifdef ANKER_Z2_STALL_SENSITIVITY
           use_z_sensorless.set_z2_value(value);
         #endif
      }
      if (parser.seen('A'))
      {
          int16_t value = parser.value_int();
          use_z_sensorless.an_stall_value=value;
      }
      if (parser.seen('B'))
      {
         int16_t value = parser.value_int();
         use_z_sensorless.set_z1_value(value);
      }
      if (parser.seen('C'))
      {
         int16_t value = parser.value_int();
         #ifdef ANKER_Z2_STALL_SENSITIVITY
           use_z_sensorless.set_z2_value(value);
         #endif
      }
      #if ENABLED(ANKER_TMC_SET)
      if (parser.seen('T'))
      {
          uint32_t value = parser.value_ulong();
          anker_tmc_set.set_tmc_z1_tcoolthrs(value);
          anker_tmc_set.set_tmc_z2_tcoolthrs(value);
      }
      #endif

        use_z_sensorless.report();
        TERN_(ANKER_TMC_SET, anker_tmc_set.report_tmc_tcoolthrs());
   }
#endif


#if ENABLED(EVT_HOMING_5X)
   void GcodeSuite::M2004()
   {
   #if ENABLED(NO_MOTION_BEFORE_HOMING)
      if (parser.seen('S'))
     {
       if(parser.value_bool())
       {
        anker_homing.no_check_all_axis=true;
        SERIAL_ECHOLNPGM("no_check_all_axis:OPEN!"); 
       }
       else
       {
        anker_homing.no_check_all_axis=false;  
        SERIAL_ECHOLNPGM("no_check_all_axis:CLOSE!"); 
       }
    }
    else
    {
       if(anker_homing.no_check_all_axis)
       {
        SERIAL_ECHOLNPGM("no_check_all_axis:OPEN!"); 
       }
       else 
       {
        SERIAL_ECHOLNPGM("no_check_all_axis:CLOSE!"); 
       }
    }
   #endif
   }  
#endif

#if ENABLED(USE_Z_SENSORLESS)
      void GcodeSuite::M2005()
      {
      if(anker_homing.get_first_end_z_axis()==Z_AXIS_IS_Z1)
       {
        SERIAL_ECHOLNPGM("first_end_z_axis:Z1!"); 
       }
       else if(anker_homing.get_first_end_z_axis()==Z_AXIS_IS_Z2)
       {
        SERIAL_ECHOLNPGM("first_end_z_axis:Z2!"); 
       }
       else if(anker_homing.get_first_end_z_axis()==Z_AXIS_IDLE)
       {
        SERIAL_ECHOLNPGM("first_end_z_axis:IDLE!");           
       }
      }

      void GcodeSuite::M2006()//Auto-Proof Threshold
      {
         use_z_sensorless.set_z1_value(80);//Set an insensitive threshold so that it must return to zero
         use_z_sensorless.set_z2_value(80);

         gcode.process_subcommands_now_P("G28 Z\n");
         
      }
#endif

#if ENABLED(TMC_AUTO_CONFIG)
   void GcodeSuite::M2007()
   {
       stepperX.rms_current(800, 1);
       stepperY.rms_current(800, 1);
       stepperX.set_pwm_thrs(301);
       stepperY.set_pwm_thrs(301);
        TMC2208_n::PWMCONF_t pwmconf{0};
        pwmconf.pwm_lim = 12;
        pwmconf.pwm_reg = 8;
        pwmconf.pwm_autograd = true;
        pwmconf.pwm_autoscale = true;
        pwmconf.pwm_freq = 0b01;
        pwmconf.pwm_grad = 14;
        pwmconf.pwm_ofs = 36;
        stepperX.PWMCONF(pwmconf.sr);
        ENABLE_AXIS_X();
        ENABLE_AXIS_Y();
        _delay_ms(200);

        SERIAL_ECHO("PER-XPWM_SCALE_AUTO: ");
        SERIAL_ECHO(stepperX.pwm_scale_auto());
        SERIAL_ECHO("\r\n");
        SERIAL_ECHO("PER-YPWM_SCALE_AUTO: ");
        SERIAL_ECHO(stepperY.pwm_scale_auto());
        SERIAL_ECHO("\r\n");
        gcode.process_subcommands_now_P("M204 P100 R100 T100\n");
        gcode.process_subcommands_now_P("G1 X10 F60\n");
        gcode.process_subcommands_now_P("G1 X20 F7500\n");
        SERIAL_ECHO("XPWM_SCALE_AUTO: ");
        SERIAL_ECHO(stepperX.pwm_scale_auto());
        SERIAL_ECHO("\r\n");
        SERIAL_ECHO("YPWM_SCALE_AUTO: ");
        SERIAL_ECHO(stepperY.pwm_scale_auto());
        SERIAL_ECHO("\r\n");
   }
#endif

#if ENABLED(ANKER_EXTRUDERS_RECEIVE)
    void GcodeSuite::M2008()
         {

           if (parser.seen('S'))
            {
               uint8_t sta= parser.value_byte();
               if(sta==0)
               {
                 gcode.process_subcommands_now_P(PSTR("G1 Y0 F240\n"));
               }
               else if(sta==1)
               {
                 gcode.process_subcommands_now_P(PSTR("G1 Y17 F240\n"));
               }
            }

         }       
#endif


#if ENABLED(ANKER_EXTRUDERS_SEND)
    void GcodeSuite::M2008()
         {
            millis_t serial_connect_timeout = millis() + 1000UL;
            MYSERIAL3.begin(BAUDRATE_3);
            serial_connect_timeout = millis() + 1000UL;
            while (!MYSERIAL3.connected() && PENDING(millis(), serial_connect_timeout)) { /*nada*/ }
            _delay_ms(100);
            MYSERIAL3.printf("M2008 S1\r\n");
            _delay_ms(100);
            MYSERIAL3.end();

            MYSERIAL1.printf("M2008 S1\r\n");
         }  
#endif

#if ENABLED(AUTO_BED_LEVELING_BILINEAR)&&ENABLED(ANKER_ROTATION)

#include "../../feature/bedlevel/bedlevel.h"
#include "../../feature/anker/anker_leveing.h"

    void GcodeSuite::M2009()
         {
            int8_t i,j;
            if (parser.seen('S'))
               {
                 anker_rotation.rotation_value=parser.value_float();
               }
             SERIAL_ECHO("rotation_value:");
             SERIAL_ECHO(anker_rotation.rotation_value);
             SERIAL_ECHO("\r\n");
             SERIAL_ECHO("z_values:\r\n");
             for(j=0;j<GRID_MAX_POINTS_Y;j++)
             {
               for(i=0;i<GRID_MAX_POINTS_X;i++)
               {
                  SERIAL_ECHO(z_values[i][j]);
                  SERIAL_ECHO(" ");
               }
               SERIAL_ECHO("\r\n");
             }
             anker_rotation.convert_rotation_value();
         }

#endif

#if ENABLED(ANKER_Z_OFFSET_FUNC)
  #include "../../feature/anker/anker_z_offset.h"
      void GcodeSuite::M2011()
         {
            bool run_flag=true;
            if (parser.seen('S'))
               {
                 if(parser.value_bool())
                 {
                   anker_z_offset.cs1237_enable();
                 }
                 else
                 {
                   anker_z_offset.cs1237_disable();
                 }
                 run_flag=false;
               }

            if (parser.seen('V'))
               {
                  anker_z_offset.cs1237_threshold_set(parser.value_int());
                  run_flag=false;
               }

           if (parser.seen('L'))
               {
                 if(parser.value_bool())
                 {
                   anker_z_offset.log=true;
                   SERIAL_ECHO(" anker_z_offset.log=true:\r\n");
                 }
                 else
                 {
                   anker_z_offset.log=false;
                   SERIAL_ECHO(" anker_z_offset.log=false:\r\n");
                 }
                 run_flag=false;
               }

           if (parser.seen('R'))
               {
                 if(parser.value_bool())
                 {
                   anker_z_offset.loop_read_flag=true;
                   SERIAL_ECHO(" anker_z_offset.loop_read_flag=true:\r\n");
                 }
                 else
                 {
                   anker_z_offset.loop_read_flag=false;
                   SERIAL_ECHO(" anker_z_offset.loop_read_flag=false:\r\n");
                 }
                 run_flag=false;
               }

               MYSERIAL1.printf("threshold: %d\r\n", anker_z_offset.cs1237_threshold_get());
              
               if(run_flag)
               {
               anker_z_offset.run();
               //anker_z_offset.update_offset();
               }

         }
#endif

#if ENABLED(ANKER_PROBE_SET)
 #include "../../feature/anker/anker_z_offset.h"
 #include "../../module/settings.h"
 #include "../../feature/interactive/uart_nozzle_tx.h"
 #include "../../feature/interactive/protocol.h"

      void GcodeSuite::M2012()
         {
            bool report=true;
            if (parser.seen('S'))
            {
              uint16_t value=parser.value_int();
              anker_probe_set.probe_start(value);
              SERIAL_ECHO("M2012 S");  
              SERIAL_ECHO(value);  
              SERIAL_ECHO("\r\n");  
              report=false;       
            }
            else if (parser.seen('H'))
            {
               anker_probe_set.homing_value=parser.value_int();
            }
            else if (parser.seen('L'))
            {
               anker_probe_set.leveing_value=parser.value_int();
            }
  
            if(report)
            {
              anker_probe_set.report_value();
            }
         }
      void GcodeSuite::M2013()
         {
            anker_probe_set.get_probe_value();
            SERIAL_ECHO("M2013\r\n");  
         }
      void GcodeSuite::M2014()
         {
            if (parser.seen('S'))
            {
               anker_probe_set.show_adc_value(parser.value_bool());  
               SERIAL_ECHO("M2014 \r\n");  
            }
         }

      void GcodeSuite::M3001()
         {
           uart_nozzle_tx_multi_data(GCP_CMD_45_M3001_DEAL, 0, 0);
           MYSERIAL1.printf("echo:M3001 R\n");
         }   

      void GcodeSuite::M3002()
         {
           uart_nozzle_tx_multi_data(GCP_CMD_46_M3002_DEAL, 0, 0);
           MYSERIAL1.printf("echo:M3002 R\n");
         }     

         void GcodeSuite::M3003()
         {
            uart_nozzle_tx_probe_leveling_val_get();
            // if (parser.seen('V'))
            // {
            //    const uint16_t leveing_value = parser.value_int();
            //    // uart_nozzle_tx_probe_leveling_val_set(leveing_value);
            //    MYSERIAL2.printLine("echo:M3003 V%d\r\n", leveing_value);
            //    // settings.save();
            //    return;
            // }

         }

      void GcodeSuite::M2201()
         {
            bool is_pid_autotune=true;
            uint16_t temp,ncyclesc ;

            if (parser.seen('S'))
            {
               temp=parser.value_ushort();
            }
            else{
               is_pid_autotune=false;
            }  

            if (parser.seen('C'))
            {
               ncyclesc=parser.value_ushort();
            }  
            else{
               is_pid_autotune=false;
            }        
            if(is_pid_autotune) 
            anker_probe_set.pid_autotune(temp,ncyclesc);
         }
      void GcodeSuite::M2202()
         {
            bool is_run=true;
            if (parser.seen('S'))
            {
               if(parser.value_bool()==0)
               {
                 anker_probe_set.run_step=0;
               }  
               is_run=false;
            }

             if(is_run)
             {
              anker_probe_set.run();
             }
         }
      void GcodeSuite::M2203()
         {
           anker_probe_set.auto_run();
         }
      void GcodeSuite::M2204()
         {
            if (parser.seen('S'))
            {
              uint16_t value=parser.value_int();
              anker_probe_set.point_test(value);
            }
            else  if (parser.seen('P'))
            {
              uint16_t value=parser.value_int();
              if(value==1)
              {
               anker_probe_set.point_test_ready();
              }
              else  if(value==0)
              {
               anker_probe_set.point_test_flag=false;
              }
            }        
         }
#endif

      #if ENABLED(ANKER_LEVEING)
       void GcodeSuite::M2300()
         {
            uint8_t select=0;
            if (parser.seen('S'))
            {
              select=parser.value_int();
              anker_leveing.test(select);
            }
         }        
      #endif

#if ENABLED(ANKER_BELT_CHECK)
     #include "../../feature/anker/anker_belt_check.h"
         void GcodeSuite::M2100()
         {
            anker_belt_check.run();
         }
#endif






     

