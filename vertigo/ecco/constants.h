// Copyright 2021 Adam Fulford
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
// 
// See http://creativecommons.org/licenses/MIT/ for more information.

#ifndef CONSTANTS_H
#define CONSTANTS_H

//#include "QSPI_Settings.h"

//#define MAX_DELAY static_cast<size_t>(48000 * 2.0f)
//#define MAX_REV_DELAY static_cast<size_t>(48000 * 5.0f)

constexpr float maxDelay{48000 * 5.0f};
constexpr float minDelay{48000 * 0.08f};

constexpr float maxRevDelay{48000 * 5.0f};
constexpr float minRevDelay{25000.0f};

constexpr int mintap{20000};    //in us (0.02s)
constexpr int maxtap{6000000};  //in us (6s)

constexpr int minTempo{960}; //0.02s in samples
constexpr int maxTempo{288000}; //6s in samples

constexpr float maxModAmp{200.0f};

constexpr float maxModRate{10.0f};
constexpr float minModRate{0.1f};

constexpr float maxModDepth{500.0f}; //used to be 1000
constexpr float minModDepth{0.0f};

constexpr float maxFB{1.25f};

constexpr float intThresh{0.005}; 

//constexpr float XFadeTime{0.005}; //in seconds
constexpr float XFadeTime{0.01}; //in seconds

//Filters
constexpr float default_Res{0.0};
constexpr float minRes{0.0};
constexpr float maxRes{0.5};

constexpr float defaultDrive{0.7f}; //used to be 0.2

//LPF
constexpr float defaultLPCut{12000.0};
constexpr float minLPCut{1500.0};
constexpr float maxLPCut{12000.0};

//HPF
constexpr float defaultHPCut{200.0};
constexpr float minHPCut{100.0};
constexpr float maxHPCut{800.0};
constexpr float FwdRevXFadeTime{0.5}; //in seconds

constexpr float NegFBCoeff{3.0f};
constexpr float NegFBOffset{0.4f};//0.4

constexpr int PPQN{1};

constexpr uint32_t shiftWait{50};
constexpr uint32_t saveWait{2000};
constexpr uint32_t resetWait{3000};

constexpr float altControlThresh{0.005f};

constexpr float AudioLimit{2.0f};

constexpr uint32_t ClockInWait{1000}; //in ms

constexpr float pickupTolerance{0.005f};

constexpr int updateDiv{15}; //division of samplerate LEDs/switches/pots are is updated

#define PIN_ADC_CV0 21
#define PIN_ADC_CV1 22
#define PIN_ADC_CV2 28
#define PIN_ADC_CV3 23
#define PIN_ADC_CV4 16
#define PIN_ADC_CV5 17
#define PIN_ADC_CV6 19

enum Pots
{
    KNOB_0,
    KNOB_1,
    KNOB_2,
    KNOB_3,
    KNOB_4,
    KNOB_5,
    KNOB_6,
    KNOB_LAST
};

#define PIN_TOGGLE3_0A 6
#define PIN_TOGGLE3_0B 5
#define PIN_TOGGLE3_1A 1
#define PIN_TOGGLE3_1B 0

enum AV_TOGGLE3
    {
        SW_0,
        SW_1,
        SW_LAST
    };

enum swState
{
    SW_OFF,
    SW_UP,
    SW_DOWN,
};

#define PIN_LED0_R 10
#define PIN_LED0_G 3
#define PIN_LED0_B 4
#define PIN_LED1_R 12
#define PIN_LED1_G 13
#define PIN_LED1_B 11
#define PIN_LED2_R 25
#define PIN_LED2_G 26
#define PIN_LED2_B 14
#define PIN_LED3_R 29
#define PIN_LED3_G 27
#define PIN_LED3_B 15

enum AV_LEDS
    {
        LED_0,
        LED_1,
        LED_2,
        LED_3,
        LED_LAST
    };

enum TempoDivs
{
    DIV6,
    DIV5,
    DIV4,
    DIV3,
    DIV2,
    UNITY,
    MULT2,
    MULT3,
    MULT4,
    MULT5,
    MULT6,

    MAXDIVS,
};

enum TapRatios
{
    QUARTER,
    DOTTED_EIGHT,
    EIGHT,
    TWELVE,

    MAXRATIOS,
};

enum pickupState
{
    BELOW,
    ABOVE,
    PICKEDUP,
};

enum SaveState
{
    IDLE,
    WAITING,
    SAVING,
};

#endif