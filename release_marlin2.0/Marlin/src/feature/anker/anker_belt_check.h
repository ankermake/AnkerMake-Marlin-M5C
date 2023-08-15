/*
 * @Author       : harley
 * @Date         : 2022-04-31 20:35:23
 * @LastEditors  : harley
 * @LastEditTime : 
 * @Description  :
 */
#include "../../inc/MarlinConfig.h"


#if ENABLED(ANKER_BELT_CHECK)
   #include "../../gcode/gcode.h"



    class Anker_Belt_Check
    {
    public:
     void run(void);
    };

    extern Anker_Belt_Check anker_belt_check;
#endif