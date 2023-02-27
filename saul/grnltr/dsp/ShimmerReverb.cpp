
#include "ShimmerReverb.h"

using namespace daisysp;

void ShimmerReverb::Init(float *buffer_l, float *buffer_r, float *buffer_frozen, float sample_rate)
{
    rev_.Init(sample_rate);
    mlooper_buf_1l_ = buffer_l;
    mlooper_buf_1r_ = buffer_r;
    mlooper_frozen_buf_1l_ = buffer_frozen;
    global_sample_rate_ = sample_rate;
    rev_.SetLpFreq(9000.0f);
    rev_.SetFeedback(0.85f);
}

float ShimmerReverb::clamp(float value,float min,float max) {
    if (value < min){
        return min;
    }
    if (value > max){
        return max;
    }
    return value;
};


float ShimmerReverb::map(float value, float start1, float stop1, float start2, float stop2) {
    return start2 + ((stop2 - start2) * (value - start1) )/ (stop1 - start1);
};



void ShimmerReverb::WriteShimmerBuffer1(float in_l, float in_r)
{   
    //writes the input to the buffer
    mlooper_buf_1l_[reverb_shimmer_write_pos1l] = in_l;
    mlooper_buf_1r_[reverb_shimmer_write_pos1r] = in_r;
    reverb_shimmer_write_pos1l = (reverb_shimmer_write_pos1l +1) % reverb_shimmer_buffer_size1l;
    reverb_shimmer_write_pos1r = (reverb_shimmer_write_pos1r +1) % reverb_shimmer_buffer_size1r;
};

void ShimmerReverb::WriteShimmerBuffer2(float in_l, float in_r)
{   
    //writes the input to the buffer
    mlooper_frozen_buf_1l_[reverb_shimmer_write_pos2] = (in_r + in_l)/2;
    reverb_shimmer_write_pos2 = (reverb_shimmer_write_pos2 +1) % reverb_shimmer_buffer_size2;
};

float ShimmerReverb::CompressSample(float sample) {
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


void ShimmerReverb::Process(float &outl, float &outr, float inl, float inr)
{   
    //Shimmer part: basically we write the buffer once every two frames and then we read it every frame at two
    //different speeds so two octaves are produced (the higher one is reduced in intensity)
    float shimmer_l = 0.0f;
    float shimmer_r = 0.0f;
    
    shimmer_l = mlooper_buf_1l_[reverb_shimmer_play_pos1l] ;
    shimmer_r = mlooper_buf_1r_[reverb_shimmer_play_pos1r];
    reverb_shimmer_play_pos1l = (reverb_shimmer_play_pos1l + 2) % reverb_shimmer_buffer_size1l;
    reverb_shimmer_play_pos1r = (reverb_shimmer_play_pos1r + 2) % reverb_shimmer_buffer_size1r;
    WriteShimmerBuffer1(inl,inr);

    float octave2_shim = mlooper_frozen_buf_1l_[(int)reverb_shimmer_play_pos2]*0.5f;
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

    rev_.SetFeedback(reverb_feedback -reverb_feedback_RMS*0.75f);
    //summing the output of the incoming audio, the previous input, and the shimmer 
    float sum_inl = (inl + shimmer_l * reverb_shimmer*(reverb_feedback*0.5f + 0.5f)*(0.5f+reverb_current_RMS*0.5f))*0.5f;
    float sum_inr = (inr + shimmer_r * reverb_shimmer*(reverb_feedback*0.5f + 0.5f)*(0.5f+reverb_current_RMS*0.5f))*0.5f;

    fonepole(reverb_target_compression, reverb_compression, .001f);

    //basic sample based limiter to avoid overloading the output
    sum_inl = CompressSample(sum_inl*reverb_target_compression);
    sum_inr = CompressSample(sum_inr*reverb_target_compression);

    rev_.Process(sum_inl, sum_inr, &outl, &outr);
    

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

void ShimmerReverb::SetTone(float tone) {
    reverb_lowpass = global_sample_rate_*tone / 2.f;
    rev_.SetLpFreq(reverb_lowpass);
}
void ShimmerReverb::SetShimmer(float shim){
    reverb_shimmer = shim;
}
void ShimmerReverb::SetFdbk(float fdbk){
    reverb_feedback = 0.8f + (std::log10(10 + fdbk*90) -1.000001f)*0.4f;
    rev_.SetFeedback(reverb_feedback);
}
void ShimmerReverb::SetCompression(float comp){
    reverb_compression = comp +0.5f;
}
void ShimmerReverb::SetDryWet(float drywet){
    reverb_drywet = drywet;
}
