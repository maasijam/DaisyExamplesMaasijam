#include "LedMatrix.h"


#include <cstring>



void LedMatrixx::init(srtest::ShiftRegister595* sr, bool invertLeds) {
	_sr = sr;
    _invertLeds = invertLeds;
    std::memset(_ledState, 0, sizeof(_ledState));
}

void LedMatrixx::process() {
    uint8_t rowData = ~(1 << _row);
    uint8_t ledData = 0;
    int activeIndex  = 0;

     _sr->SetMatrix(rowData,true);
    _sr->WriteMatrix();
    for (int col = 0; col < ColsLed; ++col) {
        int index = col * Rows + _row;
        if (_ledState[index].red.update()) {
            ledData |= (1 << (col * 2 + 1));
            activeIndex = index;
        }
        if (_ledState[index].green.update()) {
            ledData |= (1 << (col * 2));
            activeIndex = index;
        }
        _sr->SetMatrix(activeIndex,true);
        _sr->WriteMatrix();
        
    }

   
    

    //_sr->WriteOutput(0, rowData);
    //_sr->WriteOutput(1, ledData);


    _row = (_row + 1) % Rows;
}
