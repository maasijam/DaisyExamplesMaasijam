#include "../daisy_white.h"
#include "daisysp.h"
#include "dsp/voice.h"

using namespace daisy;
using namespace daisysp;

DaisyWhite hw;

plaits::Voice voice = {};
plaits::Patch patch = {};
plaits::Modulations modulations = {};
char shared_buffer[16384] = {};

#define BLOCK_SIZE 16
plaits::Voice::Frame outputPlaits[BLOCK_SIZE];


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

void updateSwitches();
void updateLeds();
void LoadPinkSettings();
void SavePinkSettings();

int patched_fmt = 0;
int patched_ml = 0;

bool trigger_save;
//bool saveLed;

//uint32_t startTime;
//uint32_t curTime;

struct PinkSettings
{
	PinkSettings() : p_fmt(0), p_ml(0), engine(0) {}
	int p_fmt, p_ml, engine;
	
    bool operator==(const PinkSettings &rhs)
    {
        if(p_fmt != rhs.p_fmt)
        {
            return false;
        }
        else if(p_ml != rhs.p_ml)
        {
            return false;
        }
		else if(engine != rhs.engine)
        {
            return false;
        }
        
        return true;
    }

    
    bool operator!=(const PinkSettings &rhs) { return !operator==(rhs); }
};



Parameter fm_att, timbre_att, morph_att;

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size) {

	//button.Debounce();
	//toggle.Debounce();

	hw.ProcessAllControls();
	updateSwitches();

	


	float pitch = hw.GetKnobValue(hw.KNOB_0) * 8.f - 4.f;

	patch.note = 60.f + pitch * 12.f;
	patch.harmonics = hw.GetKnobValue(hw.KNOB_3);
	patch.timbre = hw.GetKnobValue(hw.KNOB_1);
	patch.morph = hw.GetKnobValue(hw.KNOB_2);
	patch.lpg_colour = 0.5f;
	patch.decay = 0.5f;
	patch.frequency_modulation_amount = fm_att.Process();
	patch.timbre_modulation_amount = timbre_att.Process();
	patch.morph_modulation_amount = morph_att.Process();

	float voct_cv = hw.GetCvValue(hw.CV_0);
	float voct    = fmap(voct_cv, 0.f, 60.f);
	modulations.note = voct;

	// todo: work out how modulations.timbre_patched etc can work
	modulations.frequency = hw.GetCvValue(hw.CV_1);
	modulations.harmonics = hw.GetCvValue(hw.CV_2);
	modulations.timbre = hw.GetCvValue(hw.CV_3);
	modulations.morph = hw.GetCvValue(hw.CV_4);
	modulations.engine = hw.GetCvValue(hw.CV_5);
	modulations.level = hw.GetCvValue(hw.CV_6);

	modulations.frequency_patched = patched_fmt == 1 || patched_fmt == 3 ? true : false;
	modulations.timbre_patched = patched_fmt == 2 || patched_fmt == 3 ? true : false;
	modulations.morph_patched = patched_ml == 1 || patched_ml == 3 ? true : false;
	modulations.level_patched = patched_ml == 2 || patched_ml == 3 ? true : false;


	if (!hw.SwitchState(S0A)) {
		modulations.trigger = 5.f * hw.gate_in1.State();
		modulations.trigger_patched = true;
	}
	else {
		modulations.trigger = 0.f;
		modulations.trigger_patched = false;
	}

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

	fm_att.Init(hw.knob[hw.KNOB_8],-1.f,1.f,Parameter::LINEAR);
	timbre_att.Init(hw.knob[hw.KNOB_9],-1.f,1.f,Parameter::LINEAR);
	morph_att.Init(hw.knob[hw.KNOB_10],-1.f,1.f,Parameter::LINEAR);
	

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
	if (hw.SwitchRisingEdge(S4)) {
          //RealignPots();
          if (patch.engine >= 8) {
            patch.engine = patch.engine & 7;
          } else {
            patch.engine = (patch.engine + 1) % 8;
          }
          trigger_save = true;
        }
  
        if (hw.SwitchRisingEdge(S5)) {
          //RealignPots();
          if (patch.engine < 8) {
            patch.engine = (patch.engine & 7) + 8;
          } else {
            patch.engine = 8 + ((patch.engine + 1) % 8);
          }
          trigger_save = true;
        }

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
}

void updateLeds() {
	hw.ClearLeds();
	
	hw.SetRGBColor((patch.engine & 8 ? hw.RGB_LED_4 : hw.RGB_LED_1),DaisyWhite::Colors (patch.engine & 7));

	if(patched_fmt == 1) {
		hw.SetGreenLeds(hw.GREEN_LED_3,1.f);
	} else if(patched_fmt == 2) {
		hw.SetGreenLeds(hw.GREEN_LED_4,1.f);
	} else if(patched_fmt == 3) {
		hw.SetGreenLeds(hw.GREEN_LED_3,1.f);
		hw.SetGreenLeds(hw.GREEN_LED_4,1.f);
	}

	if(patched_ml == 1) {
		hw.SetGreenDirectLeds(hw.GREEN_D_LED_6,1.f);
	} else if(patched_ml == 2) {
		hw.SetGreenDirectLeds(hw.GREEN_D_LED_5,1.f);
	} else if(patched_ml == 3) {
		hw.SetGreenDirectLeds(hw.GREEN_D_LED_6,1.f);
		hw.SetGreenDirectLeds(hw.GREEN_D_LED_5,1.f);
	}


	hw.UpdateLeds();
}

/** @brief Sets the calibration data for 1V/Octave over Warp CV 
 *  typically set after reading stored data from external memory.
 */
inline void SetPatchedSettings(int pfmt, int pml, int engine)
{
	patched_fmt = pfmt;
	patched_ml = pml;
	patch.engine = engine;
}

static constexpr uint32_t kSettingsDataOffset = 1024;

/** Get the scale and offset data from the calibration 
 *  \retval returns true if calibration has been performed.
*/
inline void GetPatchedSettings(int &pfmt, int &pml, int &engine)
{
	pfmt  = patched_fmt;
	pml = patched_ml;
	engine = patch.engine;
}


void LoadPinkSettings()
{
	daisy::PersistentStorage<PinkSettings> settings_storage(hw.seed.qspi);
	PinkSettings                           default_settings;
	settings_storage.Init(default_settings, kSettingsDataOffset);
	auto &pink_settings = settings_storage.GetSettings();
	SetPatchedSettings(pink_settings.p_fmt, pink_settings.p_ml, pink_settings.engine);
}

void SavePinkSettings()
{
	daisy::PersistentStorage<PinkSettings> settings_storage(hw.seed.qspi);
	PinkSettings                           default_settings;
	settings_storage.Init(default_settings, kSettingsDataOffset);
	auto &pink_settings = settings_storage.GetSettings();
	GetPatchedSettings(pink_settings.p_fmt, pink_settings.p_ml, pink_settings.engine);
	settings_storage.Save();
}
