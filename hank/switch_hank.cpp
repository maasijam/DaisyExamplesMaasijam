#include "switch_hank.h"
using namespace daisy;

void SwitchHank::Init(dsy_gpio_pin pin,
                  float        update_rate,
                  long          doubleclickTime,
                  Type         t,
                  Polarity     pol,
                  Pull         pu)
{
    last_update_ = System::GetNow();
    updated_     = false;
    state_       = 0x00;
    
    t_           = t;
    clickCount_    = 0;
    doubleclickTime_ = doubleclickTime;
    doubleclicked_        = false;
    first_rising_edge_time_ = 0;
    // Flip may seem opposite to logical direction,
    // but here 1 is pressed, 0 is not.
    flip_         = pol == POLARITY_INVERTED ? true : false;
    hw_gpio_.pin  = pin;
    hw_gpio_.mode = DSY_GPIO_MODE_INPUT;
    switch(pu)
    {
        case PULL_UP: hw_gpio_.pull = DSY_GPIO_PULLUP; break;
        case PULL_DOWN: hw_gpio_.pull = DSY_GPIO_PULLDOWN; break;
        case PULL_NONE: hw_gpio_.pull = DSY_GPIO_NOPULL; break;
        default: hw_gpio_.pull = DSY_GPIO_PULLUP; break;
    }
    dsy_gpio_init(&hw_gpio_);
}
void SwitchHank::Init(dsy_gpio_pin pin, float update_rate,long doubleclickTime)
{
    Init(pin, update_rate, doubleclickTime, TYPE_MOMENTARY, POLARITY_INVERTED, PULL_UP);
}

void SwitchHank::Debounce()
{
    // update no faster than 1kHz
    uint32_t now = System::GetNow();
    updated_     = false;
    doubleclicked_ = false;

    if(now - last_update_ >= 1)
    {
        last_update_ = now;
        updated_     = true;

        // shift over, and introduce new state.
        state_
            = (state_ << 1)
              | (flip_ ? !dsy_gpio_read(&hw_gpio_) : dsy_gpio_read(&hw_gpio_));
        // Set time at which button was pressed
        if(state_ == 0x7f) {
            rising_edge_time_ = System::GetNow();
            clickCount_++;
            if(clickCount_ == 1) {
                first_rising_edge_time_ = rising_edge_time_;
            }
            
            if(clickCount_ > 2) {
                clickCount_ = 0;
                first_rising_edge_time_ = 0;
            }
        }
        
        
    }

    if ((now - first_rising_edge_time_) < doubleclickTime_ && clickCount_ == 2)
    {
         doubleclicked_ = true;
         clickCount_ = 0;
    } else if(clickCount_ == 2) {
        clickCount_ = 0;
    }

    if ((now - first_rising_edge_time_) > doubleclickTime_)
    {
        clickCount_ = 0;
    }
}
