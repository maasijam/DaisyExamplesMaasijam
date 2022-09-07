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
 */

#include <algorithm>
#include <string> 
#include <vector>

#include "theory.h"

namespace ev_theory {
    uint8_t quantizeNoteToRange(int8_t i) {
        while (i > MAX_NOTE){
            i -= SEMIS_PER_OCT;
        }
        while (i < MIN_NOTE){
            i += SEMIS_PER_OCT;
        }
        return i;
    }
    
    // Helper
    uint8_t cScaleEquivalent(uint8_t note, uint8_t modeRoot){
        return (note + SEMIS_PER_OCT - modeRoot) % SEMIS_PER_OCT;
    }

    uint8_t quantize(uint8_t note, int mode, uint8_t modeRoot){
        int base = cScaleEquivalent(note, modeRoot);

        int modesemisize = modesemi[mode].num_notes;
         
        for (uint8_t i = 0; i < modesemisize; i++){
            if(modesemi[mode].notes[i] == base) {
                // If the note is diatonic, return it immediately
                return note;
            }
        }

        // If the note isn't diatonic, quantize it. Move 1 semitone down if possible,
        // otherwise 1 up.
        //
        // NOTE: This relies on the property that every non-diatonic note is 1 semitone
        // away from a diatonic note. I'll need to rework this if I implement exotic 
        // scales with 3+ semitone distances.
        if (note == MIN_NOTE){
            note++;
        } else {
            note--;
        }
        
        return note;
    };

    uint8_t noteToScaleDegree(uint8_t note, int mode, uint8_t modeRoot){
        note = quantize(note, mode, modeRoot) % SEMIS_PER_OCT;
        int base = cScaleEquivalent(note, modeRoot);
        
        //const std::vector<int> v = modeToSemitones.at(mode);
        int modesemisize = modesemi[mode].num_notes;
        for(uint8_t i = 0; i < modesemisize; i++){
            if (modesemi[mode].notes[i] == base){
                return i;
            } 
        }

        // If you've reached this line, there's a bug
        return 1; 
    }

    uint8_t scaleDegreeToNote(uint8_t degree, int mode, uint8_t modeRoot){
        //const std::vector<int>* v = &(modeToSemitones.at(mode));
        //int modesemisize* = &modesemi[mode].num_notes;
        //int base = v->at(degree);
        int base = modesemi[mode].notes[degree];
        return (base + modeRoot) % SEMIS_PER_OCT; 
    }

    int16_t semitoneToDac(int16_t semi) {
        return round(DAC_UNITS_PER_SEMI * semi); 
    }

    int16_t centsToDac(int16_t cents) {    
        return round(DAC_UNITS_PER_SEMI * cents * .01f); 
    }

    uint16_t prepareDacValForOutput(int16_t i){
        while (i < MIN_DAC_VALUE) {
            i += DAC_UNITS_PER_OCT;
        }
        if (i == 0) {
            return 0;
        }

        i += DAC_OFFSET_FOR_NONZERO_VALUES;
        while (i > MAX_DAC_VALUE) {
            i -= DAC_UNITS_PER_OCT;
        }
        return i;
    };

    // FIXME doesn't render 0
    // Used for debug display
    std::string floatToString(float f, uint8_t dec){
        if (f == 0.) {
            return "0";
        }
        for (uint8_t i = 0; i < dec; i++) {
            f = f * 10.;
        }
        std::string ret = std::to_string(static_cast<int>(round(f)));
        ret.insert(ret.end()-dec, 1, '.');
        return ret;
    }
} // namespace ev_theory