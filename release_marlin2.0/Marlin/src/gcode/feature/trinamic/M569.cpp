/**
 * Marlin 3D Printer Firmware
 * Copyright (c) 2020 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
 *
 * Based on Sprinter and grbl.
 * Copyright (c) 2011 Camiel Gubbels / Erik van der Zalm
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "../../../inc/MarlinConfig.h"

#if HAS_STEALTHCHOP

#include "../../gcode.h"
#include "../../../feature/tmc_util.h"
#include "../../../module/stepper/indirection.h"

template<typename TMC>
void tmc_say_stealth_status(TMC &st) {
  st.printLabel();
  SERIAL_ECHOPGM(" driver mode:\t");
  SERIAL_ECHOLNPGM_P(st.get_stealthChop() ? PSTR("stealthChop") : PSTR("spreadCycle"));
}
template<typename TMC>
void tmc_set_stealthChop(TMC &st, const bool enable) {
  st.stored.stealthChop_enabled = enable;
  st.refresh_stepping_mode();
}

static void tmc_set_toff(const uint8_t index, const uint8_t val) {
  #define set_toff(Q) do{stepper##Q.printLabel();SERIAL_ECHOPGM(" driver toff:"); stepper##Q.toff(val); SERIAL_ECHO(stepper##Q.toff());}while(0)
  switch(index)
  {
      case X_AXIS: set_toff(X); break;
      case Y_AXIS: set_toff(Y); break;
      case Z_AXIS: set_toff(Z); break;
      case E_AXIS: set_toff(E0); break;
      default:SERIAL_ECHOPGM("NONE");break;
  }
  SERIAL_EOL();
}
static void tmc_set_hstrt(const uint8_t index, const uint8_t val) {
  #define set_hstrt(Q) do{stepper##Q.printLabel();SERIAL_ECHOPGM(" driver hstrt:"); stepper##Q.hstrt(val-1);SERIAL_ECHO(stepper##Q.hstrt());}while(0)
  switch(index)
  {
      case X_AXIS:set_hstrt(X); break;
      case Y_AXIS:set_hstrt(Y); break;
      case Z_AXIS:set_hstrt(Z); break;
      case E_AXIS:set_hstrt(E0); break;
      default:SERIAL_ECHOPGM("NONE");break;
  }
  SERIAL_EOL();
}
static void tmc_set_hend(const uint8_t index, const int8_t val) {
  #define set_hend(Q) do{stepper##Q.printLabel();SERIAL_ECHOPGM(" driver hend:"); stepper##Q.hend(val+3);SERIAL_ECHO(stepper##Q.hend());}while(0)
  switch(index)
  {
      case X_AXIS:set_hend(X); break;
      case Y_AXIS:set_hend(Y); break;
      case Z_AXIS:set_hend(Z); break;
      case E_AXIS:set_hend(E0); break;
      default:SERIAL_ECHOPGM("NONE");break;
  }
  SERIAL_EOL();
}

static void set_stealth_status(const bool enable, const int8_t target_e_stepper) {
  #define TMC_SET_STEALTH(Q) tmc_set_stealthChop(stepper##Q, enable)

  #if    X_HAS_STEALTHCHOP  || Y_HAS_STEALTHCHOP  || Z_HAS_STEALTHCHOP \
      || I_HAS_STEALTHCHOP  || J_HAS_STEALTHCHOP  || K_HAS_STEALTHCHOP \
      || X2_HAS_STEALTHCHOP || Y2_HAS_STEALTHCHOP || Z2_HAS_STEALTHCHOP || Z3_HAS_STEALTHCHOP || Z4_HAS_STEALTHCHOP
    const uint8_t index = parser.byteval('I');
  #endif

  LOOP_LOGICAL_AXES(i) if (parser.seen(axis_codes[i])) {
    switch (i) {
      case X_AXIS:
        TERN_(X_HAS_STEALTHCHOP,  if (index == 0) TMC_SET_STEALTH(X));
        TERN_(X2_HAS_STEALTHCHOP, if (index == 1) TMC_SET_STEALTH(X2));
        break;

      #if HAS_Y_AXIS
        case Y_AXIS:
          TERN_(Y_HAS_STEALTHCHOP,  if (index == 0) TMC_SET_STEALTH(Y));
          TERN_(Y2_HAS_STEALTHCHOP, if (index == 1) TMC_SET_STEALTH(Y2));
          break;
      #endif

      #if HAS_Z_AXIS
        case Z_AXIS:
          TERN_(Z_HAS_STEALTHCHOP,  if (index == 0) TMC_SET_STEALTH(Z));
          TERN_(Z2_HAS_STEALTHCHOP, if (index == 1) TMC_SET_STEALTH(Z2));
          TERN_(Z3_HAS_STEALTHCHOP, if (index == 2) TMC_SET_STEALTH(Z3));
          TERN_(Z4_HAS_STEALTHCHOP, if (index == 3) TMC_SET_STEALTH(Z4));
          break;
      #endif

      #if I_HAS_STEALTHCHOP
        case I_AXIS: TMC_SET_STEALTH(I); break;
      #endif
      #if J_HAS_STEALTHCHOP
        case J_AXIS: TMC_SET_STEALTH(J); break;
      #endif
      #if K_HAS_STEALTHCHOP
        case K_AXIS: TMC_SET_STEALTH(K); break;
      #endif

      #if E_STEPPERS
        case E_AXIS: {
          if (target_e_stepper < 0) return;
          switch (target_e_stepper) {
            TERN_(E0_HAS_STEALTHCHOP, case 0: TMC_SET_STEALTH(E0); break;)
            TERN_(E1_HAS_STEALTHCHOP, case 1: TMC_SET_STEALTH(E1); break;)
            TERN_(E2_HAS_STEALTHCHOP, case 2: TMC_SET_STEALTH(E2); break;)
            TERN_(E3_HAS_STEALTHCHOP, case 3: TMC_SET_STEALTH(E3); break;)
            TERN_(E4_HAS_STEALTHCHOP, case 4: TMC_SET_STEALTH(E4); break;)
            TERN_(E5_HAS_STEALTHCHOP, case 5: TMC_SET_STEALTH(E5); break;)
            TERN_(E6_HAS_STEALTHCHOP, case 6: TMC_SET_STEALTH(E6); break;)
            TERN_(E7_HAS_STEALTHCHOP, case 7: TMC_SET_STEALTH(E7); break;)
          }
        } break;
      #endif
    }
  }
}

static void say_stealth_status() {
  #define TMC_SAY_STEALTH_STATUS(Q) tmc_say_stealth_status(stepper##Q)
  OPTCODE( X_HAS_STEALTHCHOP, TMC_SAY_STEALTH_STATUS(X))
  OPTCODE(X2_HAS_STEALTHCHOP, TMC_SAY_STEALTH_STATUS(X2))
  OPTCODE( Y_HAS_STEALTHCHOP, TMC_SAY_STEALTH_STATUS(Y))
  OPTCODE(Y2_HAS_STEALTHCHOP, TMC_SAY_STEALTH_STATUS(Y2))
  OPTCODE( Z_HAS_STEALTHCHOP, TMC_SAY_STEALTH_STATUS(Z))
  OPTCODE(Z2_HAS_STEALTHCHOP, TMC_SAY_STEALTH_STATUS(Z2))
  OPTCODE(Z3_HAS_STEALTHCHOP, TMC_SAY_STEALTH_STATUS(Z3))
  OPTCODE(Z4_HAS_STEALTHCHOP, TMC_SAY_STEALTH_STATUS(Z4))
  OPTCODE( I_HAS_STEALTHCHOP, TMC_SAY_STEALTH_STATUS(I))
  OPTCODE( J_HAS_STEALTHCHOP, TMC_SAY_STEALTH_STATUS(J))
  OPTCODE( K_HAS_STEALTHCHOP, TMC_SAY_STEALTH_STATUS(K))
  OPTCODE(E0_HAS_STEALTHCHOP, TMC_SAY_STEALTH_STATUS(E0))
  OPTCODE(E1_HAS_STEALTHCHOP, TMC_SAY_STEALTH_STATUS(E1))
  OPTCODE(E2_HAS_STEALTHCHOP, TMC_SAY_STEALTH_STATUS(E2))
  OPTCODE(E3_HAS_STEALTHCHOP, TMC_SAY_STEALTH_STATUS(E3))
  OPTCODE(E4_HAS_STEALTHCHOP, TMC_SAY_STEALTH_STATUS(E4))
  OPTCODE(E5_HAS_STEALTHCHOP, TMC_SAY_STEALTH_STATUS(E5))
  OPTCODE(E6_HAS_STEALTHCHOP, TMC_SAY_STEALTH_STATUS(E6))
  OPTCODE(E7_HAS_STEALTHCHOP, TMC_SAY_STEALTH_STATUS(E7))
}

/**
 * M569: Enable stealthChop on an axis
 *   P<?_axis>  { <off_time[1..15]>, <hysteresis_end[-3..12]>, hysteresis_start[1..8] }
 *    O[<off_time[1..15]>] 
 *    D[<hysteresis_end[-3..12]>]
 *    H[hysteresis_start[1..8]]
 *   S[1|0] to enable or disable
 *   XYZE to target an axis
 *   No arguments reports the stealthChop status of all capable drivers.
 */
void GcodeSuite::M569() {
  if (parser.seen('P')){ // select 
    uint8_t index = parser.value_byte();
    if (parser.seen('O')){ tmc_set_toff(index, parser.value_byte());}
    if (parser.seen('D')){ tmc_set_hend(index, parser.value_int());}
    if (parser.seen('H')){ tmc_set_hstrt(index, parser.value_byte());}
  }
  else if (parser.seen('S'))
    set_stealth_status(parser.value_bool(), get_target_e_stepper_from_command());
  else
    say_stealth_status();
}

#endif // HAS_STEALTHCHOP
