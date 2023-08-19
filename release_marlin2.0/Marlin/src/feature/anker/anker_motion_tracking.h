/*
 * @Author       : Anan
 * @Date         : 2023-06-09 09:25:24
 * @LastEditors  : Anan
 * @LastEditTime : 2023-06-17 16:28:00
 * @Description  : Dynamically tracking the motion planning situation
 */
#pragma once

#include "../../inc/MarlinConfig.h"
#if ENABLED(ANKER_MOTION_TRACKING)

class Motion_Track {
    public:
        //#define TEMP_MOTION_IRQ_PRIO   8
        static void msg_sending(void);
        static void init(void);
        static void contorl(uint8_t switch_flag);
        static void block_info_record(const void* ptr);
        static void mode(uint8_t mode);
    private:



};

extern Motion_Track motion_track;

#endif
