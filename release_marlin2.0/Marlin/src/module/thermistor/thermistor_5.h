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
#pragma once

// R25 = 100 kOhm, beta25 = 4267 K, 4.7 kOhm pull-up
// 100k ParCan thermistor (104GT-2)
// ATC Semitec 104GT-2/104NT-4-R025H42G (Used in ParCan)
// Verified by linagee. Source: https://www.mouser.com/datasheet/2/362/semitec%20usa%20corporation_gtthermistor-1202937.pdf
// Calculated using 4.7kohm pullup, voltage divider math, and manufacturer provided temp/resistance
constexpr temp_entry_t temptable_5[] PROGMEM = {
    { OV(   1), 713 },
    { OV(  20), 300 }, // top rating 300C
    { OV(  22), 290 },
    { OV(  26), 280 },
    { OV(  30), 270 },
    { OV(  36), 260 },
    { OV(  42), 250 },
    { OV(  49), 240 },
    { OV(  59), 230 },
    { OV(  69), 220 },
    { OV(  83), 210 },
    { OV(  100), 200 },
    { OV( 120), 190 },
    { OV( 144), 180 },
    { OV( 171), 170 },
    { OV( 210), 160 },
    { OV( 250), 150 },
    { OV( 311), 140 },
    { OV( 368), 130 },
    { OV( 432), 120 },
    { OV( 498), 110 },
    { OV( 539), 100 },
    { OV( 623),  90 },
    { OV( 702),  80 },
    { OV( 777),  70 },
    { OV( 838),  60 },
    { OV( 892),  50 },
    { OV( 913),  40 },
    { OV( 959),  30 },
    { OV( 986),  20 },
    { OV(1000),  10 },
    { OV(1010),   0 }
  };
