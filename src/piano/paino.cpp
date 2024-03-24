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
    updateLEDS();

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
    _keys[pitch].state = true;
    _keys[pitch].velocity = 255;
    _keys[pitch].last_update = millis();
    updateLEDS();
}

void Piano::turnOffKey(int pitch) {
    _keys[pitch].state = false;
    _keys[pitch].last_update = millis();
    updateLEDS();
}

void Piano::resetKeys() {
    for(int i = 0; i < MAX_NUM_KEYS;i++) {
        _keys[i].last_update = 0;
        _keys[i].state = false;
        _keys[i].velocity = 0;
    }
    updateLEDS();
}


