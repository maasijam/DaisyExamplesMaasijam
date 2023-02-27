#pragma once
#ifndef DSYSP_SHIMMERREVERB_H
#define DSYSP_SHIMMERREVERB_H

#include "daisysp.h"


#define RMS_SIZE 48



namespace daisysp
{

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


class ShimmerReverb
{
  public:
    ShimmerReverb() {}
    ~ShimmerReverb() {}
    /** Initializes the reverb module, and sets the sample_rate at which the Process function will be called.
        Returns 0 if all good, or 1 if it runs out of delay times exceed maximum allowed.
    */
    void Init(float *buffer_lfloat, float *buffer_r, float *buffer_frozen, float sample_rate);

    /** Process the input through the reverb, and updates values of out1, and out2 with the new processed signal.
    */
    void Process(float &outl, float &outr, float inl, float inr);
    void SetTone(float tone);
    void SetShimmer(float shim);
    void SetFdbk(float regen);
    void SetCompression(float dense);
    void SetDryWet(float drywet);

    float reverb_drywet, reverb_feedback,reverb_lowpass, reverb_shimmer = 0;
    int reverb_shimmer_write_pos1l, reverb_shimmer_write_pos1r, reverb_shimmer_write_pos2 = 0;
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

    
    
    

  private:
    void WriteShimmerBuffer1(float in_l, float in_r);
    void WriteShimmerBuffer2(float in_l, float in_r);
    float CompressSample(float sample);
    float clamp(float value,float min,float max);
    float map(float value, float start1, float stop1, float start2, float stop2);

    float *mlooper_buf_1l_;
    float *mlooper_buf_1r_;
    float *mlooper_frozen_buf_1l_;

    float global_sample_rate_;
    
    ReverbSc rev_;
    
};


} // namespace daisysp
#endif