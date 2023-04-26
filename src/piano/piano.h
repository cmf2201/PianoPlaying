#pragma once

#include <FastLED.h>

// the max number of (reasonable) keys on the piano
#define MAX_NUM_KEYS 127

// This is the number of LEDS in the Piano array.
#define NUM_LEDS 13
#define NUM_ACTIVE_KEYS 12

#define ARRAY_LED_PIN 4

/**
 * structure to hold data for keys.
 * 
 * 
 */
typedef struct 
{
    boolean state;     //the state of the current key (on or off)
    uint8_t pitch;     //the pitch of the current key (the 'note')
    uint8_t velocity;  //velocity of the current key at the given moment.
    int last_update;   //tick of last update
} key;

//LED Array class to handle the acutal turning on/off of the LED's.
class LEDArray
{

    private:
    CRGB * _RGBleds;
    CHSV * _leds;

    public:
    LEDArray();

    void begin(CRGB * RGBLeds, CHSV * leds) {_RGBleds = RGBLeds, _leds = leds;}
    //updates all current keys on the piano, ensuring the most recent of a particular key is on
    void updateKeys(key * k);

    //updates the given list of keys (and all different Pitches of that key) on the piano
    void updateKey(key * k, int pitch);

    // helper function to return the most active of a list of given keys
    // keep track of most recent tick to figure out which notes have priority. Here is the overall priority:
    // if 2 notes are on at once:
    // last note turned off < last note activated
    key mostRecent(key * k, int pitch);

    void displayLEDS(void);

    //converts the LEDS in the HSV array to the RGB array
    void convertLeds(void);

    void turnOnKey(int pitch);
    
    void turnOffKey(int pitch);

};

//Piano class to keep track of the midi events
class Piano
{
    private:
    key _keys[MAX_NUM_KEYS];
    
    public:
    Piano();

    void begin(CRGB *RGBLeds, CHSV *led);

    // Fucntion to update a key given a particular MIDI message
    void updateKey(uint8_t code, uint8_t pitch, uint8_t velocity, long tick);

    // Function to update the visual Keyboard
    void updateLEDS();

    // Function to retrieve data from each key in the class
    key getKey(uint8_t pitch);

    //Function to initalize the keys and assign them all their approriate pitch value
    void initKeys();

    void turnOnKey(int pitch);

    void turnOffKey(int pitch);


    private:
    LEDArray _LEDArray;
};
