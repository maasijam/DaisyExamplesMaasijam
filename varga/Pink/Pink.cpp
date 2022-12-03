#include "../daisy_varga.h"
#include "daisysp.h"
#include "dsp/voice.h"

using namespace daisy;
using namespace daisysp;

DaisyVarga hw;

plaits::Voice voice = {};
plaits::Patch patch = {};
plaits::Modulations modulations = {};
char shared_buffer[16384] = {};

#define BLOCK_SIZE 16
plaits::Voice::Frame outputPlaits[BLOCK_SIZE];



void updateSwitches();
void updateLeds();
void LoadPinkSettings();
void SavePinkSettings();


bool trigger_save;
//bool saveLed;

//uint32_t startTime;
//uint32_t curTime;

struct PinkSettings
{
	PinkSettings() : engine(0) {}
	int engine;
	
    bool operator==(const PinkSettings &rhs)
    {
        if(engine != rhs.engine)
        {
            return false;
        }
        
        return true;
    }

    
    bool operator!=(const PinkSettings &rhs) { return !operator==(rhs); }
};



void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size) {

	//button.Debounce();
	//toggle.Debounce();

	hw.ProcessAllControls();
	updateSwitches();

	


	float pitch = hw.GetKnobValue(hw.KNOB_0) * 8.f - 4.f;

	patch.note = 60.f + pitch * 12.f;
	patch.harmonics = hw.GetKnobValue(hw.KNOB_1);
	patch.timbre = hw.GetKnobValue(hw.KNOB_2);
	patch.morph = hw.GetKnobValue(hw.KNOB_3);
	patch.lpg_colour = 0.5f;
	patch.decay = 0.5f;
	patch.frequency_modulation_amount = 0.f;
	patch.timbre_modulation_amount = 2.f;
	patch.morph_modulation_amount = 0.f;

	float voct_cv = hw.GetCvValue(hw.CV_3);
	float voct    = fmap(voct_cv, 0.f, 60.f);
	modulations.note = voct;

	// todo: work out how modulations.timbre_patched etc can work
	modulations.frequency = hw.GetCvValue(hw.CV_0);
	modulations.harmonics = hw.GetCvValue(hw.CV_4);
	modulations.timbre = hw.GetCvValue(hw.CV_1);
	modulations.morph = hw.GetCvValue(hw.CV_2);
	modulations.engine = hw.GetCvValue(hw.CV_5);
	

	modulations.frequency_patched = true;
	modulations.timbre_patched = true;
	modulations.morph_patched =  true;
	//modulations.level_patched = patched_ml == 2 || patched_ml == 3 ? true : false;


	//if (!hw.SwitchState(S0A)) {
		modulations.trigger = 5.f * hw.gate.State();
		modulations.trigger_patched = true;
	//}
	//else {
	//	modulations.trigger = 0.f;
	//	modulations.trigger_patched = false;
	//}

	voice.Render(patch, modulations, outputPlaits, BLOCK_SIZE);

	for (size_t i = 0; i < size; i++) {
		OUT_L[i] = outputPlaits[i].out / 32768.f;
		OUT_R[i] = outputPlaits[i].aux / 32768.f;
	}
}


int main(void) {
	
	hw.Init();
	hw.SetAudioBlockSize(BLOCK_SIZE); // number of samples handled per callback
	hw.SetAudioSampleRate(SaiHandle::Config::SampleRate::SAI_48KHZ);

	

	patch.engine = 0;
	modulations.engine = 0;

	hw.ClearLeds();
	//hw.SetRGBColor(hw.RGB_LED_1,hw.cyan);
	hw.UpdateLeds();

	stmlib::BufferAllocator allocator(shared_buffer, sizeof(shared_buffer));
	voice.Init(&allocator);
    hw.StartAdc();
	hw.StartAudio(AudioCallback);

	//trigger_save = false;
	//saveLed = false;
	
	LoadPinkSettings();

	while (1) {
		
		updateLeds();
		if(trigger_save)
        {
            SavePinkSettings();
			//startTime = System::GetNow();
			
			trigger_save = false;
			//saveLed = true;
			            
        }
		//if(saveLed) {
			//curTime = System::GetNow();
			//if((System::GetNow() - startTime) > 1000) {
			//	saveLed = false;
			//}
		//}
	}
}


void updateSwitches() {
	if (hw.s[hw.S_1].RisingEdge()) {
          //RealignPots();
          if (patch.engine >= 8) {
            patch.engine = patch.engine & 7;
          } else {
            patch.engine = (patch.engine + 1) % 8;
          }
          trigger_save = true;
        }
  
        if (hw.s[hw.S_0].RisingEdge()) {
          //RealignPots();
          if (patch.engine < 8) {
            patch.engine = (patch.engine & 7) + 8;
          } else {
            patch.engine = 8 + ((patch.engine + 1) % 8);
          }
          trigger_save = true;
        }
/*
		if (hw.SwitchRisingEdge(S2)) {
          patched_fmt++;
          if (patched_fmt > 3) {
            patched_fmt = 0;
          } 
		  trigger_save = true;
        }

		if (hw.SwitchRisingEdge(S3)) {
          patched_ml++;
          if (patched_ml > 3) {
            patched_ml = 0;
          } 
		  trigger_save = true;
        }

		if (hw.SwitchRisingEdge(S8)) {
			//trigger_save = true;
		}
	*/
}

void updateLeds() {
	hw.ClearLeds();
	
	hw.SetRGBColor((patch.engine & 8 ? hw.LED_0 : hw.LED_1),DaisyVarga::Colors (patch.engine & 7));

	hw.UpdateLeds();
}

/** @brief Sets the calibration data for 1V/Octave over Warp CV 
 *  typically set after reading stored data from external memory.
 */
inline void SetPatchedSettings(int engine)
{
	patch.engine = engine;
}

static constexpr uint32_t kSettingsDataOffset = 1024;

/** Get the scale and offset data from the calibration 
 *  \retval returns true if calibration has been performed.
*/
inline void GetPatchedSettings(int &engine)
{
	engine = patch.engine;
}


void LoadPinkSettings()
{
	daisy::PersistentStorage<PinkSettings> settings_storage(hw.seed.qspi);
	PinkSettings                           default_settings;
	settings_storage.Init(default_settings, kSettingsDataOffset);
	auto &pink_settings = settings_storage.GetSettings();
	SetPatchedSettings(pink_settings.engine);
}

void SavePinkSettings()
{
	daisy::PersistentStorage<PinkSettings> settings_storage(hw.seed.qspi);
	PinkSettings                           default_settings;
	settings_storage.Init(default_settings, kSettingsDataOffset);
	auto &pink_settings = settings_storage.GetSettings();
	GetPatchedSettings(pink_settings.engine);
	settings_storage.Save();
}
