#include "../daisy_saul.h"
#include "daisysp.h"
#include "frame.h"
#include "reverb.h"

using namespace daisy;
using namespace daisysp;


DaisySaul saul;
Reverb    reverb_;

uint16_t reverb_buffer[65536];


void AudioCallback(AudioHandle::InputBuffer  in,
                   AudioHandle::OutputBuffer out,
                   size_t                    size)
{
    float sig, delsig;           // Mono Audio Vars
    float trig, nn, decay;       // Pluck Vars
    float deltime, delfb, kval;  // Delay Vars
    float dryL, dryR, send, sendL, sendR, wetL, wetR; // Effects Vars
    float samplerate;

    
    
    

        
    saul.ProcessAnalogControls();
    float feedback = saul.knob[10].Value();
    send = saul.knob[8].Value();
    float reverb_amount = saul.knob[9].Value() * 0.95f;
    reverb_amount += feedback * (2.0f - feedback);
    CONSTRAIN(reverb_amount, 0.0f, 1.0f);

    reverb_.set_amount(reverb_amount * 0.54f);
    reverb_.set_diffusion(0.7f);
    reverb_.set_time(0.35f + 0.63f * reverb_amount);
    reverb_.set_input_gain(0.2f);
    reverb_.set_lp(0.6f + 0.37f * feedback);
    

    

    for(size_t i = 0; i < size; i++)
    {
       
        dryL = in[0][i];
        dryR = in[1][i];

        sendL = dryL * send;
        sendR = dryR * send;

        reverb_.Process(sendL, sendR, &wetL, &wetR);
        
        
        // Out 1 and 2 are Mixed 
        out[0][i] = (dryL * 1.0f) + wetL;
        out[1][i] = (dryR * 1.0f) + wetR;


    }
}

int main(void)
{
    saul.Init();
    reverb_.Init(reverb_buffer);
    saul.StartAudio(AudioCallback);
    while(1) {}
}
