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
#define BOARD_VERSION_PIN  PIN0
#define BOARD_CHIP_GPIO    GPIOA
#define BOARD_CHIP_PIN     PIN1
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
#define PROVE_CONTROL_PIN   PC5
#define PROVE_CONTROL_STATE HIGH
#define HOMING_RISE_SPEED   10 * 60 // mm/min
// Z Probe (when not Z_MIN_PIN)
#ifndef Z_MIN_PROBE_PIN
#if MACCHINE == MOTOR_5X_DVT_USE_G_SENSOR
#define Z_MIN_PROBE_PIN PD1
#else
#define Z_MIN_PROBE_PIN PA13
#endif
#endif
#else
#define HOMING_RISE_SPEED 10 * 60 // mm/min
// Z Probe (when not Z_MIN_PIN)
#ifndef Z_MIN_PROBE_PIN
#ifdef SENSORLESS_PROBING
#define Z_MIN_PROBE_PIN PB10
#else
#define Z_MIN_PROBE_PIN PA13 // PA13 
#endif
#endif
#endif

#if ENABLED(MOTOR_EN_CONTROL)
#define MOTOR_EN_PIN   PA7
#define MOTOR_EN_STATE HIGH
#endif

#if ENABLED(HEATER_EN_CONTROL)
#define HEATER_EN_PIN   PC2
#define HEATER_EN_STATE HIGH
#endif

#if ENABLED(ANKER_LEVEING)
#define ANKER_LEVEING_DELAY_BEFORE_PROBING_TRUE 1000
#define ANKER_LEVEING_DELAY_BEFORE_PROBING      200
#endif

#if ENABLED(HANDSHAKE)
#define HANDSHAKE_SDO   PC4
#define HANDSHAKE_SCK   PB0
#define HANDSHAKE_STATE HIGH
#define HANDSHAKE_TIME  5000 // ms
#endif

#if ENABLED(HOMING_BACKOFF)
#define HOMING_5X_BACKOFF_MM \
    {                        \
        0, 20, 0             \
    } // (mm) Backoff from endstops before  homing
#endif

#if ENABLED(WS1_HOMING_5X)
#define HOMING_DUAL_Z_RISZ  0 // (mm) Ascent distance between return to zero
#define HOMING_PROBE_Z_RISE 0 // (mm) The distance that the nozzle rises after moving to the middle point
#endif

#if ENABLED(EVT_HOMING_5X)
  #ifndef ANKER_Z_HOMING_SCRIPT
   #define ANKER_Z_HOMING_SCRIPT "G1 X2 Y-23 F12000\n"
  #endif

  #define ANKER_Z_SAFE_HOMING_X_POINT 50  // X point for Z homing
  #define ANKER_Z_SAFE_HOMING_Y_POINT -3.5  // Y point for Z homing
  #define ANKER_Z_AFTER_HOMING        2     // (mm) Halfway up the Z6.2
  #define ANKER_TPU_HOMING_SCRIPT_ABSOLUTE0     "G92 E0\nG1 X20 Y-15 F1500\nM106 S255\nM104 S150\nG4 S1\nG2 I0 J6 F600\nG2 I0 J6\nG0 X30 Y-3.5\nG2 I0 J3 F200\nG2 I0 J3\nG2 I0 J3\nG2 I0 J3\nG2 I0 J3\nG1 Y-15 F1500\nG0 X55 F1000\nG4 S1\nM107\nG1 X80\nG1 Y-12\nG1 X60\nG1 Y-10\nG1 X80\nG1 Y-8\nG1 X60\n"
  #define ANKER_TPU_HOMING_SCRIPT_NO_ABSOLUTE0  "G92 E0\nG1 X20 Y-15 F1500\nM106 S255\nM104 S150\nG4 S1\nG2 I0 J6 F600\nG2 I0 J6\nG0 X30 Y-3.5\nG2 I0 J3 F200\nG2 I0 J3\nG2 I0 J3\nG2 I0 J3\nG2 I0 J3\nG1 Y-15 F1500\nG0 X55 F1000\nG4 S1\nM107\nG1 X80\nG1 Y-12\nG1 X60\nG1 Y-10\nG1 X80\nG1 Y-8\nG1 X60\n"
  #define ANKER_HOMING_SCRIPT_ABSOLUTE0     "G92 E0\nG1 X20 Y-15 F1500\nM106 S255\nG4 S2\nM107\nG2 I0 J6 F600\nG2 I0 J6\nG1 X60 F1500\nG1 X162 E30 F1500\nG1 E25 F900\nG1 E20 F1800\nG1 E0 F3000\nG1 Y-12\nG1 X60 F1500\nG91\nG1 Z5 F600\nG4 S5\nG1 Y3\nG1 Z-5\nG90\nG1 X162 F1500\nG1 Y-6\nG1 X60\nG1 Y-3.5\nG1 X105 F1500\n"
  #define ANKER_HOMING_SCRIPT_ABSOLUTE1     "G1 X110 F1500\nG91\nG1 Z10 F600\nG4 S10\nG1 X10 Y0.4\nG1 Z-10.12\nG1 Z0.12 F60\nG1 X5 F200\nG90\nG1 X140 F600\nG1 X130\nG1 Y-2.0\nG1 X140\nG2 I0 J5 F600\nG92 E0\n"
  #define ANKER_HOMING_SCRIPT_NO_ABSOLUTE0  "G92 E0\nG1 X20 Y-15 F1500\nM106 S255\nG4 S2\nM107\nG2 I0 J6 F600\nG2 I0 J6\nG1 X60 F1500\nG1 X162 E30 F1500\nG1 E-5 F900\nG1 E-5 F1800\nG1 E-20 F3000\nG1 Y-12\nG1 X60 F1500\nG91\nG1 Z5 F600\nG4 S5\nG1 Y3\nG1 Z-5\nG90\nG1 X162 F1500\nG1 Y-6\nG1 X60\nG1 Y-3.5\nG1 X105 F1500\n"
  #define ANKER_HOMING_SCRIPT_NO_ABSOLUTE1  "G1 X110 F1500\nG91\nG1 Z10 F600\nG4 S10\nG1 X10 Y0.4\nG1 Z-10.12\nG1 Z0.12 F60\nG1 X5 F200\nG90\nG1 X140 F600\nG1 X130\nG1 Y-2.0\nG1 X140\nG2 I0 J5 F600\nG92 E0\n"
  #define ANKER_HOMING_SCRIPT_RESET_E0_ABS  "G92 E0\nG1 X50 Y-1 Z0.2 F3600\nG1 X105 E30 F1200\nG1 X110 F1500\nG1 X115 E28 F3000\nG92 E0\n"
  #define ANKER_HOMING_SCRIPT_RESET_E0_REL  "G92 E0\nG1 X50 Y-1 Z0.2 F3600\nG1 X105 E30 F1200\nG1 X110 F1500\nG1 X115 E2 F3000\nG92 E0\n"
  #define PROBE_HOMING_BUMP_MM      { 0, 0, 2 } 
#endif

#if ENABLED(ANKER_ANLIGN)
#define ALIGN_PER_RESET
#define ANLIGN_RISE      1 // ascent
#define ANLIGN_NUM       6
#define ANLIGN_MAX_VALUE 1.5
#define ANLIGN_ALLOWED   0.08//0.1 // Error allowed
#define SCREW_DISTANCE   293  // mm
#endif

#if ENABLED(ANKER_PROBE_DETECT_TIMES)
  #define Z_PROBE_DETECTION_DEVIATION 0.05f  // Acceptable deviation between detections
  #define Z_CLEARANCE_ERR_PROBE (Z_CLEARANCE_MULTI_PROBE + 5)// Wait longer to eliminate distractions
#endif

#if ENABLED(ANKER_ANLIGN_ONLY_Z)
#define ANLIGN_RISE    4 // ascent
#define ANLIGN_NUM     3
#define ANLIGN_ALLOWED 0.1 // Error allowed
#endif

#if ENABLED(ANKER_TMC_SET)
// #define TCOOLTHRS_X  500
// #define TCOOLTHRS_Y  520
// #define TCOOLTHRS_Z1 1000
// #define TCOOLTHRS_Z2 1000
#endif

#if ENABLED(ANKER_Z_OFFSET_FUNC)
#define CS1237_SCL_PIN      PB8
#define CS1237_SDA_PIN      PB9
#define CS1237_PWR_CTRL_PIN PE0
#endif

#ifdef SENSORLESS_HOMING
#define ANKER_SENSORLESS_HOMING_BACKOFF_MM \
    {                                      \
        5, 5, 0                            \
    } // (mm) Backoff from endstops before  homing
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
 #ifdef SENSORLESS_HOMING
    #define Z_MIN_PIN                          PA8  // Z1-STOP
    #define Z2_MIN_PIN                         PD14 // Z2-STOP Z2_MIN_PIN
 #else
    #define Z_MIN_PIN PB3 // PE1  // Z1-STOP
    #define Z_MAX_PIN PE1 // PB3  // Z2-STOP
 #endif
#else
#define Z_STOP_PIN PA13 // Z1-STOP  // PA13 
//#define Z2_STOP_PIN                     PD14  // Z1-STOP
#endif

#define X_ENABLE_PIN PE4
#define X_STEP_PIN   PB8
#define X_DIR_PIN    PE6
// #ifndef X_CS_PIN
//   #define X_CS_PIN                         PD1
// #endif

#define Y_ENABLE_PIN X_ENABLE_PIN
#define Y_STEP_PIN   PB11
#define Y_DIR_PIN    PE8
// #ifndef Y_CS_PIN
//   #define Y_CS_PIN                         PD0
// #endif

#define Z_ENABLE_PIN X_ENABLE_PIN
#define Z_STEP_PIN   PC8
#define Z_DIR_PIN    PB9

#define Z2_ENABLE_PIN X_ENABLE_PIN
#define Z2_STEP_PIN   PC9
#define Z2_DIR_PIN    PE3

// #define Z_ENABLE_PIN                       X_ENABLE_PIN
// #define Z_STEP_PIN                         PC8
// #define Z_DIR_PIN                          PB5

// #define Z2_ENABLE_PIN                      X_ENABLE_PIN
// #define Z2_STEP_PIN                        PC9
// #define Z2_DIR_PIN                         PB7

#define E0_ENABLE_PIN X_ENABLE_PIN
#define E0_STEP_PIN   PA3
#define E0_DIR_PIN    PB0

#define EN_24V_HEAT                           PC3
//
// Temperature Sensors
//
#define TEMP_BED_PIN PA4 // TB
#define TEMP_0_PIN   PA4 // TH0
//#define TEMP_BOARD_PIN                      PA0   // T BOARD
//
// Heaters / Fans
//
#ifndef HEATER_0_PIN
#define HEATER_0_PIN PE0 // Heater0
#endif

#ifndef HEATER_BED_PIN
#define HEATER_BED_PIN PB15 // Hotbed
#endif

//#define HEATER_0_INVERTING true
#define HEATER_BED_INVERTING true

#ifndef FAN_PIN
#define FAN_PIN PB2 // PC6 // Fan0
#endif
#ifndef FAN1_PIN
#define FAN1_PIN PB2 // PC7 // Fan1
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
#define TMC_SW_SCK  PD6
#elif ENABLED(ANKER_MOTION_TRACKING)
#define DEBUG_TP172 PD5
#define DEBUG_TP173 PD4
#define DEBUG_TP175 PD6
#endif

#if ENABLED(NO_MOTION_BEFORE_HOMING)
   #define DISABLED_MOTION_BEFORE_PIN    PD1
   #define DISABLED_MOTION_BEFORE_STATE  LOW

  #if MACCHINE == MOTOR_5X_DVT_USE_G_SENSOR
   #error "Z_MIN_PROBE_PIN = DISABLED_MOTION_BEFORE_PIN = PD1!"
  #endif
  //#if X_CS_PIN == PD1
 //  #error "X_CS_PIN = DISABLED_MOTION_BEFORE_PIN = PD1!"
 // #endif
  
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

#define Z2_SERIAL_TX_PIN PE9
#define Z2_SERIAL_RX_PIN Z2_SERIAL_TX_PIN

#define E0_SERIAL_TX_PIN PA2
#define E0_SERIAL_RX_PIN E0_SERIAL_TX_PIN
// // Reduce baud rate to improve software serial reliability
#define TMC_BAUD_RATE 19200
#endif