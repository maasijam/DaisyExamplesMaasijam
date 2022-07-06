#ifndef DRONEOSC_H
#define DRONEOSC_H

#include "daisysp.h"

using namespace daisysp;

class DroneOscillator
{
	Oscillator		osc1_a, osc1_b, osc1_c, osc2_a, osc2_b, osc2_c;
	float			m_amplitude = 0.0f;
    float			m_detune = 0.0f;

public:

	void init(float sample_rate)
	{
		auto init_osc =[sample_rate](Oscillator& osc)
		{
			osc.Init(sample_rate);
			osc.SetWaveform(osc.WAVE_POLYBLEP_SAW);
			osc.SetAmp(1.0f);
		};

		init_osc(osc1_a);
		init_osc(osc1_b);
		init_osc(osc1_c);

		init_osc(osc2_a);
		init_osc(osc2_b);
		init_osc(osc2_c);

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

	void set_waveform(int waveform)
	{
		osc1_a.SetWaveform( waveform );
		osc1_b.SetWaveform( waveform );
		osc1_c.SetWaveform( waveform );
		osc2_a.SetWaveform( waveform );
		osc2_b.SetWaveform( waveform );
		osc2_c.SetWaveform( waveform );
	}

    void set_freq( float coarse, float cv_voct, float transpose, float rdm )
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
        float freq_low1     = freq_base + (0.03 * freq_base * detune_amt);
        float freq_high1     = freq_base - (0.06 * freq_base * detune_amt);

		float freq_low2     = freq_base + ((0.07 + rdm) * freq_base * detune_amt);
        float freq_high2     = freq_base - ((0.04 + rdm) * freq_base * detune_amt);

		osc1_a.SetFreq( freq_low1 );
		osc1_b.SetFreq( freq_base );
		osc1_c.SetFreq( freq_high1 );

		osc2_a.SetFreq( freq_low2 );
		osc2_b.SetFreq( freq_base );
		osc2_c.SetFreq( freq_high2 );
	}

	

	float process_left()
	{
		const float avg_sin = (osc1_a.Process() + osc1_b.Process() + osc1_c.Process() ) / 3;
		return avg_sin * m_amplitude;
	}
	float process_right()
	{
		const float avg_sin = (osc2_a.Process() + osc2_b.Process() + osc2_c.Process() ) / 3;
		return avg_sin * m_amplitude;
	}
};

#endif