#pragma once

#include "../../inc/ANKER_Config.h"

#if ADAPT_DETACHED_NOZZLE

#include "uart_nozzle_tx.h"
#include <string.h>

extern "C"
{
    typedef struct
    {
        uint8_t hw_version[3];
        uint8_t sw_version[3];

        uint8_t material_sta;
    } nozzle_t;

    extern nozzle_t nozzle;

    void latch_clear(void);

    void uart_nozzle_init(void);

    void uart_nozzle_polling(void);

    void uart_nozzle_tx_probe_val(uint16_t val);
}

#endif
