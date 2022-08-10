// Copyright 2021 Adam Fulford
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

#include "LEDs.h"

//TempoLED Functions:

void TempoLED::init(uint8_t ledidx,float samplerate)  //led pin number
{
    ////led.Init(ledpin,false,samplerate);
    ////led.Set(0.0f);
    ////led.Update();
    ledidx_ = ledidx;
    ledon_ = false;
    blink.Init(samplerate);
    blink.SetWaveform(blink.WAVE_SQUARE);
    blink.SetAmp(1.0f);
    blink.SetFreq(2.0f);
    //div_int = 1;

}

void TempoLED::setTempo(float tempo)
{
    blink.SetFreq(tempo);
}

void TempoLED::resetPhase()
{
    blink.Reset();
}

/*
void TempoLED::resetPhaseCounter()
{
    phaseCounter_ = 0;
*/

void TempoLED::update()
{
    
    float ledvalue{ (blink.Process() + 1.0f) / 2.0f };

    if(ledvalue > 0.6f) {
        //saul.SetLed(ledidx_,false);
        ledon_ = true;
      
        
    } else {
        //saul.SetLed(ledidx_,true);
        ledon_ = false;
       
        
    }

}

//dummy - set to phase of base tempo
void TempoLED::update(TempoDivs div, float phase)
{
    float delayPhase{};

    //update multi or div value
    float div_int = GetDivInt(div);
    delayPhase = phase * div_int;

    float ledvalue{sinf(delayPhase) < 0.0f ? 0.0f : 1.0f};

    if(ledvalue > 0.6f) {
        ledon_ = true;

    } else {
        ledon_ = false;

    }

}


bool TempoLED::isEOC()
{
    return blink.IsEOC();
}

//get div as an integer
float TempoLED::GetDivInt(TempoDivs div)
{
    float retVal{};

    switch (div)
    {
        case DIV6:
            retVal = 6 * 6;
        break;

        case DIV5:
            retVal = 5 * 6;
        break;
        case DIV4:
            retVal = 4 * 6;
        break;
        case DIV3:
            retVal = 3 * 6;
        break;
        case DIV2:
            retVal = 2 * 6;
        break;
        
        case UNITY:
            retVal = 6;
        break;

        case MULT2:
            retVal = 3.0f;
        break;

        case MULT3:
            retVal = 2.0f;
        break;
        
        case MULT4:
            retVal = 6/4;
        break;
        
        case MULT5:
            retVal = 6/5;
        break;   
        
        case MULT6:
            retVal = 1.0f;
        break;       

        default:
        retVal = 6.0f;    
        break;  
    }

return retVal;
}


//ButtonLED Functions:

void ButtonLED::init(uint8_t ledidx, dsy_gpio_pin switch_pin, SwitchType switchtype, float Samplerate)
{
    
    
    ledidx_ = ledidx;
    ledon_ = false;
    sw.Init(switch_pin, Samplerate);
    switchtype_ = switchtype;
    isON = false; 
       
}



void ButtonLED::update() //check switch and update LED
{
sw.Debounce();

switch (switchtype_)
{
    case Momentary:
        if(sw.RisingEdge())
        {
            toggle();
        }
    break;

    case Toggle:
        if(!sw.Pressed())
        {
            turnON();
        } 
        else
        {
            turnOFF();
        }
    break;
    case Toggle_inverted:
        if(sw.Pressed())
        {
            turnON();
        } 
        else
        {
            turnOFF();
        }
    break;
    default:
    break;
}

}

void ButtonLED::toggle()
{
    isON = !isON;
    ledon_ = isON;
    if(isON)
    {
        
    }
    else
    {
      
    } 
}

void ButtonLED::turnON()
{
    isON = true;
    ledon_ = isON;

}

void ButtonLED::turnOFF()
{
    isON = false;
    ledon_ = isON;

}



bool ButtonLED::getState() //getter function, so nothing else can change isON
{
    return isON;
}

bool ButtonLED::RisingEdge()    //access switch rising and falling edges
{
    return sw.RisingEdge();
}

bool ButtonLED::FallingEdge()
{
    return sw.FallingEdge();
}




//ButtonSW Functions:

void ButtonSW::init(dsy_gpio_pin switch_pin, SwitchType switchtype, float Samplerate)
{
    sw.Init(switch_pin, Samplerate);
    switchtype_ = switchtype;
    isON = false; 
}

void ButtonSW::update() //check switch and update LED
{
sw.Debounce();
switch (switchtype_)
{
    case Momentary:
        if(sw.RisingEdge())
        {
            toggle();
        }
    break;

    case Toggle:
        if(!sw.Pressed())
        {
            turnON();
        } 
        else
        {
            turnOFF();
        }
    break;
    case Toggle_inverted:
        if(sw.Pressed())
        {
            turnON();
        } 
        else
        {
            turnOFF();
        }
    break;
    default:
    break;
}

}

void ButtonSW::toggle()
{
    isON = !isON;
}

void ButtonSW::turnON()
{
    isON = true;
}

void ButtonSW::turnOFF()
{
    isON = false;
}

bool ButtonSW::getState() //getter function, so nothing else can change isON
{
    return isON;
}

bool ButtonSW::RisingEdge()    //access switch rising and falling edges
{
    return sw.RisingEdge();
}

bool ButtonSW::FallingEdge()
{
    return sw.FallingEdge();
}

