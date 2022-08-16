# :candy: Jellybeans :candy:
Quantizing diatonic arpeggiator for the Electrosmith Daisy Patch eurorack module.

### Demo video
[![Click here for the demo video](https://img.youtube.com/vi/7cRScZM9cCY/0.jpg)](https://www.youtube.com/watch?v=7cRScZM9cCY&ab_channel=Evan%27sUniverse)

### What does it do?
Jellybeans is an arpeggiator with special features. The user configures a scale and sends in v/oct root input, then Jellybeans quantizes and arpeggiates the diatonic chord with that root. This is great for approximating chord progressions with eurorack's chiefly monophonic sound sources.

### Features
* Arpeggiator
* Configurable scale (root and quality)
* Quantizes v/oct input
* Lots of arpeggator options (chord voicings, inversions, clock divisions)
* Sends a bass note (root) out through _CV out 2_
* Inspired by piano chord voicings, WMD Arpitecht, and the Access Virus TI2's arpeggiator

### Tips and tricks
* Send a note CV to CTRL 4 and recieve from CV OUT 2 to use Jellybeans as a plain 'ol quantizer
* If notes from the arp would go out of range, it will transpose them back into range in a musically pleasing way. Play with those high octaves to get some interesting results!

## Build Instructions

<details>
<summary><b>"I'm not a developer, I just want to use the darn program!" (Daisy web programmer)</b></summary>

Note: If this is your first time installing anything on your Daisy, I recommend that you start by installing blink [(official instructions)](https://forum.electro-smith.com/t/welcome-to-daisy-get-started-here/15) as a way to learn the Daisy web programmer.

From the [Daisy web programmer](https://electro-smith.github.io/Programmer/):
1. With the eurorack module's power turned off, connect Daisy Seed via USB. If you're re-building, you'll need to briefly disconnect and then reconnect to reset things.
2. Put Daisy into bootloader mode by holding the BOOT button down, and then pressing the RESET button. Once you release the RESET button, you can also let go of the BOOT button. 
3. Download [build/Jellybeans.bin](build/Jellybeans.bin)
4. In the web programmer, click "connect". Select "DFU in FS mode"
6. Click "choose file" and select `Jellybeans.bin`
7. Click "program"

I've tested this in Chrome, compatibility with other browsers may vary.

</details>


<details>
<summary><b>Debug probe from vscode (recommended for devs)</b></summary>

From a vscode instance in this project's root directory:
1. Connect the [ST LINK-V3 debug probe](https://www.electro-smith.com/daisy/stlink-v3mini) to Daisy
2. With the eurorack module's power turned on, connect Daisy Seed via USB.
3. Hit ctrl+p to bring up the command palette 
4. Type `task build_and_program` and hit enter

</details>

<details>
<summary><b>Direct USB -> Daisy connection from command line</b></summary>

From this project's root directory:
1. With the eurorack module's power turned off, connect Daisy Seed via USB. If you're re-building, you'll need to briefly disconnect and then reconnect to reset things.
2. Put Daisy into bootloader mode by holding the BOOT button down, and then pressing the RESET button. Once you release the RESET button, you can also let go of the BOOT button. 
3. `make program-dfu`
4. Disconnect USB and power on the module.

</details>

<details>
<summary><b>Direct USB -> Daisy connection from vscode</b></summary>

From a vscode instance in this project's root directory:
From this project's root directory:
1. With the eurorack module's power turned off, connect Daisy Seed via USB. If you're re-building, you'll need to briefly disconnect and then reconnect to reset things.
2. Put Daisy into bootloader mode by holding the BOOT button down, and then pressing the RESET button. Once you release the RESET button, you can also let go of the BOOT button. 
3. Hit ctrl+p to bring up the command palette 
4. Type `task build_and_program_dfu` and hit enter
5. Disconnect USB and power on the module

</details>

<details>
<summary><b> Check compilation with no Daisy </b></summary>
From this project's root directory: `make`

</details>

Note that you might have to run additional tasks to re-build the binary. For additional resources: [Official Daisy documentation](https://github.com/electro-smith/DaisyWiki/wiki/1.-Setting-Up-Your-Development-Environment)

## Usage Instructions
### TL;DR
* Use `ENC 1` to navigate the menu (push down to toggle between scrolling and editing)
* Set the note in with the menu or send v/oct to `CTRL 4`
* Set BPM in the menu or send quarter note tempo to `GATE IN 1`
* The arpeggiator will send v/oct to `OUT 1`, triggers to `GATE OUT 1` and bass v/oct to `OUT 2`

### Controls
* **Gate in 1:** Trigger in 
<!--* **Gate in 2:**-->
* **Gate out 1:** Trigger out
* **Ctrl 1:** Pattern
* **Ctrl 2:** Voicing
* **Ctrl 3:** Inversion
* **Ctrl 4:** v/oct
* **Enc 1 (push):** Select alter/browse menu item 
* **Enc 1 (rotate):** Alter/browse menu item
<!--* **Audio in 1:**
* **Audio in 2:**
* **Audio in 3:**
* **Audio in 4:**
* **Audio out 1:**
* **Audio out 2:**
* **Audio out 3:**
* **Audio out 4:**
* **MIDI in:** 
* **MIDI out:** -->
* **CV out 1:** Arpeggio v/oct
* **CV out 2:** Bass v/oct

### Menu
Organized as a vertical scrollable menu. Rotating the encoder scrolls to the next menu item. Pushing encoder in highlights the currently selected item, in which case rotating it edits the value. Items marked with a '(p)' are persisted when the device is powered off.

* **Pattern** (_Ctrl 1)_: Pattern the arp will play notes in
    * Up
    * Down
    * Up + down inclusive (plays the last note on the way up and the way down e.g. C->E->G->G->E->C)
    * Up + down exclusive (play the last note only on the way up e.g. C->E->G->E->C)
    * Random
* **Voicing** (_Ctrl 2)_
    * Triad
    * Triad+ (triad + root)
    * 7th
    * Sus2
    * Sus4
    * Power (root+5)
    * Shell 1
    * Shell 2
    <!--* Kenny Barron-->
* **Inversion** (_Ctrl 3)_
    * None
    * 1st
    * 2nd
    * 3rd
* **Click Division** (p): Division that the arp will play notes at
    * 1/128
    * 1/64
    * 1/32
    * 1/16
    * 1/8
    * 1/4
    * 1/2
    * 1
    * 2
    * 3
    * 4
    * 5
    * 6
    * 7
    * 8
    * 16
    * 32
    * 64
    * 128
    <!--* 4 bars
    * 2 bars
    * 1 bar
    * Half
    * 4th
    * 8th
    * 16th
    * 32nd
    * 64th (maaaybe) -->
<!--* Rhythm
    * Straight
    * Dotted
    * Swing 25%
    * Swing 50%
    * Swing 75%
    * Swing 100%
    * A number of fun virus-like patterns
    * Trig in (Plays a note when something is sent to trig in. Changes notes for every new trigger)-->
* **Arp Oct** (p): Transposes arp output by x octaves
    * -4 to +4
* **Bass Oct** (p): Transposes bass output by x octaves
    * -4 to +4
* **Root** (p): Root note of the quantizer's scale
    * Any note
* **Mode** (p): Mode of the quantizer's scale
    * Major
    * Dorian
    * Phyrgian
    * Lydian
    * Mixolydian
    * Minor
    * Locrian
* **BPM** (p): BPM of the internal clock (overriden by input to GATE IN 1)
    * 20-500
* **Clock** (p): Clock mode
    * BPM: Keep an internal clock. If clock is coming in through GATE IN 1, it will detect tempo. Otherwise BPM can be adjusted in the menu.
    * PerTrig: Don't use a clock, instead advance the arp by 1 step each time a trigger is revieved through GATE IN 1.
* **Op Mode** (p): Mode of operation
    * Arp: Standard mode
    * Quant: Disable the arp completely; act as a quantizer. The incoming note from CTRL 4 will be quantized and sent out throuhg CV OUT 1.
* **Note in** (_Ctrl 4)_: The note being fed to the quantizer
    * Any note in a 5 octave register C0-B4
    * Can select a value or supply volt/octave
* **In Tune** (p): alters incoming notes by +/- 12 semitones
    * -12 to 12
* **Arp Tune** (p): alters outgoing arp notes by a +/- 100 cents
    * -100 to 100
* **Bass Tune** (p): alters outgoing bass notes by a +/- 100 cents
    * -100 to 100


<!--
* Arp octave range (-2 to +4)
* Bass note division
    * Hold (this will play 1 sustained note until the next chord change)
    * 4 bars
    * 2 bars
    * 1 bar
    * Half
    * 4th
    * 8th
    * 16th
    * 32nd
* Bass Rhythm
    * Straight
    * Dotted
    * Swing 25%
    * Swing 50%
    * Swing 75%
    * Swing 100%
    * A number of fun virus-like patterns designed specifically for bass grooves
    * Trig in (Plays a note when something is sent to trig in. Changes notes for every new trigger)
* Bass Order
    * Up
    * Down
    * Up+down inc.
    * Up+down exc.
    * A number of fun virus-like patterns designed specifically for bass
* Bass Voicing
    * Root only
    * Melody (same chord type as arp)
    * Power (root+5)
    * Walk 1 (root+2)
    * Walk 2 (root+3)
    * Standard (triad)
    * Might include the same options as melody but maybe not. Issue would be option amount on a physical knob
* Clock in rate
    * 1
    * ½
    * ¼
    * ⅛
    * 1/16 -->

## TODOs
### Definitely planned for the future
* more elementary rhythmic variations such as 1->5->3->7
* varying rhythms, similar to the Access Virus TI2's arp (implemented as a StepSequencer class)
* user can select which cv input maps to which parameter
* MIDI support (MIDI output and clock in)
* Move QSPI save/load stuff to the gui lib

### Maybe, if I have the time
* automate the creation of version tagged .bin files
* use persistent_storage isntead of qspi https://github.com/electro-smith/libDaisy/pull/396
* Display flats instead of sharps when appropriate
* multi-octave arpeggiation
* Add some sort of "inversion+oct displacement" setting
* slides (per-step portamento)
* Add other piano-inspired patterns/features e.g. adding a low root+5th or low root or high root. Maybe add  a "reinforce" option that adds these things without having to have separate "7th+oct" voicings
* Separate diagnostics page that can monitor perf data on-unit
* seprate menus implemented in gui lib - maybe double clicking the encoder cycles menus
* give bass more options (possibly implement as 2nd arp)
* automated semver
* changing clock divs maintains timing
* add a "pick up where you left off" option to the arp so that it doesn't restart at the root change voicing, etc
* varying fonts on startup screen. add a drawWithFont() function to gui lib
    * maybe not, since flash space is at a premium and importing a font is large. ill have to test this and see how much flash it costs.
* minecraft-esque fun message on startup screen
    * again, barring space concerns. maybe could read these from a file
* figure out how to make the binary smaller so i can fit more features
* quantizer + offset mode https://forum.electro-smith.com/t/jellybeans-diatonic-quantizing-arpeggiator-for-daisy-patch/2178/2?u=evan
* alternate mode where the user selects a scale degree per note and can noodle with them similar to elektron arps

## Known Bugs
* CV values jitter when knob is stuck between 2 values. Need to implement hysteresis https://forum.electro-smith.com/t/petal-potentiometer-noise/1797/2?u=evan
* Trying to set the 3r header in ev_gui::PatchGui causes a crash on startup

## Contributing
See [CONTRIBUTING.md](../CONTRIBUTING.md)

## Contact me
See [CONTACT.md](../CONTACT_ME.md)

## License
Author: Evan Pernu

GNU GPL 3.0 - see [../LICENSE.md](../LICENSE.md).