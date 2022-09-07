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

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size) {

	//button.Debounce();
	//toggle.Debounce();

	hw.ProcessAllControls();

	if (hw.SwitchRisingEdge(S1) || hw.gate_in2.Trig()) {
		patch.engine = (patch.engine + 1) % voice.GetNumEngines();
	}

	float pitch = hw.GetKnobValue(hw.KNOB_0) * 8.f - 4.f;

	patch.note = 60.f + pitch * 12.f;
	patch.harmonics = hw.GetKnobValue(hw.KNOB_3);
	patch.timbre = hw.GetKnobValue(hw.KNOB_1);
	patch.morph = hw.GetKnobValue(hw.KNOB_2);
	patch.lpg_colour = 0.5f;
	patch.decay = 0.5f;
	patch.frequency_modulation_amount = 0.f;
	patch.timbre_modulation_amount = 0.f;
	patch.morph_modulation_amount = 0.f;

	float voct_cv = hw.GetCvValue(hw.CV_0);
	float voct    = fmap(voct_cv, 0.f, 60.f);
	modulations.note = voct;

	// todo: work out how modulations.timbre_patched etc can work
	modulations.harmonics = hw.GetCvValue(hw.CV_1);
	modulations.timbre = hw.GetCvValue(hw.CV_2);
	modulations.morph = hw.GetCvValue(hw.CV_3);

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
	

	patch.engine = 0;
	modulations.engine = 0;

	stmlib::BufferAllocator allocator(shared_buffer, sizeof(shared_buffer));
	voice.Init(&allocator);
    hw.StartAdc();
	hw.StartAudio(AudioCallback);

	while (1) {	}
}
