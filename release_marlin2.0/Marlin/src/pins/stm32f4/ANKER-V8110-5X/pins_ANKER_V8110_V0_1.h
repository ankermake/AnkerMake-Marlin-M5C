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

#include "../env_validate.h"

// USB Flash Drive support
#define HAS_OTG_USB_HOST_SUPPORT

// Used to detect the hardware version number and which chip is used by the hardware
#if ENABLED(BOARD_CONFIGURE)
#define BOARD_VERSION_GPIO GPIOA
#define BOARD_VERSION_PIN PIN0
#define BOARD_CHIP_GPIO GPIOA
#define BOARD_CHIP_PIN PIN1
#endif

// Use one of these or SDCard-based Emulation will be used
#if NO_EEPROM_SELECTED
//#define SRAM_EEPROM_EMULATION                 // Use BackSRAM-based EEPROM emulation
#define FLASH_EEPROM_EMULATION // Use Flash-based EEPROM emulation
#endif

#if ENABLED(FLASH_EEPROM_EMULATION)
// Decrease delays and flash wear by spreading writes across the
// 128 kB sector allocated for EEPROM emulation.
#define FLASH_EEPROM_LEVELING
#endif

// Avoid conflict with TIMER_TONE
#define STEP_TIMER 10

#if HOTENDS > 2 || E_STEPPERS > 2
#error "ANKER_V8110_V1_0 supports up to 2 hotends / E-steppers."
#endif

#ifdef NOZZLE_AS_PROBE
    #define PROVE_CONTROL_PIN PC5
    #define PROVE_CONTROL_STATE HIGH
    #define HOMING_RISE_SPEED 40 * 60 // mm/min
    // Z Probe (when not Z_MIN_PIN)
    #ifndef Z_MIN_PROBE_PIN
      #if MACCHINE == MOTOR_5X_DVT_USE_G_SENSOR
       #define Z_MIN_PROBE_PIN PD1
      #else
       #define Z_MIN_PROBE_PIN PB0
      #endif
    #endif
#else
#define HOMING_RISE_SPEED 10 * 60 // mm/min
// Z Probe (when not Z_MIN_PIN)
#ifndef Z_MIN_PROBE_PIN
#ifdef SENSORLESS_PROBING
    #define Z_MIN_PROBE_PIN PB10
#else
    #define Z_MIN_PROBE_PIN PB0
#endif
#endif
#endif

#if ENABLED(MOTOR_EN_CONTROL)
#define MOTOR_EN_PIN PA7
#define MOTOR_EN_STATE HIGH
#endif

#if ENABLED(HEATER_EN_CONTROL)
#define HEATER_EN_PIN PC2
#define HEATER_EN_STATE HIGH
#endif

#if ENABLED(ANKER_LEVEING)
#define ANKER_LEVEING_DELAY_BEFORE_PROBING_TRUE 1000
#define ANKER_LEVEING_DELAY_BEFORE_PROBING 5000
#endif

#if ENABLED(HANDSHAKE)
#define HANDSHAKE_SDO PC4
#define HANDSHAKE_SCK PB0
#define HANDSHAKE_STATE HIGH
#define HANDSHAKE_TIME 5000 // ms
#endif

#if ENABLED(HOMING_BACKOFF)
#define HOMING_5X_BACKOFF_MM \
  {                          \
    0, 20, 0                 \
  } // (mm) Backoff from endstops before  homing
#endif

#if ENABLED(WS1_HOMING_5X)
#define HOMING_DUAL_Z_RISZ 0  // (mm) Ascent distance between return to zero
#define HOMING_PROBE_Z_RISE 0 // (mm) The distance that the nozzle rises after moving to the middle point
#endif

#if ENABLED(EVT_HOMING_5X)
#define ANKER_Z_SAFE_HOMING_X_POINT 50  // X point for Z homing
#define ANKER_Z_SAFE_HOMING_Y_POINT 2.2 // Y point for Z homing
#define ANKER_Z_AFTER_HOMING 2          // (mm) Halfway up the Z
#define PROBE_HOMING_BUMP_MM 1.5        // (mm) Strain gauge retraction distance
#define ANKER_HOMING_SCRIPT "G92 E0\nG1 X200 Y2.2 E10 F3000\nG1 X117.5 Y1.6 E15 F3000\nG1 E10 F3600\nG92 E0\n"

// #define ANKER_Z_SAFE_HOMING_X_POINT 185  // X point for Z homing
// #define ANKER_Z_SAFE_HOMING_Y_POINT 237.8  // Y point for Z homing
// #define ANKER_Z_AFTER_HOMING        2     // (mm) Halfway up the Z
// #define PROBE_HOMING_BUMP_MM        1.5   // (mm) Strain gauge retraction distance
// #define ANKER_HOMING_SCRIPT  "G92 E0\nG1 X35 Y237.8 E10 F3000\nG1 X117.5 Y237 E15 F3000\nG1 E10 F3600\nG92 E0\n"

#endif

#if ENABLED(ANKER_ANLIGN)
#define ALIGN_PER_RESET
#define ANLIGN_RISE 1 // ascent
#define ANLIGN_NUM 6
#define ANLIGN_MAX_VALUE 1.5
#define ANLIGN_ALLOWED 0.05 // Error allowed
#define SCREW_DISTANCE 300  // mm
#endif

#if ENABLED(ANKER_ANLIGN_ONLY_Z)
#define ANLIGN_RISE 4 // ascent
#define ANLIGN_NUM 3
#define ANLIGN_ALLOWED 0.1 // Error allowed
#endif

#if ENABLED(ANKER_TMC_SET)
 #define TCOOLTHRS_X 500
 #define TCOOLTHRS_Y 520
 #define TCOOLTHRS_Z1 1000
 #define TCOOLTHRS_Z2 1000
#endif

#if ENABLED(ANKER_Z_OFFSET_FUNC)
#define CS1237_SCL_PIN PB8
#define CS1237_SDA_PIN PB9
#define CS1237_PWR_CTRL_PIN PE0
#endif

#ifdef SENSORLESS_HOMING
#define ANKER_SENSORLESS_HOMING_BACKOFF_MM \
  {                                        \
    5, 5, 0                                \
  } // (mm) Backoff from endstops before  homing
#endif

#if ENABLED(ANKER_PROBE_SET)
  #define HOMING_FIRST_PROBE_VALUE  300
  #define HOMING_SECOND_PROBE_VALUE 200
  #define LEVEING_FIRST_PROBE_VALUE  200
  #define LEVEING_SECOND_PROBE_VALUE 200
#endif

//
// Limit Switches
//
#ifdef SENSORLESS_HOMING
#define X_STOP_PIN PC15 // X-STOP
#define Y_STOP_PIN PE11 // Y-STOP
#else
#define X_STOP_PIN PC15 // X-STOP
#define Y_STOP_PIN PE11 // Y-STOP
#endif

#if ENABLED(Z_MULTI_ENDSTOPS)
//  #ifdef SENSORLESS_HOMING
//   #define Z_MIN_PIN                          PD14  // Z1-STOP
//   #define Z_MAX_PIN                          PB10  // Z2-STOP
//  #else
#define Z_MIN_PIN PB3 // PE1  // Z1-STOP
#define Z_MAX_PIN PE1 // PB3  // Z2-STOP
//#endif
#else
#define Z_STOP_PIN PB0 // Z1-STOP
//#define Z2_STOP_PIN                     PD14  // Z1-STOP
#endif

#define X_ENABLE_PIN PE4
#define X_STEP_PIN PE3
#define X_DIR_PIN PE6
// #ifndef X_CS_PIN
//   #define X_CS_PIN                         PD1
// #endif

#define Y_ENABLE_PIN X_ENABLE_PIN
#define Y_STEP_PIN PB2
#define Y_DIR_PIN PE8
// #ifndef Y_CS_PIN
//   #define Y_CS_PIN                         PD0
// #endif

#define Z_ENABLE_PIN X_ENABLE_PIN
#define Z_STEP_PIN PC9
#define Z_DIR_PIN PB7

#define Z2_ENABLE_PIN X_ENABLE_PIN
#define Z2_STEP_PIN PC8
#define Z2_DIR_PIN PB5

// #define Z_ENABLE_PIN                       X_ENABLE_PIN
// #define Z_STEP_PIN                         PC8
// #define Z_DIR_PIN                          PB5

// #define Z2_ENABLE_PIN                      X_ENABLE_PIN
// #define Z2_STEP_PIN                        PC9
// #define Z2_DIR_PIN                         PB7

#define E0_ENABLE_PIN X_ENABLE_PIN
#define E0_STEP_PIN PD15
#define E0_DIR_PIN PA15

//
// Temperature Sensors
//
#define TEMP_BED_PIN PA4 // TB
#define TEMP_0_PIN PC0   // TH0
//#define TEMP_BOARD_PIN                      PA0   // T BOARD
//
// Heaters / Fans
//
#ifndef HEATER_0_PIN
#define HEATER_0_PIN PE5 // Heater0
#endif

#ifndef HEATER_BED_PIN
#define HEATER_BED_PIN PB15 // Hotbed
#endif

//#define HEATER_0_INVERTING true
#define HEATER_BED_INVERTING true

#ifndef FAN_PIN
#define FAN_PIN PC6 // Fan0
#endif
#ifndef FAN1_PIN
#define FAN1_PIN PC7 // Fan1
#endif
// #ifndef FAN2_PIN
//   #define FAN2_PIN                       PB13  // Fan2
// #endif
// #ifndef FAN3_PIN
//   #define FAN3_PIN                       PB14  // Fan3
// #endif

#ifdef TMC_USE_SW_SPI
#define TMC_SW_MOSI PD4
#define TMC_SW_MISO PD5
#define TMC_SW_SCK PD6
#endif

#if HAS_TMC_UART
/**
 * TMC2209 stepper drivers
 * Hardware serial communication ports.
 */
/**
 * Four TMC2209 drivers can use the same HW/SW serial port with hardware configured addresses.
 * Set the address using jumpers on pins MS1 and MS2.
 * Address | MS1  | MS2
 *       0 | LOW  | LOW
 *       1 | HIGH | LOW
 *       2 | LOW  | HIGH
 *       3 | HIGH | HIGH
 *
 * Set *_SERIAL_TX_PIN and *_SERIAL_RX_PIN to match for all drivers
 * on the same serial port, either here or in your board's pins file.
 */
// #define X_HARDWARE_SERIAL  PA2
// #define Y_HARDWARE_SERIAL  PA2
//#define Z_HARDWARE_SERIAL  PA2
//#define E0_HARDWARE_SERIAL PA2

// Default TMC slave addresses
#ifndef X_SLAVE_ADDRESS
#define X_SLAVE_ADDRESS 1
#endif
#ifndef Y_SLAVE_ADDRESS
#define Y_SLAVE_ADDRESS 3
#endif
#ifndef Z_SLAVE_ADDRESS
#define Z_SLAVE_ADDRESS 0
#endif
// #ifndef Z2_SLAVE_ADDRESS
//   #define Z2_SLAVE_ADDRESS  0
// #endif
#ifndef E0_SLAVE_ADDRESS
#define E0_SLAVE_ADDRESS 2
#endif
/**
 * TMC2208/TMC2209 stepper drivers
 *
 * Hardware serial communication ports.
 * If undefined software serial is used according to the pins below
 */

#define X_SERIAL_TX_PIN PA2
#define X_SERIAL_RX_PIN X_SERIAL_TX_PIN

#define Y_SERIAL_TX_PIN PA2
#define Y_SERIAL_RX_PIN Y_SERIAL_TX_PIN

#define Z_SERIAL_TX_PIN PA2
#define Z_SERIAL_RX_PIN Z_SERIAL_TX_PIN

#define Z2_SERIAL_TX_PIN PA3
#define Z2_SERIAL_RX_PIN Z2_SERIAL_TX_PIN

#define E0_SERIAL_TX_PIN PA2
#define E0_SERIAL_RX_PIN E0_SERIAL_TX_PIN
// // Reduce baud rate to improve software serial reliability
#define TMC_BAUD_RATE 19200
#endif