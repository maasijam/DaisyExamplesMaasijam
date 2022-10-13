// Copyright 2021 Adam Fulford / 2022 maasijam
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
// 
// See http://creativecommons.org/licenses/MIT/ for more information.


//#include "QSPI_Settings.h"
#include "../daisy_varga.h"
#include "daisysp.h"




using namespace daisy;
using namespace daisysp;


DaisyVarga hw;

Parameter p_knob1, p_knob2, p_knob3, p_knob4;

int main(void)
{
    
    
    hw.Init();
    //hw.ClearLeds();
    //hw.UpdateLeds(); 

    dsy_gpio hw_pinG_;
    hw_pinG_.pin  = hw.seed.GetPin(29);
    hw_pinG_.mode = DSY_GPIO_MODE_OUTPUT_PP;
    dsy_gpio_init(&hw_pinG_);

    dsy_gpio hw_pinB_;
    hw_pinB_.pin  = hw.seed.GetPin(30);
    hw_pinB_.mode = DSY_GPIO_MODE_OUTPUT_PP;
    dsy_gpio_init(&hw_pinB_);
    
    
    float r1 = 0, g1 = 0, b1 = 0;
    float r2 = 0, g2 = 0, b2 = 0;
    p_knob1.Init(hw.knob[0], 0, 1, Parameter::LINEAR);
    p_knob2.Init(hw.knob[1], 0, 1, Parameter::LINEAR);
    p_knob3.Init(hw.knob[2], 0, 1, Parameter::LINEAR);
    p_knob4.Init(hw.knob[3], 0, 1, Parameter::LINEAR);

    hw.StartAdc();

    while(1)
    {
        r1 = p_knob1.Process();
        g1 = p_knob2.Process();

        r2 = p_knob3.Process();
        g2 = p_knob4.Process();

        hw.leds[0].Set(r1, g1, b1);
        hw.leds[1].Set(r2, g2, b2);

        hw.UpdateLeds();
    }
}