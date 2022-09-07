/* 
 * Copyright (C) 2021, 2022 Evan Pernu. Author: Evan Pernu
 * 
 * You may use, distribute and modify this code under the
 * terms of the GNU AGPLv3 license.
 * 
 * This program is part of "Evan's Daisy Projects".
 * 
 * "Evan's Daisy Projects" is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
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
 * ====================================================
 * =   __       _        _                            =
 * =   \ \  ___| | |_   _| |__   ___  __ _ _ __  ___  =
 * =    \ \/ _ \ | | | | | '_ \ / _ \/ _` | '_ \/ __| =
 * = /\_/ /  __/ | | |_| | |_) |  __/ (_| | | | \__ \ =
 * = \___/ \___|_|_|\__, |_.__/ \___|\__,_|_| |_|___/ =
 * =                |___/                             =
 * ====================================================
 * 
 * Jellybeans is a diatonic quanitizing arpeggiator designed
 * for the Electrosmith Daisy Patch eurorack module.
 */

#include "daisysp.h"
#include "Jarp.h"
#include "resources.h"

#include <string>
#include <array>

// 0: OpMode - Mode of operation
//     0: Arp: Standard mode
//     1: Quant: Disable the arp completely; act as a quantizer. The incoming note
//               from CTRL 4 will be quantized and sent out throuhg CV OUT 1.
#define RAM_OP_MODE 0
#define RAM_BPM 1
#define RAM_ROOT 2
#define RAM_MODE 3
#define RAM_CLOCK_DIV 4
#define RAM_CLOCK_MODE 5
#define RAM_IN_TUNE 6
#define RAM_ARP_TUNE 7
#define RAM_BASS_TUNE 8
#define RAM_ARP_OCT 9
#define RAM_BASS_OCT 10
// Used to determine if this is a fresh install of Jellybeans, in which case default
// settings will be written to QSPI.
#define RAM_IS_INIT 11 

using namespace daisy;
using namespace daisysp;
using namespace jarp;
using namespace ev_theory;

/*
 * Update this with each change
 */
const std::string VERSION = "1.4.1";

/*
 * Change this to enable debug output
 */
const bool DEBUG_MODE = false;

// Some settings are stored in QSPI memory and persisted on startup
const uint8_t SETTINGS_BUFF_SIZE = 12;
uint8_t DSY_QSPI_BSS settings_qspi[SETTINGS_BUFF_SIZE];
uint8_t settings_ram[SETTINGS_BUFF_SIZE];

uint8_t noteInIndex;
constexpr float pickupTolerance{0.005f};
bool shift{false};  

// State objects
Arp*        arp;
Rhythm*    rhythm;

// Previous note in's semitone value
uint8_t lastNote;

// Used to track time divisions
// TODO factor this out into a rhythm sequencer library
uint16_t divMax;
uint16_t divCounter;

// DAC output value of bass note
float bassDac;

// Offset applied to inbound notes (semitones)
int8_t       inTune;
const int8_t MIN_IN_TUNE = -12;
const int8_t MAX_IN_TUNE =  12;

// Offsets applied to outbound note CVs (cents)
float        arpOutTune;
float        bassOutTune;
const int8_t MIN_OUT_TUNE = -100;
const int8_t MAX_OUT_TUNE =  100;
int16_t      bassOctMod;
int16_t      arpOctMod;
const int8_t MIN_OCT_MOD  = -4;
const int8_t MAX_OCT_MOD  =  4;

// Tracks the blinking icon next to bpm
int       blink;
const int BLINK_FRAMES = 35;
int       blinkgate;
const int BLINK_GATE_FRAMES = 20;

bool clockmode = false;
bool last_clockmode = false;
int clockdivindex = 0;
//int arpoctindex; 
//int16_t arpoctave;

bool gate1 = false;
int scalemode;
int scalevoicing;
int inversionindex;
int patternindex;


void updateControls();
void updateOutputs();
void updatePattern();
void updateVoicing();
void updateInversion();
void updateMode();
void updateBpm();
void updateClockdiv();
void updateArpoct();
void updateLeds();
void updateBtns();
float AnalogVal(int knobidx,int cvidx);
void SetRGBColor (DaisyWhite::LeddriverLeds idx, int color);

// Copy the values in settings_ram (volatile) to settings_qspi (non-volatile)
void saveSettingsToQSPI(){
    size_t size = sizeof(settings_qspi);
    size_t address = (size_t)settings_qspi;
    white.seed.qspi.Erase(address, address + size);
    white.seed.qspi.Write(address, size, (uint8_t*)settings_ram);
}

void loadQSPISettingsToRAM(){
    memcpy(settings_ram, settings_qspi, sizeof(settings_ram));
}

// Called when there's a fresh install of Jellybeans
void saveDefaultSettingsToQSPI(){
    settings_ram[RAM_CLOCK_DIV]  = 5;
    settings_ram[RAM_ARP_OCT]    = 0;
    settings_ram[RAM_ROOT]       = 0;
    settings_ram[RAM_MODE]       = 0;
    settings_ram[RAM_BASS_OCT]   = 0;
    settings_ram[RAM_OP_MODE]    = 0;
    settings_ram[RAM_IN_TUNE]    = 0 - MIN_IN_TUNE;
    settings_ram[RAM_ARP_TUNE]   = 0 - MIN_OUT_TUNE;
    settings_ram[RAM_BASS_TUNE]  = 0 - MIN_OUT_TUNE;
    settings_ram[RAM_IS_INIT]    = 42;
    //saveSettingsToQSPI();
}

// Compute a new bass CV value
void updateBassNote(){
    int semi = arp->getChord()->getRoot();
    semi += SEMIS_PER_OCT * 1;
    bassDac = semitoneToDac(semi);
};

/* Callback functions */

void cbRoot(){
    arp->getChord()->setModeRoot(settings_ram[RAM_ROOT]);
    arp->updateTraversal();
    updateBassNote();
};

void cbArpOct(){
    //arpoctave = allOctaves2[arpoctindex];
    //arpOctMod = semitoneToDac(arpoctave * SEMIS_PER_OCT);
};

void cbBassOct(){
    bassOctMod = semitoneToDac(settings_ram[RAM_BASS_OCT] * SEMIS_PER_OCT);
};

void cbNoteIn(){
    arp->getChord()->setDegreeByNote(noteInIndex);
    arp->updateTraversal();
    updateBassNote();
};

void cbClockDiv(){
    if(!clockmode){
        // Internal: timing is determined by clock divisions
        // divMax = clockDivTo256ths.at((menu->getItem("Clock Div")->getValue()));
        divMax = clockDivTo256ths2[clockdivindex];
    } else {
        // External: timing is determined by pulses per note.
        // Fractional clock values will just be set to 1.
        divMax = std::max(clockDivTo256ths2[clockdivindex]/256, 1);
    }
    divCounter = 0;
};

void cbClockMode(){
    rhythm->setClock(!clockmode);
    cbClockDiv();   
};

void cbOpMode(){
    settings_ram[RAM_OP_MODE] = 0;
};



void cbInTune(){
    //inTune = menu->getItem("In Tune")->getIndex();
    inTune = settings_ram[RAM_IN_TUNE] + MIN_IN_TUNE;

    // Since settings must be uints, need to add an offset to ensure 
    // the vlue written is > 0. 
    settings_ram[RAM_IN_TUNE] = inTune - MIN_IN_TUNE;
    //saveSettingsToQSPI();
}

void cbArpOutTune(){
    arpOutTune = centsToDac(0);

    settings_ram[RAM_ARP_TUNE] = 0 - MIN_OUT_TUNE;
    //saveSettingsToQSPI();
}

void cbBassOutTune(){
    bassOutTune = centsToDac(0);

    settings_ram[RAM_BASS_TUNE] = 0 - MIN_OUT_TUNE;
    //saveSettingsToQSPI();
}

// Invoked whenever the timer ticks
void cbRhythm(){
    divCounter++;
    divCounter = divCounter % divMax;
    if(divCounter == 0){
       arp->onClockPulse();
    }
};


int main(void) {
    // Initialize vars and objects
    white.Init();
    arp   = new Arp();
    rhythm = new Rhythm(true, cbRhythm);

    //loadQSPISettingsToRAM();

    // If the RAM_IS_INIT setting isn't 42, we know that Jellybeans doesn't have settings
    // here and we should use default settings.
    if (settings_ram[RAM_IS_INIT] != 42){
        saveDefaultSettingsToQSPI();
    } 

    if (!DEBUG_MODE){
        // Boot screen gets annoying during development
        //gui->drawStartupScreen("Jellybeans", VERSION, 1500);
    }

    bassDac    = 0.f;
    lastNote   = 0;
    divMax     = clockDivTo256ths2[5];
    divCounter = 0;
    blink      = 0;
    arpOctMod  = 0.f;
    bassOctMod = 0.f;

    // "Prime" menu items
    cbInTune();
    cbArpOutTune();
    cbBassOutTune();
    cbRoot();
    cbClockMode(); // NOTE: cvClockMode() calls cbClockDiv()
    //patch->ClearLeds();

    // "In case if you wondered, the fucking thing starts the circular DMA transfer
    // that receives ADC readings from knobs / CV inputs."
    //
    // Thanks, antisvin :P
    white.StartAdc();

    // Main event loop
    while(1){
        
        updateControls();
        updatePattern();
        updateVoicing();
        updateInversion();
        updateMode();
        updateBpm();
        updateClockdiv();
        updateOutputs();
        updateBtns();
        updateLeds();
        rhythm->update();
        white.DelayMs(1);
    }
}

// Handle any input to Patches' hardware
void updateControls() {
    white.ProcessAllControls();
    
    // Read v/oct from CTRL 4
    float knobpitch = white.GetKnobValue(DaisyWhite::KNOB_2);
    float cvpitch = white.GetCvValue(DaisyWhite::CV_2);
    float thepitch = white.CVKnobCombo(cvpitch,knobpitch);
    uint8_t i = static_cast<uint8_t>(std::round(thepitch*60.f) + inTune);
    //i = quantizeNoteToRange(i); // TODO this may be redundant

    // Check that a new cv value has been input, otherwise encoder input to
    // note in won't work. Might remove later and just let CTRL 4 handle it
    if(i !=  lastNote){
        lastNote = i;
        ///menu->getItem("Note In")->setIndex(i);
        noteInIndex = i;
        cbNoteIn();
    }
    
    // GATE IN 1 -> clock pulse
    blink--;
    if(white.TrigIn1()){
        blink = BLINK_FRAMES;
        rhythm->pulse();
        
    } 
    
}

void updateOutputs()
{
    uint16_t dac1 = 0;
    uint16_t dac2 = 0;
    
    
    switch (0){
        case 0:
            // Arp CV -> CV OUT 1
            dac1 = arp->getDacValue() + arpOutTune + arpOctMod;
            // Bass CV -> CV OUT 2
            dac2 = bassDac + bassOutTune + bassOctMod;
            // Arp Gate -> GATE OUT 1
            gate1 = arp->getTrig();
            blinkgate--;
            if(gate1){
                blinkgate = BLINK_GATE_FRAMES;                
            } 
            
            break;

        case 1:
            dac1 = bassDac + arpOutTune + arpOctMod;
            dac2 = bassDac + bassOutTune + bassOctMod;
            break;
    } 

    dsy_gpio_write(&white.gate_out_1, gate1);
    white.seed.dac.WriteValue(DacHandle::Channel::ONE, prepareDacValForOutput(dac1));
    white.seed.dac.WriteValue(DacHandle::Channel::TWO, prepareDacValForOutput(dac2));
}

void updateBtns()
{
    if(!white.SwitchState(S0A))
    {
        clockmode = true;        
    } else {
        clockmode = false;  
    }
    if (last_clockmode != clockmode)
    {
        cbClockMode();
        last_clockmode = clockmode;
    }
    if(!white.SwitchState(S1A))
    {
        arpOctMod = semitoneToDac(allOctaves2[2] * SEMIS_PER_OCT);           
    } else if (!white.SwitchState(S1B)) {
         arpOctMod = semitoneToDac(allOctaves2[0] * SEMIS_PER_OCT);
    } else {
        arpOctMod = semitoneToDac(allOctaves2[1] * SEMIS_PER_OCT);
    }
    
}

void updateLeds()
{
    white.ClearLeds();
    
    if (((blinkgate > 0) && !clockmode) || ((blink > 0) && clockmode)) {
                //white.SetRgbLeds(DaisyWhite::RGB_LED_4,0.f,0.f,1.f);
                //SetRGBColor(DaisyWhite::RGB_LED_4,blue);
                ///*
                switch (arp->getTraversalIndex())
                {
                case 0:
                    white.SetGreenLeds(DaisyWhite::GREEN_LED_3,1.f);
                    break;
                case 1:
                    white.SetGreenLeds(DaisyWhite::GREEN_LED_4,1.f);
                    break;
                case 2:
                    white.SetGreenDirectLeds(DaisyWhite::GREEN_D_LED_4,1.f);
                    break;
                case 3:
                    white.SetGreenDirectLeds(DaisyWhite::GREEN_D_LED_2,1.f);
                    break;
                case 4:
                    white.SetGreenDirectLeds(DaisyWhite::GREEN_D_LED_6,1.f);
                    break;
                case 5:
                    white.SetGreenDirectLeds(DaisyWhite::GREEN_D_LED_5,1.f);
                    break;
                case 6:
                    white.SetGreenDirectLeds(DaisyWhite::GREEN_D_LED_3,1.f);
                    break;
                case 7:
                    white.SetGreenDirectLeds(DaisyWhite::GREEN_D_LED_1,1.f);
                    break;
                }
                 //*/
    }
   
    if(!clockmode) 
    {
        //white.SetRgbLeds(DaisyWhite::RGB_LED_1,0.f,0.7f,0.f);
       white.SetGreenLeds(DaisyWhite::GREEN_LED_1,1.f);
    }

    SetRGBColor(DaisyWhite::RGB_LED_4,scalemode); 
    SetRGBColor(DaisyWhite::RGB_LED_3,scalevoicing); 
    SetRGBColor(DaisyWhite::RGB_LED_2,inversionindex); 
    SetRGBColor(DaisyWhite::RGB_LED_1,patternindex); 
    
    

    white.UpdateLeds();
}

int old_ipattern = 0;
void updatePattern()
{
    
    float pattern_target = fmap(AnalogVal(DaisyWhite::KNOB_4,DaisyWhite::CV_4),0.0,7.0);
    int ipattern = (int) pattern_target;

    //int poly = polyListValue.GetIndex();
    if(old_ipattern != ipattern)
    {
        arp->setPattern(ipattern);
        patternindex = ipattern;
    }
    old_ipattern = ipattern;

}

int old_ivoicing = 0;
void updateVoicing()
{
    float voicing_target = fmap(AnalogVal(DaisyWhite::KNOB_6,DaisyWhite::CV_6),0.0,14.0);
    int ivoicing = (int) voicing_target;

    //int poly = polyListValue.GetIndex();
    if(old_ivoicing != ivoicing)
    {
        arp->getChord()->setVoicing(ivoicing);
        arp->updateTraversal();
        scalevoicing = ivoicing;
    }
    old_ivoicing = ivoicing;
}

int old_iinversion = 0;
void updateInversion()
{
    float inversion_target = fmap(AnalogVal(DaisyWhite::KNOB_5,DaisyWhite::CV_5),0.0,4.0);
    int iinversion = (int) inversion_target;

    //int poly = polyListValue.GetIndex();
    if(old_iinversion != iinversion)
    {
        arp->getChord()->setInversion(iinversion);
        arp->updateTraversal();
        inversionindex = iinversion;
    }
    old_iinversion = iinversion;
}

int old_imode = 0;
void updateMode()
{
    float mode_target = fmap(AnalogVal(DaisyWhite::KNOB_7,DaisyWhite::CV_7),0.0,MODES_LAST);
    int imode = (int) mode_target;

    //int poly = polyListValue.GetIndex();
    if(old_imode != imode)
    {
        arp->getChord()->setMode(imode);
        arp->updateTraversal();
        scalemode = imode;
        //updateBassNote();
    }
    old_imode = imode;
}

uint16_t old_ibpm = 0;
void updateBpm()
{
    float bpm_target = fmap(AnalogVal(DaisyWhite::KNOB_0,DaisyWhite::CV_0),20.0,500.0);
    uint16_t ibpm = (uint16_t) bpm_target;

    //int poly = polyListValue.GetIndex();
    if(old_ibpm != ibpm)
    {
        rhythm->setBPM(ibpm);
    }
    old_ibpm = ibpm;
}

int old_iclockdiv = 0;
void updateClockdiv()
{

    float clockdiv_target = fmap(AnalogVal(DaisyWhite::KNOB_3,DaisyWhite::CV_3),0.0,15.0);
    int iclockdiv = (int) clockdiv_target;

    if(old_iclockdiv != iclockdiv)
    {
        clockdivindex = iclockdiv;
        cbClockDiv();
    }
    old_iclockdiv = iclockdiv;
}



float AnalogVal(int knobidx, int cvidx) 
{
    static bool _pickup{};
    static float _Last{};

    //update pot values
    float _Pot{white.GetKnobValue(knobidx)};
    float _Cv{white.GetCvValue(cvidx)};
   
    static float _new{};
    //udpate pickup
    
    if(!_pickup)  //not picked up
    {
        if(abs(_Pot - _new) > pickupTolerance)  //checked if changed from new value
        {
            _pickup = true;   //set to picked up
        }
    } 
    

    float _potcvval{};

    if(_pickup)
    {
        _potcvval = white.CVKnobCombo(_Cv,_Pot);
        _Last = _Pot;
        
    }

    else
    {
        _potcvval =  white.CVKnobCombo(_Cv,_Last);
        
    }
    return _potcvval;
}

void SetRGBColor (DaisyWhite::LeddriverLeds idx, int color)
{
    
    switch (color)
    {
    case red:
        white.SetRgbLeds(idx,1.f,0.f,0.f);
        break;
    case green:
        white.SetRgbLeds(idx,0.f,0.7f,0.f);
        break;
    case blue:
        white.SetRgbLeds(idx,0.f,0.f,1.f);
        break;
    case yellow:
        white.SetRgbLeds(idx,1.f,0.7f,0.f);
        break;
    case cyan:
        white.SetRgbLeds(idx,0.f,1.f,1.f);
        break;
    case magenta:
        white.SetRgbLeds(idx,1.f,0.f,1.f);
        break;
    case orange:
        white.SetRgbLeds(idx,1.f,0.3f,0.f);
        break;
    case darkgreen:
        white.SetRgbLeds(idx,0.f,0.2f,0.f);
        break;
    case darkblue:
        white.SetRgbLeds(idx,0.2f,0.2f,0.6f);
        break;
    case darkred:
        white.SetRgbLeds(idx,0.4f,0.f,0.f);
        break;
    case turq:
        white.SetRgbLeds(idx,0.f,0.5f,0.5f);
        break;
    case grey:
        white.SetRgbLeds(idx,0.75f,0.75f,0.75f);
        break;
    case darkorange:
        white.SetRgbLeds(idx,0.5f,0.2f,0.f);
        break;
    case white_:
        white.SetRgbLeds(idx,1.f,1.f,1.f);
        break;
    
    }
    
}