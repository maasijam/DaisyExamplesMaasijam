#include "../daisy_saul.h"
#include "daisysp.h"

using namespace daisy;
using namespace daisysp;
using namespace saul;


DaisySaul hw;

static ReverbSc  rev;

#define RMS_SIZE 48
#define LOOPER_MAX_SIZE (48000 * 60 * 1) // 1 minutes stereo of floats at 48 khz

float reverb_drywet, reverb_feedback,reverb_lowpass, reverb_shimmer = 0;
int reverb_shimmer_write_pos1l, reverb_shimmer_write_pos1r, 
    reverb_shimmer_write_pos2 = 0;
int reverb_shimmer_play_pos1l, reverb_shimmer_play_pos1r;
float reverb_shimmer_play_pos2=0.f ;

int reverb_shimmer_buffer_size1l = 24000*0.773;
int reverb_shimmer_buffer_size1r = 24000*0.802;
int reverb_shimmer_buffer_size2 = 48000*0.753*2;


float reverb_previous_inl, reverb_previous_inr = 0;
float reverb_current_outl, reverb_current_outr = 0;

int reverb_feedback_display = 0;
int   reverb_rmsCount;
float reverb_current_RMS, reverb_target_RMS, reverb_feedback_RMS=0.f;
float reverb_target_compression, reverb_compression = 1.0f;


void Controls();
void GetReverbSample(float &outl, float &outr, float inl, float inr);

float global_sample_rate;


float DSY_SDRAM_BSS mlooper_buf_1l[LOOPER_MAX_SIZE];
float DSY_SDRAM_BSS mlooper_buf_1r[LOOPER_MAX_SIZE];

float DSY_SDRAM_BSS mlooper_frozen_buf_1l[LOOPER_MAX_SIZE];

float clamp(float value,float min,float max) {
    if (value < min){
        return min;
    }
    if (value > max){
        return max;
    }
    return value;
};


float map(float value, float start1, float stop1, float start2, float stop2) {
    return start2 + ((stop2 - start2) * (value - start1) )/ (stop1 - start1);
};


class Averager {

    float buffer[RMS_SIZE];
    int cursor;
    public:
    
    Averager() {
        Clear();
    }
    ~Averager() {}
    
    float ProcessRMS() {
        float sum = 0.f;
        for (int i =0; i< cursor; i++) {
            sum = sum + buffer[i];
        }
        float result = sqrt(sum/cursor);
        Clear();
        return result;
    }
    void Clear() {
        for (int i =0; i< RMS_SIZE; i++) {
            buffer[i] = 0.f;
        }
        cursor = 0;
    }
    void Add(float sample){
        buffer[cursor] = sample;
        cursor++;
    }
};

static Averager reverb_averager;


void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
    float out1, out2, in1, in2;
    
    Controls();
  
    
    for(size_t i = 0; i < size; i ++)
    {
        
        in1 = in[0][i];
        in2 = in[1][i];

        out1 = 0.f;
        out2 = 0.f;

        GetReverbSample(out1, out2, in1, in2);

        out[0][i] = out1;
        out[1][i] = out2;
    }
}



int main(void)
{
    float sample_rate;
    
    
    hw.Init();
    //hw.SetAudioBlockSize(4);
    sample_rate = hw.AudioSampleRate();
    global_sample_rate = sample_rate;
    rev.Init(sample_rate);
    
    //reverb parameters
    rev.SetLpFreq(9000.0f);
    rev.SetFeedback(0.85f);
    
    

    
    hw.StartAdc();
    hw.StartAudio(AudioCallback);


    while (1) {
        
    }
}

void UpdateKnobs()

{  
    float blend = hw.GetKnobValue(KNOB_8);
    //float speed = hw.GetKnobValue(KNOB_1); 
    float tone = hw.GetKnobValue(KNOB_2);
    float index = hw.GetKnobValue(KNOB_9);
    float regen = hw.GetKnobValue(KNOB_10); 
    //float size = hw.GetKnobValue(KNOB_5);
    float dense = hw.GetKnobValue(KNOB_6);

    //float tone_freq;

    //int sw1,sw2;

    
    //blend = reverb wet/dry
    //tone = reverb_lowpass
    //speed = 
    //index = shimmer
    //regen = reverb feedback
    //size =
    //dense = reverb compression


    reverb_lowpass = global_sample_rate*tone / 2.f;
    rev.SetLpFreq(reverb_lowpass);      
    reverb_shimmer = index;
    reverb_feedback = 0.8f + (std::log10(10 + regen*90) -1.000001f)*0.4f;
    reverb_feedback_display = regen*100;
    rev.SetFeedback(reverb_feedback);
    
    reverb_compression = dense +0.5f;

    reverb_drywet = blend;
    
}


void Controls()
{

    reverb_drywet = 0;

    hw.ProcessAnalogControls();
    UpdateKnobs();

}

void WriteShimmerBuffer1(float in_l, float in_r)
{   
    //writes the input to the buffer
    mlooper_buf_1l[reverb_shimmer_write_pos1l] = in_l;
    mlooper_buf_1r[reverb_shimmer_write_pos1r] = in_r;
    reverb_shimmer_write_pos1l = (reverb_shimmer_write_pos1l +1) % reverb_shimmer_buffer_size1l;
    reverb_shimmer_write_pos1r = (reverb_shimmer_write_pos1r +1) % reverb_shimmer_buffer_size1r;
};

void WriteShimmerBuffer2(float in_l, float in_r)
{   
    //writes the input to the buffer
    mlooper_frozen_buf_1l[reverb_shimmer_write_pos2] = (in_r + in_l)/2;
    reverb_shimmer_write_pos2 = (reverb_shimmer_write_pos2 +1) % reverb_shimmer_buffer_size2;
};

float CompressSample(float sample) {
    if (sample > 0.4) {
        sample = clamp(sample - map(sample, 0.4f, 5.0f, 0.0f, 0.6f), 0.0f, 2.0f);
    }
    if (sample < -0.4) {
        sample = clamp(sample - map(sample, -5.0f,-0.4f,  -0.6f, 0.0f), -2.0f, 0.0f);
    }

    if (sample > 0.8) {
        sample = clamp(sample - map(sample, 0.8f, 2.0f, 0.0f, 0.1f), 0.0f, 0.9f);
    }
    if (sample < -0.8) {
        sample = clamp(sample - map(sample, -2.0f,-0.8f,  -0.1f, 0.0f), -0.9f, 0.0f);
    }
    return sample;
}


void GetReverbSample(float &outl, float &outr, float inl, float inr)
{   
    //Shimmer part: basically we write the buffer once every two frames and then we read it every frame at two
    //different speeds so two octaves are produced (the higher one is reduced in intensity)
    float shimmer_l = 0.0f;
    float shimmer_r = 0.0f;
    
    shimmer_l = mlooper_buf_1l[reverb_shimmer_play_pos1l] ;
    shimmer_r = mlooper_buf_1r[reverb_shimmer_play_pos1r];
    reverb_shimmer_play_pos1l = (reverb_shimmer_play_pos1l + 2) % reverb_shimmer_buffer_size1l;
    reverb_shimmer_play_pos1r = (reverb_shimmer_play_pos1r + 2) % reverb_shimmer_buffer_size1r;
    WriteShimmerBuffer1(inl,inr);

    float octave2_shim = mlooper_frozen_buf_1l[(int)reverb_shimmer_play_pos2]*0.5f;
    shimmer_l = shimmer_l + octave2_shim;
    shimmer_r = shimmer_r + octave2_shim;
    reverb_shimmer_play_pos2 = (reverb_shimmer_play_pos2 + 4);
    if (reverb_shimmer_play_pos2 > reverb_shimmer_buffer_size2) {
        reverb_shimmer_play_pos2 = reverb_shimmer_play_pos2-reverb_shimmer_buffer_size2;
    }
    WriteShimmerBuffer2(inl,inr);

    reverb_rmsCount++;
    reverb_rmsCount %= (RMS_SIZE);

    if (reverb_rmsCount == 0) {
        reverb_target_RMS = reverb_averager.ProcessRMS();
    }
    fonepole(reverb_current_RMS, reverb_target_RMS, .1f);
    fonepole(reverb_feedback_RMS, reverb_target_RMS, .01f);

    rev.SetFeedback(reverb_feedback -reverb_feedback_RMS*0.75f);
    //summing the output of the incoming audio, the previous input, and the shimmer 
    float sum_inl = (inl + shimmer_l * reverb_shimmer*(reverb_feedback*0.5f + 0.5f)*(0.5f+reverb_current_RMS*0.5f))*0.5f;
    float sum_inr = (inr + shimmer_r * reverb_shimmer*(reverb_feedback*0.5f + 0.5f)*(0.5f+reverb_current_RMS*0.5f))*0.5f;

    fonepole(reverb_target_compression, reverb_compression, .001f);

    //basic sample based limiter to avoid overloading the output
    sum_inl = CompressSample(sum_inl*reverb_target_compression);
    sum_inr = CompressSample(sum_inr*reverb_target_compression);

    rev.Process(sum_inl, sum_inr, &outl, &outr);
    

    reverb_current_outl =  outl;
    reverb_current_outr =  outr;
    
    reverb_averager.Add((reverb_current_outl*reverb_current_outl + reverb_current_outr*reverb_current_outr)/2);
    //equal power crossfade dry wet
    if (reverb_drywet > 0.98f) {
        reverb_drywet = 1.f;
    }
    outl = (sqrt(0.5f * (reverb_drywet*2.0f))*reverb_current_outl + sqrt(0.95f * (2.f - (reverb_drywet*2))) * inl)*0.7f;
    outr = (sqrt(0.5f * (reverb_drywet*2.0f))*reverb_current_outr + sqrt(0.95f * (2.f - (reverb_drywet*2))) * inr)*0.7f;


}