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
#include "daisy_patch.h"
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
using namespace jellybeans;
using namespace patch_gui;
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

// State objects
DaisyPatch* patch;
PatchGui*   gui;
Arp*        arp;
Menu*       menu;
Rhythm*     rhythm;

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

FontDef font       = Font_7x10;
uint8_t fontWidth  = 7;
uint8_t fontHeight = 10;

void updateControls();
void updateOled();
void updateOutputs();

// Copy the values in settings_ram (volatile) to settings_qspi (non-volatile)
void saveSettingsToQSPI(){
    size_t size = sizeof(settings_qspi);
    size_t address = (size_t)settings_qspi;
    patch->seed.qspi.Erase(address, address + size);
    patch->seed.qspi.Write(address, size, (uint8_t*)settings_ram);
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
    settings_ram[RAM_BPM]        = 120;
    settings_ram[RAM_CLOCK_MODE] = 0;
    settings_ram[RAM_OP_MODE]    = 0;
    settings_ram[RAM_IN_TUNE]    = 0 - MIN_IN_TUNE;
    settings_ram[RAM_ARP_TUNE]   = 0 - MIN_OUT_TUNE;
    settings_ram[RAM_BASS_TUNE]  = 0 - MIN_OUT_TUNE;
    settings_ram[RAM_IS_INIT]    = 42;
    saveSettingsToQSPI();
}

// Compute a new bass CV value
void updateBassNote(){
    int semi = arp->getChord()->getRoot();
    semi += SEMIS_PER_OCT * menu->getItem("Bass Oct")->getIndex();
    bassDac = semitoneToDac(semi);
};

/* Callback functions invoked whenever menu parameters are changed */

void cbPattern(){
    arp->setPattern(menu->getItem("Pattern")->getValue());  // TODO make the titles of each menuItem const
};

void cbVoicing(){
    arp->getChord()->setVoicing(menu->getItem("Voicing")->getValue());
    arp->updateTraversal();
};

void cbInversion(){
    arp->getChord()->setInversion(menu->getItem("Inversion")->getIndex());
    arp->updateTraversal();

};

void cbMode(){
    settings_ram[RAM_MODE] = menu->getItem("Mode")->getIndex();
    saveSettingsToQSPI();

    arp->getChord()->setMode(menu->getItem("Mode")->getValue());
    arp->updateTraversal();
    updateBassNote();
};

void cbRoot(){
    settings_ram[RAM_ROOT] = menu->getItem("Root")->getIndex();
    saveSettingsToQSPI();

    arp->getChord()->setModeRoot(settings_ram[RAM_ROOT]);
    arp->updateTraversal();
    updateBassNote();
};

void cbArpOct(){
    settings_ram[RAM_ARP_OCT] = menu->getItem("Arp Oct")->getIndex();
    saveSettingsToQSPI();

    arpOctMod = semitoneToDac(settings_ram[RAM_ARP_OCT] * SEMIS_PER_OCT);
};

void cbBassOct(){
    settings_ram[RAM_BASS_OCT] = menu->getItem("Bass Oct")->getIndex();
    saveSettingsToQSPI();

    bassOctMod = semitoneToDac(settings_ram[RAM_BASS_OCT] * SEMIS_PER_OCT);
};

void cbNoteIn(){
    arp->getChord()->setDegreeByNote(menu->getItem("Note In")->getIndex());
    arp->updateTraversal();
    updateBassNote();
};

void cbClockDiv(){
    settings_ram[RAM_CLOCK_DIV] = menu->getItem("Clock Div")->getIndex();
    saveSettingsToQSPI();

    if(settings_ram[RAM_CLOCK_MODE] == 0){
        // Internal: timing is determined by clock divisions
        divMax = clockDivTo256ths.at((menu->getItem("Clock Div")->getValue()));
    } else {
        // External: timing is determined by pulses per note.
        // Fractional clock values will just be set to 1.
        divMax = std::max(clockDivTo256ths.at(menu->getItem("Clock Div")->getValue())/256, 1);
    }
    divCounter = 0;
};

void cbClockMode(){
    settings_ram[RAM_CLOCK_MODE] = menu->getItem("Clock")->getIndex();
    rhythm->setClock(settings_ram[RAM_CLOCK_MODE] == 0);
    cbClockDiv();
    // cbClockDiv() already calls saveSettingsToQSPI(), so no need to call it in this function.
};

void cbOpMode(){
    settings_ram[RAM_OP_MODE] = menu->getItem("Op Mode")->getIndex();
    saveSettingsToQSPI();
};

void cbBPM(){
    settings_ram[RAM_BPM] = menu->getItem("BPM")->getIndex();
    saveSettingsToQSPI();

    rhythm->setBPM(settings_ram[RAM_BPM]);
}

void cbInTune(){
    inTune = menu->getItem("In Tune")->getIndex();

    // Since settings must be uints, need to add an offset to ensure 
    // the vlue written is > 0. 
    settings_ram[RAM_IN_TUNE] = inTune - MIN_IN_TUNE;
    saveSettingsToQSPI();
}

void cbArpOutTune(){
    arpOutTune = centsToDac(menu->getItem("Arp Tune")->getIndex());

    settings_ram[RAM_ARP_TUNE] = menu->getItem("Arp Tune")->getIndex() - MIN_OUT_TUNE;
    saveSettingsToQSPI();
}

void cbBassOutTune(){
    bassOutTune = centsToDac(menu->getItem("Bass Tune")->getIndex());

    settings_ram[RAM_BASS_TUNE] = menu->getItem("Bass Tune")->getIndex() - MIN_OUT_TUNE;
    saveSettingsToQSPI();
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
    patch = new DaisyPatch();
    patch->Init();
    arp   = new Arp();
    menu  = new Menu();

    gui = new PatchGui(patch, menu, &font, fontWidth, fontHeight, 4);
    rhythm = new Rhythm(true, cbRhythm);

    loadQSPISettingsToRAM();

    // If the RAM_IS_INIT setting isn't 42, we know that Jellybeans doesn't have settings
    // here and we should use default settings.
    if (settings_ram[RAM_IS_INIT] != 42){
        saveDefaultSettingsToQSPI();
    } 

    if (!DEBUG_MODE){
        // Boot screen gets annoying during development
        gui->drawStartupScreen("Jellybeans", VERSION, 1500);
    }

    bassDac    = 0.f;
    lastNote   = 0;
    divMax     = clockDivTo256ths.at("1/4");
    divCounter = 0;
    blink      = 0;
    arpOctMod  = 0.f;
    bassOctMod = 0.f;

    // Initialize menu items
    menu->append("Pattern", "   ", arpPatterns,                      0,                                          cbPattern);
    menu->append("Voicing", "   ", voicings,                         0,                                          cbVoicing);
    menu->append("Inversion", " ", allInversions,                    0,                                          cbInversion);
    menu->append("Clock Div", " ", clockDivs,                        settings_ram[RAM_CLOCK_DIV],                cbClockDiv);
    menu->append("Arp Oct", "   ", MIN_OCT_MOD,     MAX_OCT_MOD,     settings_ram[RAM_ARP_OCT],                  cbArpOct);
    menu->append("Root", "      ", allNotes,                         settings_ram[RAM_ROOT],                     cbRoot);
    menu->append("Mode", "      ", modes,                            settings_ram[RAM_MODE],                     cbMode);
    menu->append("Bass Oct", "  ", MIN_OCT_MOD,     MAX_OCT_MOD,     settings_ram[RAM_BASS_OCT],                 cbBassOct);
    menu->append("BPM", "       ", Rhythm::MIN_BPM, Rhythm::MAX_BPM, settings_ram[RAM_BPM],                      cbBPM);
    menu->append("Clock", "     ", clockModes,                       settings_ram[RAM_CLOCK_MODE],               cbClockMode);
    menu->append("Op Mode", "   ", opModes,                          settings_ram[RAM_OP_MODE],                  cbOpMode);
    menu->append("Note In", "   ", allNotes5Oct,                     0,                                          cbNoteIn);
    menu->append("In Tune", "   ", MIN_IN_TUNE,     MAX_IN_TUNE,     settings_ram[RAM_IN_TUNE] + MIN_IN_TUNE,    cbInTune);
    menu->append("Arp Tune", "  ", MIN_OUT_TUNE,    MAX_OUT_TUNE,    settings_ram[RAM_ARP_TUNE] + MIN_OUT_TUNE,  cbArpOutTune);
    menu->append("Bass Tune", " ", MIN_OUT_TUNE,    MAX_OUT_TUNE,    settings_ram[RAM_BASS_TUNE] + MIN_OUT_TUNE, cbBassOutTune);
    
    // Initialize CV params
    gui->assignToCV("Pattern",   1);
    gui->assignToCV("Voicing",   2);
    gui->assignToCV("Inversion", 3);

    // "Prime" menu items
    cbBPM();
    cbInTune();
    cbArpOutTune();
    cbBassOutTune();
    cbMode();
    cbRoot();
    cbClockMode(); // NOTE: cvClockMode() calls cbClockDiv()


    // "In case if you wondered, the fucking thing starts the circular DMA transfer
    // that receives ADC readings from knobs / CV inputs."
    //
    // Thanks, antisvin :P
    patch->StartAdc();

    // Main event loop
    while(1){
        updateControls();
        updateOled();
        updateOutputs();
        rhythm->update();
    }
}

// Handle any input to Patches' hardware
void updateControls() {
    patch->ProcessAnalogControls();
    patch->ProcessDigitalControls();

    // Read v/oct from CTRL 4
    float ctrl = patch->GetKnobValue((DaisyPatch::Ctrl)3);
    uint8_t i = static_cast<uint8_t>(std::round(ctrl*60.f) + inTune);
    //i = quantizeNoteToRange(i); // TODO this may be redundant

    // Check that a new cv value has been input, otherwise encoder input to
    // note in won't work. Might remove later and just let CTRL 4 handle it
    if(i !=  lastNote){
        lastNote = i;
        menu->getItem("Note In")->setIndex(i);
    }
    
    // GATE IN 1 -> clock pulse
    blink--;
    if(patch->gate_input[0].Trig()){
        blink = BLINK_FRAMES;
        rhythm->pulse();
    }
}

void updateOutputs()
{
    uint16_t dac1 = 0;
    uint16_t dac2 = 0;
    bool gate1 = false;
    
    switch (settings_ram[RAM_OP_MODE]){
        case 0:
            // Arp CV -> CV OUT 1
            dac1 = arp->getDacValue() + arpOutTune + arpOctMod;
            // Bass CV -> CV OUT 2
            dac2 = bassDac + bassOutTune + bassOctMod;
            // Arp Gate -> GATE OUT 1
            gate1 = arp->getTrig();
            break;

        case 1:
            dac1 = bassDac + arpOutTune + arpOctMod;
            dac2 = bassDac + bassOutTune + bassOctMod;
            break;
    } 

    dsy_gpio_write(&patch->gate_output, gate1);
    patch->seed.dac.WriteValue(DacHandle::Channel::ONE, prepareDacValForOutput(dac1));
    patch->seed.dac.WriteValue(DacHandle::Channel::TWO, prepareDacValForOutput(dac2));
}

void updateOled(){
    gui->updateControls();

    // Display a blinking '.' on every clock pulse in
    std::string h2 = "";
    if (blink > 0) {
        h2 += ".";
    }
    h2 += rhythm->bpmToString();

    // Set headers
    gui->setHeader(arp->toString(), 0);
    gui->setHeader(h2, 1);
    gui->setHeader(arp->getChord()->toString(), 2);
//    gui->setHeader(allNotes5Oct.at(lastNote), 3);

    // Keeping a few useful debug outputs here in case I need them later
    if (DEBUG_MODE){
        // gui->setHeader("CV1: " + std::to_string(static_cast<int>(bassDac)) + 
        //        " CV2: " + std::to_string(static_cast<int>(arp->getDacValue())), 2);
        //gui->setHeader(rhythm->toString(), 2); 
        //gui->setHeader(std::to_string(divCounter) + " " + std::to_string(divMax), 2); 
        //gui->setHeader(std::to_string(divCounter) + " " + std::to_string(divMax), 2); 
       // gui->setHeader(std::to_string(menu->getItem("BPM")->getIndex()), 2);
      // gui->setHeader(std::to_string(arp->getChord()->getOctave()), 2);
    }

    gui->render();
}