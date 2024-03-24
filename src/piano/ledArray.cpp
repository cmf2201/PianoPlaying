#include <Arduino.h>
#include <FastLED.h>
#include "piano.h"



LEDArray::LEDArray() 
{
    
}

void LEDArray::convertLeds(void) {
    for(int i = 0; i < NUM_LEDS; i++) {
        hsv2rgb_rainbow(_leds[i],_RGBleds[i]);
    }
}

void LEDArray::updateKeys(key * k) 
{
    for(int i = 0; i < NUM_ACTIVE_KEYS; i++)
    {
        updateKey(k,i);
    }
}

void LEDArray::updateKey(key * k, int pitch) 
{
    key activeKey = mostRecent(k,pitch);

    int octive = activeKey.pitch/12 - 1;

    if(pitch >= 5) 
    {
        pitch = pitch+1;
    }
    //!!! UPDATE THIS FUNCTION TO ACCOUNT FOR DIFFERENT OCTIVE RANGES DYNAMICALLY
    _leds[pitch] = CHSV(map(octive,0,8,0,255),255,activeKey.velocity*2);
}

void LEDArray::displayLEDS(void)
{
    convertLeds();
}

key LEDArray::mostRecent(key * keyList, int pitch) {
    long MRT = 0;
    key activeKey;
    activeKey.pitch = pitch;
    int size = ceil((MAX_NUM_KEYS - pitch)/12);
    for(int i = 0; i < size; i ++) {
        key curKey = keyList[i*12 + pitch];
        //first check to see if key is enabled
        if(curKey.state)
        {
            //then check to see if key is most recently enabled
            if(curKey.last_update >= MRT) 
            {
                activeKey = curKey;
                MRT = curKey.last_update;
            }
        }
    }
    return activeKey;
}



//FADE OFF skeleton code
/*
void fadeOff(int key) {
  //DELETE LATER
  key = key + currentKey;
  //-------
  int octive = (key/12) -1;
  key = key%12;
  if(key >= 5) 
  {
    key = key+1;
  }
  int fadeVal = leds[key].v;
  fadeVal = fadeVal - 5;
  if(fadeVal <= 5) {
    fadeVal = 0; 
  }
  leds[key] = leds[key].setHSV(leds[key].h,leds[key].s,fadeVal);
  showLEDS();

}
*/