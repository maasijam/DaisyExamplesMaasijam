#include "hid/gatein.h"

using namespace daisy;

void GateIn::Init(dsy_gpio_pin *pin_cfg)
{
    pin_.pin    = *pin_cfg;
    pin_.mode   = DSY_GPIO_MODE_INPUT;
    pin_.pull   = DSY_GPIO_NOPULL;
    prev_state_ = 0;
    state_      = 0;
    dsy_gpio_init(&pin_);
}

bool GateIn::Trig()
{
    // Inverted because of typical BJT input circuit.
    prev_state_ = state_;
    state_      = !dsy_gpio_read(&pin_);
    return state_ && !prev_state_;
}

bool GateIn::RisingEdge()
{
    // Inverted because of typical BJT input circuit.
    prev_state_ = state_;
    state_      = !dsy_gpio_read(&pin_);

    bool rising_ = false;

    if( state_ == true ) { rising_ = (state_ && !prev_state_); }

    return rising_;
}

bool GateIn::FallingEdge()
{
    // Inverted because of typical BJT input circuit.
    prev_state_ = state_;
    state_      = dsy_gpio_read(&pin_);

    bool falling_ = false;

    if( state_ == true ) { falling_ = (state_ && !prev_state_); }

    return falling_;
}

bool GateIn::StateChange()
{
    // Inverted because of typical BJT input circuit.
    state_      = !dsy_gpio_read(&pin_);

    bool statechange_ = false;

    statechange_ = (state_ && !prev_state_);

    prev_state_ = state_;

    return statechange_;
}