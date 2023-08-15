#include "../../inc/ANKER_Config.h"

#if ADAPT_DETACHED_NOZZLE

#include "uart_nozzle_tx.h"

#include "../../inc/MarlinConfig.h"
#include "../../module/temperature.h"
#include "clock.h"

extern "C"
{

#include "uart_nozzle_rx.h"

#include <stdint.h>

#include "gcp.h"
#include "gcp_parser.h"
#include "oci.h"
#include "protocol.h"
#include "soft_timer.h"

#define OCI_IN_PIN            PB13
#define OCI_CLR_PIN           PD10
#define OCI_NMOS_CHK_PIN      PD11
#define OCI_HEATER_HOTBED_PIN HEATER_BED_PIN
#define OCI_HOTBED_PWR_PIN    EN_24V_HEAT

    typedef struct
    {
        soft_timer_t mos_soft_timer;
    } oci_t;

    oci_t oci;
    uint8_t fatal_err;

#define OCI_FEED_READ() READ(OCI_NMOS_CHK_PIN)

    static void oci_latch_clear(void)
    {
        OUT_WRITE(OCI_CLR_PIN, LOW);
        _delay_ms(1);
        OUT_WRITE(OCI_CLR_PIN, HIGH);
    }

    static void oci_delay(void)
    {
        volatile uint32_t count = 5000;
        while (count--)
            ;
    }

    static void oci_mos_check_callback(void)
    {
        uint8_t level;

        if (Temperature::temp_bed.target != 0)
            return;

        OUT_WRITE(OCI_CLR_PIN, LOW);

        DISABLE_TEMPERATURE_INTERRUPT();
        level = READ_OUT(OCI_HEATER_HOTBED_PIN);
        OUT_WRITE(OCI_HEATER_HOTBED_PIN, HIGH);
        oci_delay();
        if (OCI_FEED_READ() == 1)
        {
            safe_delay(5);
            if (OCI_FEED_READ() == 1)
            {
                OUT_WRITE(OCI_HOTBED_PWR_PIN, LOW);
                fatal_err = 1;
                MYSERIAL2.printLine("Error:Demage:1 bed\n");
            }
        }

        OUT_WRITE(OCI_HEATER_HOTBED_PIN, level);
        ENABLE_TEMPERATURE_INTERRUPT();

        OUT_WRITE(OCI_CLR_PIN, HIGH);
    }

    void oci_init(void)
    {
        oci_latch_clear();

        SET_INPUT_PULLUP(OCI_IN_PIN); // falling edge trigger

        soft_timer_init(&oci.mos_soft_timer, 5000, 0, oci_mos_check_callback);
        soft_timer_start(&oci.mos_soft_timer);
    }
}

#endif
