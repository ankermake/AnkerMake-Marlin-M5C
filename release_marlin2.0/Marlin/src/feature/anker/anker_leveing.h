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

    #if ENABLED(ANKER_ROTATION)

            #define ROTATION_VALUE 0.12
            class Anker_Rotation {
            public:
                  float rotation_value=ROTATION_VALUE;
                  void convert_rotation_value(void);//G29
            };
            extern  Anker_Rotation anker_rotation;
    #endif
     
    #if  ENABLED(ANKER_LEVEING)

           class Anker_Leveing {
            public:
                static xy_pos_t xy[GRID_MAX_POINTS_X];
                void test(uint8_t select);
           };
           extern  Anker_Leveing anker_leveing;
    #endif

#endif
