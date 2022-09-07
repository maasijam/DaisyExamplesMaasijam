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

#include <string>
#include <vector>

#include "../../Jarp.h"
#include "chord.h"

using namespace daisy;

namespace ev_theory {
    class Arp {
    private:
        // Maximum arp steps
        int maxSteps;

        // Tracks the current position in arpTraversal
        int traversalIndex;
        int traversalLedIndex;

        // Current step index, 0 based
        int step;

        // If true, a new note is being played this frame and a trigger
        // should be sent out.
        bool trig; 

        // The CV value currently being sent to Patches' DAC's output 1
        // This is stored so that it's only calculated when necessary
        float dacValue;


        // Stores which arp note should be played at each next step.
        // Contains a list of indices of arpNotes
        std::vector<int> traversal;

        // Must be an element of mu::arpPatterns
        int pattern;

        // String representation of the arp
        std::string string;

        // Underlying chord
        DiatonicChord* chord;

        void resetState();
    public:
        Arp();

        // @param maxSteps
        // @param chord
        // @param pattern - ∈ arpPatterns
        Arp(int, int);

        // Intended to be called every time a clock pulse is received
        void onClockPulse();
    
        // Updates the arp traversal values based on the current pattern
        void updateTraversal();

        // Called every time the arp steps to the next note
        void updateStep();

        void updateString();

        std::string toString();

        // Returns true if there's a new note trigger. Also resets the 
        // trigger bool, effectively telling the arp "I've consumed the trigger"
        bool getTrig();

        float getDacValue();

        int getTraversalIndex();

        DiatonicChord* getChord();

        // @param ∈ arpPatterns
        void setPattern(int);
    };

    
    enum arpPatterns {
        Up,
        Down,
        UDIn,
        UDEx,
        UpD,
        DownD,
        Random
    };
} // namespace ev_theory