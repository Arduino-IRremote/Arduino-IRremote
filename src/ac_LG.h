/*
 * ac_LG.h
 *
 *  Contains definitions for receiving and sending LG air conditioner IR Protocol
 *
 *  This file is part of Arduino-IRremote https://github.com/Arduino-IRremote/Arduino-IRremote.
 *
 ************************************************************************************
 * MIT License
 *
 * Copyright (c) 2021 Armin Joachimsmeyer
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is furnished
 * to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
 * OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 ************************************************************************************
 */
// see also: https://github.com/crankyoldgit/IRremoteESP8266/blob/master/src/ir_LG.h
#include <Arduino.h>

/** \addtogroup Airconditoners Air conditioner special code
 * @{
 */

#define LG_ADDRESS  0x88

/*
 * The basic IR command codes
 * Parts of the codes (especially the lower nibbles) may be modified to contain
 * additional information like temperature, fan speed and minutes.
 */
#define LG_SWITCH_ON_MASK       0x0800  // This bit is masked if we switch Power on
#define LG_MODE_COOLING         0x0800  // Temperature and fan speed in lower nibbles
#define LG_MODE_DEHUMIDIFIYING  0x0990  // sets also temperature to 24 and fan speed to 0
#define LG_MODE_FAN             0x0A30  // sets also temperature to 18
#define LG_MODE_AUTO            0x0B00  // The remote initially sets also temperature to 22 and fan speed to 4
#define LG_MODE_HEATING         0x0C00  // Temperature and fan speed in lower nibbles
#define LG_ENERGY_SAVING_ON     0x1004
#define LG_ENERGY_SAVING_OFF    0x1005
#define LG_JET_ON               0x1008
#define LG_WALL_SWING_ON        0x1314
#define LG_WALL_SWING_OFF       0x1315
#define LG_SWING_ON             0x1316  // not verified, for AKB73757604
#define LG_SWING_OFF            0x1317  // not verified, for AKB73757604
#define LG_TIMER_ON             0x8000  // relative minutes in lower nibbles
#define LG_TIMER_OFF            0x9000  // relative minutes in lower nibbles
#define LG_SLEEP                0xA000  // relative minutes in lower nibbles
#define LG_CLEAR_ALL            0xB000  // Timers and sleep
#define LG_POWER_DOWN           0xC005
#define LG_LIGHT                0xC00A
#define LG_AUTO_CLEAN_ON        0xC00B
#define LG_AUTO_CLEAN_OFF       0xC00C

/*
 * Commands as printed in menu and uses as first parameter for sendCommandAndParameter
 */
#define LG_COMMAND_OFF          '0'
#define LG_COMMAND_ON           '1'
#define LG_COMMAND_SWING        's'
#define LG_COMMAND_AUTO_CLEAN   'a'
#define LG_COMMAND_JET          'j'
#define LG_COMMAND_ENERGY       'e'
#define LG_COMMAND_LIGHT        'l'
#define LG_COMMAND_FAN_SPEED    'f'
#define LG_COMMAND_TEMPERATURE  't'
#define LG_COMMAND_TEMPERATURE_PLUS '+'
#define LG_COMMAND_TEMPERATURE_MINUS '-'
#define LG_COMMAND_MODE         'm'
#define LG_COMMAND_SLEEP        'S'
#define LG_COMMAND_TIMER_ON     'T'
#define LG_COMMAND_TIMER_OFF    'O'
#define LG_COMMAND_CLEAR_ALL    'C'

/*
 * The modes are encoded as character values for easy printing :-)
 */
#define AC_MODE_COOLING         'c'
#define AC_MODE_DEHUMIDIFIYING  'd'
#define AC_MODE_FAN             'f'
#define AC_MODE_AUTO            'a'
#define AC_MODE_HEATING         'h'

// see https://github.com/crankyoldgit/IRremoteESP8266/blob/master/src/ir_LG.h
union LGProtocol {
    uint32_t raw;  ///< The state of the IR remote in IR code form.
    struct {
        uint32_t Checksum :4;
        uint32_t Fan :3;
        uint32_t FanExt :1;
        uint32_t Temp :4;
        uint32_t Mode :4;
        uint32_t Function :3;
        uint32_t SwitchOnMask :1; /* Content is 0 when switching from off to on */
        uint32_t Signature :8; /* Content is 0x88, LG_ADDRESS */
    };
};

class Aircondition_LG {
public:
    bool sendCommandAndParameter(char aCommand, int aParameter);
    void setType(bool aIsWallType);
    void printMenu();
    void sendIRCommand(uint16_t aCommand);
    void sendTemperatureFanSpeedAndMode();
    /*
     * Internal state of the air condition
     */
#define LG_IS_WALL_TYPE true
#define LG_IS_TOWER_TYPE false
    bool ACIsWallType;      // false : TOWER, true : WALL
    bool PowerIsOn;

    // These value are encoded and sent by AC_LG_SendCommandAndParameter()
    uint8_t FanIntensity = 1;    // 0 -> low, 4 high, 5 -> cycle
    uint8_t Temperature = 22;    // temperature : 18 ~ 30
    uint8_t Mode = AC_MODE_COOLING;
    bool useLG2Protocol = false;
};

/** @}*/
