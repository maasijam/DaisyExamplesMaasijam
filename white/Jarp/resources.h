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

#include "lib/ev_theory/theory.h"
#include "lib/ev_theory/chord.h"
#include "lib/ev_theory/arp.h"
#include "lib/ev_theory/rhythm.h"
//#include "lib/ev_gui/gui.h"

/* Menu values specific to Jellybeans */
namespace jarp {

    // Order must match order of OpMode
    const std::vector<std::string> opModes {
        "Arp",
        "Quant",
    };

    const std::vector<std::string> clockModes {
        "BPM",
        "PerTrig",
    };

    const std::vector<std::string> clockDivs {
        "1/128",
        "1/64",
        "1/32",
        "1/16",
        "1/8",
        "1/4",
        "1/2", 
        "1",
        "2",
        "4",
        "8",
        "16",
        "32",
        "64",
        "128",
    };


    uint16_t clockDivTo256ths2[15] = {32768,16384,8192,4096,2048,1024,512,256,128,62,32,16,8,4,2};
    int16_t allOctaves2[3] = {-1,0,1};

    // Given the 1V/oct and 0-5V range of the CV out port,
    // we are limited to a 5 octave register. Voicings span
    // up to 2 octaves and coarse tuning (mRoot) spans another,
    // leaving us 2 octaves of room for upwards transposition.
    //
    // Note that the indices of the elements are also their octave distances from 0
    const std::vector<std::string> allOctaves {
        "-3",
        "-2",
        "-1",
        "0",
        "+1",
        "+2",
        "+3",
    };

    enum
    {
        S1      = 15,
        S2      = 14,
        S3      = 13,
        S4      = 12,
        S5      = 11,
        S6      = 10,
        S7      = 9,
        S8      = 8,
        S0A     = 7,
        S0B     = 6,
        S1A     = 5,
        S1B     = 4,
    };

    enum rgbcolor
    {
        red, // 1,0,0
        green, // 0,1,0
        blue, // 0,0,1
        yellow, // 1,1,0
        cyan, // 0,1,1
        magenta, // 1,0,1
        orange, // 1,0.4,0
        darkgreen, // 0,0.5,0
        darkblue, // 0.2,0.2,0.6
        darkred, // 0.5,0,0
        turq, // 0,0.5,0.5
        grey, // 0.5,0.5,0.5
        darkorange, // 1,1,0.8
        white_// 1,1,1

    };
}