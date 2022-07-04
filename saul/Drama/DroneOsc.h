#ifndef DRONEOSC_H
#define DRONEOSC_H

#include "daisysp.h"

using namespace daisysp;

class DroneOscillator
{
	Oscillator		m_low_osc;
	Oscillator		m_base_osc;
	Oscillator		m_high_osc;
	float			m_amplitude = 0.0f;
    float			m_detune = 0.0f;

public:

	void initialise(float sample_rate)
	{
		auto init_osc =[sample_rate](Oscillator& osc)
		{
			osc.Init(sample_rate);
			osc.SetWaveform(osc.WAVE_POLYBLEP_SAW);
			osc.SetAmp(1.0f);
		};

		init_osc(m_low_osc);
		init_osc(m_base_osc);
		init_osc(m_high_osc);

		m_amplitude				= 0.0f;
	}

	void set_amplitude(float a)
	{
		m_amplitude = a;
	}

    void set_detune(float d)
	{
		m_detune = d;
	}

    void set_freq( float coarse, float cv_voct, float transpose )
	{
		float coarse_tune = fmap(coarse, 12, 84);
        float voct    = fmap(cv_voct, 0, 60);
        float midi_nn = fclamp(coarse_tune + voct, 0.f, 127.f);
        midi_nn += transpose;
        if(midi_nn > 127.f) {
            midi_nn = 127.f;
        }
        float freq_base  = mtof(midi_nn);

        float detune_amt = m_detune;
        float freq_low     = freq_base + (0.05 * freq_base * detune_amt);
        float freq_high     = freq_base - (0.05 * freq_base * detune_amt);

		m_low_osc.SetFreq( freq_low );
		m_base_osc.SetFreq( freq_base );
		m_high_osc.SetFreq( freq_high );
	}

	

	float process()
	{
		const float avg_sin = (m_low_osc.Process() + m_base_osc.Process() + m_high_osc.Process() ) / 3;
		//float avg_sin = m_base_osc.Process();
		return avg_sin * m_amplitude;
	}
};

#endif