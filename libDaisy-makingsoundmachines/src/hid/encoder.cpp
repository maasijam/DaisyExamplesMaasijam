#include "hid/encoder.h"

using namespace daisy;

void Encoder::Init(dsy_gpio_pin a,
                   dsy_gpio_pin b,
                   dsy_gpio_pin click,
                   float        update_rate)
{
    // Init GPIO for A, and B
    hw_a_.pin  = a;
    hw_a_.mode = DSY_GPIO_MODE_INPUT;
    hw_a_.pull = DSY_GPIO_PULLUP;
    hw_b_.pin  = b;
    hw_b_.mode = DSY_GPIO_MODE_INPUT;
    hw_b_.pull = DSY_GPIO_PULLUP;
    dsy_gpio_init(&hw_a_);
    dsy_gpio_init(&hw_b_);
    // Default Initialization for Switch
    sw_.Init(click);
    // Set initial states, etc.
    inc_ = 0;
    a_ = b_ = 0xff;
    a16_ = b16_ = 0xFFFF;
    a32_ = b32_ = 0xFFFFFFFF;
}

void Encoder::Debounce()
{
    // Shift Button states to debounce
    a_ = (a_ << 1) | dsy_gpio_read(&hw_a_);
    b_ = (b_ << 1) | dsy_gpio_read(&hw_b_);
    // Debounce built-in switch
    sw_.Debounce();
    // infer increment direction
    inc_ = 0; // reset inc_ first
    if((a_ & 0x03) == 0x02 && (b_ & 0x03) == 0x00)
    {
        inc_ = 1;
    }
    else if((b_ & 0x03) == 0x02 && (a_ & 0x03) == 0x00)
    {
        inc_ = -1;
    }
}

void Encoder::Debounce16()
{
    // Shift Button states to debounce
    a16_ = (a16_ << 1) | dsy_gpio_read(&hw_a_);
    b16_ = (b16_ << 1) | dsy_gpio_read(&hw_b_);
    // Debounce built-in switch
    sw_.Debounce16();
    // infer increment direction
    inc_ = 0; // reset inc_ first
    if((a16_ & 0x03FF) == 0x0200 && (b16_ & 0x03FF) == 0x0000)
    {
        inc_ = 1;
    }
    else if((b16_ & 0x03FF) == 0x0200 && (a16_ & 0x03FF) == 0x0000)
    {
        inc_ = -1;
    }
}

void Encoder::Debounce32()
{
    // Shift Button states to debounce
    a32_ = (a32_ << 1) | dsy_gpio_read(&hw_a_);
    b32_ = (b32_ << 1) | dsy_gpio_read(&hw_b_);
    // Debounce built-in switch
    sw_.Debounce32();
    // infer increment direction
    inc_ = 0; // reset inc_ first
    if((a32_ & 0x03FFFFFF) == 0x02000000 && (b32_ & 0x03FFFFFF) == 0x00000000)
    {
        inc_ = 1;
    }
    else if((b32_ & 0x03FFFFFF) == 0x02000000 && (a32_ & 0x03FFFFFF) == 0x00000000)
    {
        inc_ = -1;
    }
}