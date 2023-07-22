#pragma once


#include "../daisy_saul.h"
#include "daisysp.h"
#include "sr_595.h"

#include <array>
#include <utility>

#include <cstdint>
#include <cstdlib>

using namespace daisy;
using namespace daisysp;
using namespace std;

// Button Led Matrix
#define CONFIG_BLM_ROWS                 8
#define CONFIG_BLM_COLS_LED             3

class LedMatrixx
{
	public:
    
	LedMatrixx() {}
	~LedMatrixx() {} 

    static constexpr int Rows = CONFIG_BLM_ROWS;
    static constexpr int ColsLed = CONFIG_BLM_COLS_LED;

    

    void init(srtest::ShiftRegister595* sr, bool invertLeds = false);

    void setLed(int index, uint8_t red, uint8_t green) {
        if (_invertLeds) {
            std::swap(red, green);
        }
        _ledState[index].red.intensity = red >> 4;
        _ledState[index].green.intensity = green >> 4;
        if (red == 0) {
            _ledState[index].red.counter = 0;
        }
        if (green == 0) {
            _ledState[index].green.counter = 0;
        }
    }

    inline void setLed(int row, int col, uint8_t red, uint8_t green) {
        setLed(col * Rows + row, red, green);
    }

    void setLeds(const std::array<std::pair<uint8_t, uint8_t>, Rows * ColsLed> &leds) {
        for (size_t i = 0; i < leds.size(); ++i) {
            setLed(i, leds[i].first, leds[i].second);
        }
    }

    

    void process();

    

	private:
		struct Led {
			uint8_t intensity : 4;
			uint8_t counter : 4;
			inline bool update() {
				// return true;
				uint8_t sum = counter + intensity;
				//bool active = sum >= 0x0f;
                bool active = intensity > 0;
				counter = sum & 0x0f;
				return active;
			};
		};

		struct LedState {
			Led red;
			Led green;
		} __attribute__((packed));

		
		srtest::ShiftRegister595* _sr;
        
		bool _invertLeds;

		LedState _ledState[Rows * ColsLed];

		uint8_t _row = 0;
};
