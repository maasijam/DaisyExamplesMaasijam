#pragma once

#define CC_TO_VAL(x, min, max) (min + (x / 127.0f) * (max - min))

class PagedParam
{
  public:
    PagedParam() {}
    ~PagedParam() {}

    void Init(float init, float min, float max, float thresh)
    {
      
    
      thresh_ = thresh;
      locked_ = true;
      min_ = min;
      max_ = max;
      range_ = max_ - min_;
    
      cur_val_ = (init - min_) / range_;
    
    }

    void SetRange(float min, float max)
    {
      min_ = min;
      max_ = max;
      range_ = max - min;
    }


    void lock(float in)
    {
      locked_ = true;
      cur_val_ = (in - min_) / range_;
    }

    float Process(float in)
    {
      cur_val_ = in;
          
      return fminf(max_, fmaxf(min_, (min_ + cur_val_ * range_)));
    }

    /*
     * Midi input ignores pages and sets the value immediately.
     * It will always lock the value to prevent the physical controls from overriding
     * the midi message.
     */
    float MidiIn(uint8_t midi_byte)
    {
      locked_ = true;
      cur_val_ = CC_TO_VAL(midi_byte, 0.0f, 1.0f);
      return min_ + cur_val_ * range_;
    }

  private:
    bool locked_;
    float thresh_, cur_val_, min_, max_, range_;
    uint8_t page_;

};
