/*
 * @Author       : harley
 * @Date         : 2022-04-31 20:35:23
 * @LastEditors  : harley
 * @LastEditTime : 
 * @Description  :
 */
#include "anker_belt_check.h"

#if ENABLED(ANKER_BELT_CHECK)
#include "../../module/planner.h"
#include "../tmc_util.h"

  Anker_Belt_Check anker_belt_check;

   void Anker_Belt_Check:: run(void)
   {
     gcode.process_subcommands_now_P(PSTR("G28"));
     gcode.process_subcommands_now_P(PSTR("G1 Z20 F600"));
     gcode.process_subcommands_now_P(PSTR("G1 X5 Y5 F12000"));      
     planner.synchronize();
     gcode.process_subcommands_now_P(PSTR("G1 X215 F3000"));   
     SERIAL_ECHO("X100mm/s:\r\n");
     planner.anker_belt_check_synchronize(1);
     gcode.process_subcommands_now_P(PSTR("G1 X5 F6000"));   
     SERIAL_ECHO("X200mm/s:\r\n"); 
     planner.anker_belt_check_synchronize(1);   
     gcode.process_subcommands_now_P(PSTR("G1 Y215 F3000"));  
     SERIAL_ECHO("Y100mm/s:\r\n"); 
     planner.anker_belt_check_synchronize(2);
     gcode.process_subcommands_now_P(PSTR("G1 Y5 F6000"));    
     SERIAL_ECHO("Y200mm/s:\r\n"); 
     planner.anker_belt_check_synchronize(2);     
   }
#endif