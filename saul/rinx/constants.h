#ifndef CONSTANTS_H
#define CONSTANTS_H

constexpr uint32_t shiftWait{1000};

enum LEDS
{
    LED_NOTE,
    LED_EXCITER,
    LED_STRUM,
    LED_S2_R,
    LED_POLY4,
    LED_S4_L,
    LED_POLY2,
    LED_S4_R,
    LED_1_RED,
    LED_1_GREEN,
    LED_1_BLUE,
    LED_2_RED,
    LED_2_GREEN,
    LED_2_BLUE,
    LED_3_RED,
    LED_3_GREEN,
    LED_3_BLUE,
    LED_4_RED,
    LED_4_GREEN,
    LED_4_BLUE,
    LED_POT9,
    LED_POT11
};

enum PUSHBUTTONS
{
    BTN_NOTE_STRUM,
    BTN_EXCITER,
    BTN_EGG_FX,
    BTN_MODEL,
    BTN_POLY,
    BTN_4,
    BTN_TAP
};

enum SWITCHES
{
    PIN_SW_LEFT_A = 18,
    PIN_SW_LEFT_B = 19,
    PIN_SW_RIGHT_A = 20,
    PIN_SW_RIGHT_B = 21
};

#endif