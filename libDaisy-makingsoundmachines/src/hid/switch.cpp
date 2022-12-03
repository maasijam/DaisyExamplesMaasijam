#include "hid/switch.h"
using namespace daisy;

void Switch::Init(dsy_gpio_pin pin,
                  float        update_rate,
                  Type         t,
                  Polarity     pol,
                  Pull         pu)
{
    time_per_update_ = 1.0f / update_rate;
    state_           = 0x00;
    state16_           = 0x0000;
    state32_           = 0x00000000;
    time_held_       = 0;
    t_               = t;
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
void Switch::Init(dsy_gpio_pin pin, float update_rate)
{
    Init(pin, update_rate, TYPE_MOMENTARY, POLARITY_INVERTED, PULL_UP);
}

void Switch::Debounce()
{
    // shift over, and introduce new state.
    state_ = (state_ << 1)
             | (flip_ ? !dsy_gpio_read(&hw_gpio_) : dsy_gpio_read(&hw_gpio_));
    // Set time at which button was pressed
    if(state_ == 0x7f)
        rising_edge_time_ = System::GetNow();
}

void Switch::Debounce16()
{
    // shift over, and introduce new state.
    state16_ = (state16_ << 1)
             | (flip_ ? !dsy_gpio_read(&hw_gpio_) : dsy_gpio_read(&hw_gpio_));
    // Reset time held on any edge.
    if(state16_ == 0x7fff || state16_ == 0x8000)
        time_held_ = 0;
    // Add while held (16-tick delay on hold due to debouncing).
    if(state16_ == 0xffff)
        time_held_ += time_per_update_;
}

void Switch::Debounce32()
{
    // shift over, and introduce new state.
    state32_ = (state32_ << 1)
             | (flip_ ? !dsy_gpio_read(&hw_gpio_) : dsy_gpio_read(&hw_gpio_));
    // Reset time held on any edge.
    if(state32_ == 0x7fffffff || state32_ == 0x80000000)
        time_held_ = 0;
    // Add while held (32-tick delay on hold due to debouncing).
    if(state32_ == 0xffffffff)
        time_held_ += time_per_update_;
}