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

#define BOARD_INFO_NAME "Anet EX 2.0"

#include "env_validate.h"

// Use one of these or SDCard-based Emulation will be used
#if NO_EEPROM_SELECTED
  //#define SRAM_EEPROM_EMULATION                 // Use BackSRAM-based EEPROM emulation
  #define FLASH_EEPROM_EMULATION                  // Use Flash-based EEPROM emulation
#endif

#if ENABLED(FLASH_EEPROM_EMULATION)
  // Decrease delays and flash wear by spreading writes across the
  // 128 kB sector allocated for EEPROM emulation.
  #define FLASH_EEPROM_LEVELING
#endif

#define HAS_OTG_USB_HOST_SUPPORT                  // USB Flash Drive support

// Avoid conflict with TIMER_TONE
#define STEP_TIMER 10

//
// Servos
//
#define SERVO0_PIN                          PC6

//
// Trinamic Stallguard pins
//
#define X_DIAG_PIN                          PB11   // X-STOP
#define Y_DIAG_PIN                          PE15   // Y-STOP
#define Z_DIAG_PIN                          PB10   // Z-STOP
#define E0_DIAG_PIN                         PD12   // E0DET
#define E1_DIAG_PIN                         PD13   // E1DET

//
// Limit Switches
//
#define X_STOP_PIN                        PB11   // X-STOP
#define Y_STOP_PIN                        PE15   // Y-STOP
#ifndef Z_STOP_PIN
  #define Z_STOP_PIN                      PB10   // Z-STOP
#endif

//
// Z Probe (when not Z_MIN_PIN)
//
#ifndef Z_MIN_PROBE_PIN
  #define Z_MIN_PROBE_PIN                   PD14
#endif

//
// Probe enable
//
#if ENABLED(PROBE_ENABLE_DISABLE)
  #ifndef PROBE_ENABLE_PIN
    #define PROBE_ENABLE_PIN          SERVO0_PIN
  #endif
#endif

//
// Filament Runout Sensor
//
#define FIL_RUNOUT_PIN                      PD12   // E0DET
#define FIL_RUNOUT2_PIN                     PD13   // E1DET

//
// Power Supply Control
//
#ifndef PS_ON_PIN
  #define PS_ON_PIN                         PB6   // PS-ON
#endif

//
// Power Loss Detection
//
#ifndef POWER_LOSS_PIN
  #define POWER_LOSS_PIN                    PB7  // PWRDET
#endif

//
// Control pin of driver/heater/fan power supply
//
#define SAFE_POWER_PIN                      PB8

//
// Steppers
//
#define X_STEP_PIN                          PC1
#define X_DIR_PIN                           PC2
#define X_ENABLE_PIN                        PC3
#ifndef X_CS_PIN
  #define X_CS_PIN                          PE0
#endif

#define Y_STEP_PIN                          PC14
#define Y_DIR_PIN                           PC15
#define Y_ENABLE_PIN                        PC0
#ifndef Y_CS_PIN
  #define Y_CS_PIN                          PD3
#endif

#define Z_STEP_PIN                          PE5
#define Z_DIR_PIN                           PE6
#define Z_ENABLE_PIN                        PC13
#ifndef Z_CS_PIN
  #define Z_CS_PIN                          PB9
#endif

#ifndef E0_STEP_PIN
  #define E0_STEP_PIN                       PE4
#endif
#ifndef E0_DIR_PIN
  #define E0_DIR_PIN                        PE3
#endif
#ifndef E0_ENABLE_PIN
  #define E0_ENABLE_PIN                     PE2
#endif
#ifndef E0_CS_PIN
  #define E0_CS_PIN                         PE1
#endif

//
// Temperature Sensors
//
#ifndef TEMP_0_PIN
  #define TEMP_0_PIN                        PA6   // TH0
#endif
#ifndef TEMP_BED_PIN
  #define TEMP_BED_PIN                      PA3   // TB
#endif

//
// Heaters / Fans
//
#ifndef HEATER_0_PIN
  #define HEATER_0_PIN                      PA2   // Heater0
#endif
#ifndef HEATER_BED_PIN
  #define HEATER_BED_PIN                    PA1   // Hotbed
#endif
#ifndef FAN_PIN
  #define FAN_PIN                           PA0   // Fan0
#endif

#if HAS_TMC_UART
  /**
   * TMC2208/TMC2209 stepper drivers
   *
   * Hardware serial communication ports.
   * If undefined software serial is used according to the pins below
   */

  //
  // Software serial
  //
  #define X_SERIAL_TX_PIN                   PE0
  #define X_SERIAL_RX_PIN        X_SERIAL_TX_PIN

  #define Y_SERIAL_TX_PIN                   PD3
  #define Y_SERIAL_RX_PIN        Y_SERIAL_TX_PIN


  // Reduce baud rate to improve software serial reliability
  #define TMC_BAUD_RATE                    19200
#endif

//
// SD Connection
//
#ifndef SDCARD_CONNECTION
  #define SDCARD_CONNECTION              CUSTOM_CABLE
#endif

/**
 *                   ------                                              ------
 *     (BEEPER) PA8 | 1  2 | PC8  (BTN_ENC)                 (MISO) PC11 | 1  2 | PC10 (SCK)
 *     (LCD_EN) PD8 | 3  4 | PD9  (LCD_RS)               (BTN_EN1) PA11 | 3  4 | PA15 (SD_SS/MKS_IO4)
 *    (LCD_D4) PD11 | 5  6   PC9  (LCD_D5)               (BTN_EN2) PD10 | 5  6   PC12 (MOSI)
 *  (LCD_D6/TX) PA9 | 7  8 | PA10 (LCD_D7/RX)  (SD_DETECT/MKS_RST) PA12 | 7  8 | RESET
 *              GND | 9 10 | 5V                                     GND | 9 10 | --
 *                   ------                                         ------
 *                    EXP1                                           EXP2
 */
#define EXP1_01_PIN                         PA8
#define EXP1_02_PIN                         PC8
#define EXP1_03_PIN                         PD8
#define EXP1_04_PIN                         PD9
#define EXP1_05_PIN                         PD11
#define EXP1_06_PIN                         PC9
#define EXP1_07_PIN                         PA9
#define EXP1_08_PIN                         PA10

#define EXP2_01_PIN                         PC11
#define EXP2_02_PIN                         PC10
#define EXP2_03_PIN                         PD0
#define EXP2_04_PIN                         PA15
#define EXP2_05_PIN                         PD10
#define EXP2_06_PIN                         PC12
#define EXP2_07_PIN                         PA12
#define EXP2_08_PIN                         -1

//
// SD card. Hardware SPI3
//
#if SD_CONNECTION_IS(LCD)
  #define SDSS                       EXP2_04_PIN
  #define SD_SS_PIN                         SDSS
  #define SD_SCK_PIN                 EXP2_02_PIN
  #define SD_MISO_PIN                EXP2_01_PIN
  #define SD_MOSI_PIN                EXP2_06_PIN
  #define SD_DETECT_PIN              EXP2_07_PIN
#endif

//
// LCDs and Controllers
//
#if IS_TFTGLCD_PANEL

  #if ENABLED(TFTGLCD_PANEL_SPI)
    #define TFTGLCD_CS               EXP2_03_PIN
  #endif

#elif HAS_WIRED_LCD

  #define BEEPER_PIN               EXP1_01_PIN
  #define BTN_ENC                  EXP1_02_PIN

  #if ENABLED(CR10_STOCKDISPLAY)

    #define LCD_PINS_RS              EXP1_07_PIN

    #define BTN_EN1                  EXP1_03_PIN
    #define BTN_EN2                  EXP1_05_PIN

    #define LCD_PINS_ENABLE          EXP1_08_PIN
    #define LCD_PINS_D4              EXP1_06_PIN

  #elif ENABLED(MKS_MINI_12864)

    #define DOGLCD_A0                EXP1_07_PIN
    #define DOGLCD_CS                EXP1_06_PIN
    #define BTN_EN1                  EXP2_03_PIN
    #define BTN_EN2                  EXP2_05_PIN

  #else

    #define BTN_EN1                  EXP2_03_PIN
    #define BTN_EN2                  EXP2_05_PIN

    #if ENABLED(FYSETC_MINI_12864)
      #define DOGLCD_SCK             EXP2_02_PIN
      #define DOGLCD_MOSI            EXP2_06_PIN
      #define DOGLCD_CS              EXP1_03_PIN
      #define DOGLCD_A0              EXP1_04_PIN
      #define LCD_RESET_PIN          EXP1_05_PIN  // Must be high or open for LCD to operate normally.
      #if ENABLED(FYSETC_MINI_12864_2_1)
        #define NEOPIXEL_PIN         EXP1_06_PIN
      #endif
    #endif // !FYSETC_MINI_12864
  #endif
#endif // HAS_WIRED_LCD

// Alter timing for graphical display
#if IS_U8GLIB_ST7920
  #ifndef BOARD_ST7920_DELAY_1
    #define BOARD_ST7920_DELAY_1             120
  #endif
  #ifndef BOARD_ST7920_DELAY_2
    #define BOARD_ST7920_DELAY_2              80
  #endif
  #ifndef BOARD_ST7920_DELAY_3
    #define BOARD_ST7920_DELAY_3             580
  #endif
#endif

#if HAS_SPI_TFT

  #define BTN_EN1                    EXP2_03_PIN
  #define BTN_EN2                    EXP2_05_PIN
  #define BTN_ENC                    EXP1_02_PIN

  //
  // 480x320, 3.5", SPI Stock Display with Rotary Encoder
  //
  #define TFT_CS_PIN                 EXP2_04_PIN
  #define TFT_A0_PIN                 EXP2_07_PIN
  #define TFT_DC_PIN                 EXP1_07_PIN
  #define TFT_SCK_PIN                EXP2_02_PIN
  #define TFT_MISO_PIN               EXP2_01_PIN
  #define TFT_MOSI_PIN               EXP2_06_PIN
  #define TFT_RESET_PIN              EXP1_08_PIN

  #define TOUCH_INT_PIN              EXP1_07_PIN
  #define TOUCH_MISO_PIN             EXP1_06_PIN
  #define TOUCH_MOSI_PIN             EXP1_03_PIN
  #define TOUCH_SCK_PIN              EXP1_05_PIN
  #define TOUCH_CS_PIN               EXP1_04_PIN

  //#define LCD_USE_DMA_SPI

#endif

//
// NeoPixel LED
//
#ifndef NEOPIXEL_PIN
  #define NEOPIXEL_PIN                      PC9
#endif

//
// MKS WIFI
//
#define MKS_WIFI

#ifdef MKS_WIFI
  #define MKS_WIFI_SERIAL_NUM                SERIAL_PORT_2
  #define MKS_WIFI_SERIAL_BAUDRATE           BAUDRATE_2
  #undef  PLATFORM_M997_SUPPORT

  #define MKS_WIFI_IO4                       EXP2_04_PIN
  #define MKS_WIFI_IO_RST                    EXP2_07_PIN
#endif

