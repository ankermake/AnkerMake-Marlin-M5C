/**
 * Marlin 3D Printer Firmware
 * Copyright (c) 2021 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
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
#pragma once

/*******************************Version number management****************************/
// Motherboard Version Management
#define MAIN_BOARD_V8110_V0_2 "V8110_MAIN_V0.2"
#define MAIN_BOARD_V8110_V0_3 "V8110_MAIN_V0.3"

#define MAIN_BOARD_VERSION MAIN_BOARD_V8110_V0_2

// Motherboard chip management
#define STM32F407VGT6_CHIP "STM32F407VGT6"
#define GD32F407VGT6_CHIP  "GD32F407VGT6"

#define MAIN_BOARD_CHIP GD32F407VGT6_CHIP

/*******************************Printer Profile Management****************************/
// V8110
#define MOTOR_V8110_5X                3001
#define MOTOR_V8110_5X_EVT            3006
#define MOTOR_V8110_5X_DVT            3008

// Profile selection
#define MACCHINE MOTOR_V8110_5X_DVT

/*******************************Functional API Configuration****************************/
#if MACCHINE == MOTOR_V8110_5X_DVT
#ifndef SHORT_BUILD_VERSION
#define SHORT_BUILD_VERSION "V8110_V3.0.21"
#endif
#define ANKER_ROTATION        1 //
#define ANKER_LOG_DEBUG       0 // Used to print logs for debugging
#define ANKER_MAKE_API        1 // gcode
#define ANKER_TMC_SET         1 // For TMC driver settings
#define ANKER_FIX_ENDSTOPR    0 // Used to resolve limit switch conflicts on motherboards
#define ANKER_ANLIGN          1 // Z axis automatic alignment
#define ANKER_LEVEING_FADE    1
#define ACCELERATION_CONTROL  1 // ACCELERATION control
#define REPORT_LEVEL_PORT     1
#define PHOTO_Z_LAYER         1 // Photo function for each layer
#define ANKER_PAUSE_FUNC      1 // Anker pause function enable/disable
#define ANKER_MULTIORDER_PACK 1 // anekr multi order in one packet in once communication
#define GD32F427VE_SUPPORT    0
#define ANKER_Z_OFFSET_FUNC   0 // anker z offset function enable/disable
#define ANKER_BELT_CHECK      0 // for belt inspection
#define ANKER_PROBE_SET       1
#define ADAPT_DETACHED_NOZZLE 1 // adapt detached nozzle board(GD32E230)
#define ANKER_TEMP_WATCH      1
#define USE_Z_SENSORLESS      1 //
#define EVT_HOMING_5X         1
#define WS1_HOMING_5X         1
#define ANKER_LEVEING         1
#define ANKER_NOZZLE_PROBE_OFFSET 1 //
#define NO_CHECK_Z_HOMING         1 // Does not detect whether Z is zeroed
#define ANKER_M_CMDBUF            1 //
#define ANKER_PROBE_DETECT_TIMES  1 // Probe multiple times at the same point
#define ANKER_SIMPLE_HOMING       1 // Probe multiple times at the same point
#define ANKER_E_SMOOTH            1
#define ANKER_OVERPRESSURE_REPORT 0 // if overpressure is detected, the down-probing function should be stopped and an error should be reported.
#define ANKER_MOTION_TRACKING     1 // Motion tracking
#define ANKER_RETRACTION_E_JERK   1 // retraction_e_jerk. Independent setting of starting speeds for E-axis retraction and feed-in
#define ANKER_VIBRATION_CONTROL   1 // T/S curve switching and Zero configuration(Zeroconf)
#define ANKER_CORNER_CALC         1 // Corner angle calculation
#define ANKER_STARTUP_SPEED_ERR   0 // Excessive startup speed error
#define ANKER_FILTER_LEVEL_GRID   0 //filter_leveling_grid
#endif

/*******************************Error detection****************************/
#if HANDSHAKE
#if PROVE_CONTROL == 0
#error "HANDSHAKE needs to be enabled PROVE_CONTROL"
#endif
#if HEATER_EN_CONTROL == 0
#error "HANDSHAKE needs to be enabled HEATER_EN_CONTROL"
#endif
#endif
