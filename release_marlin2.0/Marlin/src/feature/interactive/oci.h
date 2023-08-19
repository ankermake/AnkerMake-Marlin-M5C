#include "../../inc/ANKER_Config.h"

#if ADAPT_DETACHED_NOZZLE

#pragma once

extern "C"
{
#include <stdint.h>

    void oci_init(void);

    extern uint8_t fatal_err;
}

#endif
