#include <Arduino.h>
#include "piano.h"

Piano::Piano()
{
    _LEDArray = LEDArray();
}

void Piano::begin(CRGB *RGBLeds, CHSV *leds) 
{
    initKeys();
    _LEDArray.begin(RGBLeds,leds);
}

void Piano::updateKey(uint8_t code, uint8_t pitch, uint8_t velocity,long tick)
{
    switch(code)
    {
        case 0x80:
        _keys[pitch].state = false;
        _keys[pitch].last_update = tick;
        break;

        case 0x90:
        _keys[pitch].state = true;
        _keys[pitch].velocity = velocity;
        _keys[pitch].last_update = tick;
        break;

        default:
        break;
    }

}

void Piano::initKeys(void) {
    for( int k = 0; k < MAX_NUM_KEYS; k++)
    {
        _keys[k].pitch = k;
    }
}

key Piano::getKey(uint8_t pitch)
{
    if(pitch < MAX_NUM_KEYS)
    {
        return _keys[pitch];
    }
    return _keys[0];
}

void Piano::updateLEDS() {
    _LEDArray.updateKeys(_keys);
    _LEDArray.displayLEDS();
}

void Piano::turnOnKey(int pitch) {
    _LEDArray.turnOnKey(pitch);
}

void Piano::turnOffKey(int pitch) {
    _LEDArray.turnOffKey(pitch);
}


