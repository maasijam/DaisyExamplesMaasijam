#pragma once

#include "grain.h"
#include "crc_noise.h"

#define MAX_GRAINS 16
#define DEFAULT_GRAIN_DUR 0.2f 
#define DEFAULT_GRAIN_PITCH 1.0f 
#define DEFAULT_SCAN_RATE 1.0f 
#define DEFAULT_GRAIN_DENSITY 10 
#define DEFAULT_SCATTER_DIST  0.1f 
#define DEFAULT_PITCH_DIST    0.1f 
#define DEFAULT_GRAIN_VOL     0.7f

// Let's stick to 16bit samples for now
// This can be templated later
class Granulator
{
  public:
    Granulator() {}
    ~Granulator() {}

    void Init(float sr, int16_t *start, size_t len, float *env, size_t env_len) 
    {
      sr_ = sr;
      len_ = len;
      sample_pos_.Init(sr_, len_);
      sample_pos_.ToggleLoop();
      sample_loop_ = true;
      env_mem_ = env;
      env_len_ = env_len;
      density_count_ = -1;
      SetGrainDuration(DEFAULT_GRAIN_DUR);
      SetGrainPitch(DEFAULT_GRAIN_PITCH);
      SetScanRate(DEFAULT_SCAN_RATE);
      SetDensity(sr_/DEFAULT_GRAIN_DENSITY);
      SetScatterDist(DEFAULT_SCATTER_DIST);
      SetPitchDist(DEFAULT_PITCH_DIST);
      stop_ = random_pitch_ = scatter_grain_ = reverse_grain_ = false;
      for (size_t i = 0; i < MAX_GRAINS; i++) {
	silo[i].Init(sr_, start, len, DEFAULT_GRAIN_VOL, env_mem_, env_len_);
      }
      rng.Init();
    }

    void Stop()
    {
      stop_ = true;
    }

    void Start()
    {
      stop_ = false;
    }

    void Reset(int16_t *start, size_t len) 
    {
      len_ = len;
      sample_pos_.Init(sr_, len_);
      sample_pos_.ToggleLoop();
      sample_loop_ = true;
      density_count_ = -1;
      SetGrainDuration(DEFAULT_GRAIN_DUR);
      SetGrainPitch(DEFAULT_GRAIN_PITCH);
      SetScanRate(DEFAULT_SCAN_RATE);
      SetDensity(sr_/DEFAULT_GRAIN_DENSITY);
      SetScatterDist(DEFAULT_SCATTER_DIST);
      SetPitchDist(DEFAULT_PITCH_DIST);
      random_pitch_ = scatter_grain_ = reverse_grain_ = false;
      for (size_t i = 0; i < MAX_GRAINS; i++) {
	silo[i].Init(sr_, start, len, DEFAULT_GRAIN_VOL, env_mem_, env_len_);
      }
      stop_ = false;
    }

    void ToggleSampleLoop(bool state)
    {
      sample_loop_ = state;
      sample_pos_.ToggleLoop();
      if (sample_loop_) Start();
    }

    void SetGrainDuration(float dur)
    {
      grain_dur_ = dur;
    }

    void SetGrainPitch(float pitch)
    {
      grain_pitch_ = pitch;
    }

    void SetPitchDist(float dist)
    {
      pitch_dist_ = dist;
    }

    void SetScanRate(float rate)
    {
      sample_pos_.SetPitch(rate);
    }

    // dist is % of the sample length
    // scatter_dist_ is stored as number of samples
    void SetScatterDist(float dist)
    {
      scatter_dist_ = dist * len_;
    }

    void ChangeEnv(float *env)
    {
      env_mem_ = env;
    }

    void Dispatch(size_t sample_pos)
    {
      float rand;
      float pitch = grain_pitch_;
      for (size_t i = 0; i < MAX_GRAINS; i++) {
	if (silo[i].IsDone()) {
	  if (random_pitch_) {
	    rand = rng.Process();
	    pitch = fminf(4.0f, fmaxf(0.25f, pitch * (1.0 + (rand * pitch_dist_))));
	  }
	  silo[i].Dispatch(sample_pos, grain_dur_, env_mem_, pitch, reverse_grain_);
	  return;
	}
      }
    }

    // density is number of samples until a new grain is dispatched
    void SetDensity(int32_t density)
    {
      density_ = density;
    }


    void ToggleScanReverse()
    {
      sample_pos_.ToggleReverse();
    }

    void ToggleGrainReverse()
    {
      reverse_grain_ = reverse_grain_ ? false : true;
    }

    void ToggleScatter()
    {
      scatter_grain_ = scatter_grain_ ? false : true;
    }

    void ToggleFreeze()
    {
      freeze_ = freeze_ ? false : true;
      scatter_grain_ = freeze_;
    }

    void ToggleRandomPitch(bool state)
    {
      random_pitch_ = state;
    }

    // All pos are in the range 0 to 1
    // They will be multiplied by len_ to get actual size_t sample position
    void SetSampleStart(float pos)
    {
      sample_pos_.SetStartPos(pos);
    }

    void SetSampleEnd(float pos)
    {
      sample_pos_.SetEndPos(pos);
    }

    float Process()
    {
      float sample = 0;
      float rand;
      int32_t offset;
      size_t pos;
      bool eot = false;

      if (stop_) return 0.0f;

      if (freeze_)
      {
	pos = sample_pos_.GetPos();
      } else {
	pos = sample_pos_.Process(&eot);
      }


      if (density_count_-- < 0)
      {
	density_count_ = density_;
	if (scatter_grain_) {
	  rand = rng.Process();
	  offset = rand * scatter_dist_;

	  offset += pos;
	  if (offset < 0) {
	    offset += len_;
	  } else if (offset > len_) {
	    offset -= len_;
	  }
	  pos = (size_t)offset;
	}
	Dispatch(pos);
      }

      for (size_t i = 0; i < MAX_GRAINS; i++) {
	if (!silo[i].IsDone()) {
	  sample += silo[i].Process();
	}
      }

      if (eot && (sample_loop_ == false)) Stop();
      // clamp?
      return fminf(1.0f, fmaxf(-1.0f, sample));
    }

  private:
    Grain<int16_t> silo[MAX_GRAINS];
    Phasor sample_pos_;
    size_t len_, env_len_, scatter_dist_;
    int32_t density_, density_count_;
    float sr_, grain_dur_, grain_pitch_, pitch_dist_;
    float *env_mem_;
    bool sample_loop_, stop_, reverse_grain_, scatter_grain_, random_pitch_, freeze_;
    daisysp::crc_noise rng;
};

