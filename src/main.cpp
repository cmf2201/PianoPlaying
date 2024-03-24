#include <Arduino.h>
#include <FastLED.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <IRremote.hpp>
#include "remoteControl/RemoteConstants.h"
#include <SdFat.h>
#include <MD_MIDIFile.h>
#include "piano/piano.h"
#include "songHandler/songSelector.h"

//MIDI LIBRARY
// SD chip select pin for SPI comms.
// Default SD chip select is the SPI SS pin (10 on Uno, 53 on Mega).
const uint8_t SD_SELECT = 5;
const uint16_t WAIT_DELAY = 2000; // ms

#define ARRAY_SIZE(a) (sizeof(a)/sizeof((a)[0]))

#define DEBUG(s, x)  do { Serial.print(F(s)); Serial.print(x); } while(false)
#define DEBUGX(s, x) do { Serial.print(F(s)); Serial.print(F("0x")); Serial.print(x, HEX); } while(false)
#define DEBUGS(s)    do { Serial.print(F(s)); } while (false)

#define SERIAL_RATE 57600


// The files in the tune list should be located on the SD card 
// or an error will occur opening the file and the next in the 
// list will be opened (skips errors).
static char *tuneList[] = 
{
  "LOOPDEMO.MID",  // simplest and shortest file
  "BANDIT.MID",
  "ELISE.MID",
  "TWINKLE.MID",
  "GANGNAM.MID",
  "FUGUEGM.MID",
  "POPCORN.MID",
  "AIR.MID",
  "PRDANCER.MID",
  "MINUET.MID",
  "FIRERAIN.MID",
  "MOZART.MID",
  "FERNANDO.MID",
  "SONATAC.MID",
  "SKYFALL.MID",
  "XMAS.MID",
  "GBROWN.MID",
  "PROWLER.MID",
  "IPANEMA.MID",
  "JZBUMBLE.MID",
};

CRGB RGBleds[NUM_LEDS];
CHSV leds[NUM_LEDS];

// SD Card Declarations

SDFAT	SD;
MD_MIDIFile SMF;

// Piano Library Declarations
Piano piano;

// Song Selector Class Declaration
SongSelector songSelector;

//finite state machine Pause/play command
boolean irPausePlay = false;
boolean irStop = false;

#define ONBOARD_LED 2
void playingStateMachine();
uint8_t playState = 0;

void midiCallback(midi_event *pev)
// Called by the MIDIFile library when a file event needs to be processed
// thru the midi communications interface.
// This callback is set up in the setup() function.
{
  // DEBUG("\n", millis());
  // DEBUG("\tM T", pev->track);
  // DEBUG(":  Ch ", pev->channel+1);
  // DEBUGS(" Data");
  // for (uint8_t i=0; i<pev->size; i++)
  //   DEBUGX(" ", pev->data[i]);
  songSelector.addChan(pev->channel);
  if(pev->channel == songSelector.getChannel()){
    piano.updateKey(pev->data[0],pev->data[1],pev->data[2],millis());
    FastLED.show();
  }
}

void sysexCallback(sysex_event *pev)
// Called by the MIDIFile library when a system Exclusive (sysex) file event needs 
// to be processed through the midi communications interface. Most sysex events cannot 
// really be processed, so we just ignore it here.
// This callback is set up in the setup() function.
{
  DEBUG("\nS T", pev->track);
  DEBUGS(": Data");
  for (uint8_t i=0; i<pev->size; i++)
    DEBUGX(" ", pev->data[i]);
}

void midiSilence(void)
// Turn everything off on every channel.
// Some midi files are badly behaved and leave notes hanging, so between songs turn
// off all the notes and sound
{
  midi_event ev;

  // All sound off
  // When All Sound Off is received all oscillators will turn off, and their volume
  // envelopes are set to zero as soon as possible.
  ev.size = 0;
  ev.data[ev.size++] = 0x80;
  ev.data[ev.size++] = 0;
  ev.data[ev.size++] = 0;

  for (ev.channel = 0; ev.channel < 16; ev.channel++)
    midiCallback(&ev);
}



LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display
byte b_play[8] = {0b00000,0b01000,0b01100,0b01110,0b01110,0b01100,0b01000,0b00000};
byte b_pause[8] = {0b00000,0b01010,0b01010,0b01010,0b01010,0b01010,0b01010,0b00000};
byte b_stop[8] = {0b00000,0b11111,0b01010,0b00100,0b00100,0b01010,0b11111,0b00000};
void pianoPlaybackTest(uint16_t code);
void LCDReadings();



#define IR_PIN 14


int testKeys[] = {60,61,62,63,64,65,66,67,68,69,70,71,72,73};
int testVelocity[] = {};

int testVel = 255;
int currentKey = 0;

long debounceDelay = 1000;

void setup() {

  pinMode(ONBOARD_LED,OUTPUT);
    
  FastLED.addLeds<WS2812,ARRAY_LED_PIN,GRB>(RGBleds,NUM_LEDS);


  // put your setup code here, to run once:
  Serial.begin(57600);
  Serial.println("STARTING...");

  // Song Selector Startup
  songSelector.setSongs(tuneList,20);
  Serial.println(songSelector.getSelectedSong());

  //SD Card startup
  if (!SD.begin(SD_SELECT, SPI_SIXTEENTH_SPEED))
  {
    DEBUGS("\nSD init fail!");
    while (true) ;
  }

  // Initialize MIDIFile
  SMF.begin(&SD);
  SMF.setMidiHandler(midiCallback);
  SMF.setSysexHandler(sysexCallback);


  //LCD setup
  lcd.init();                      // initialize the lcd 
  lcd.backlight();
  lcd.createChar(0,b_play);
  lcd.createChar(1,b_pause);
  lcd.createChar(2,b_stop);
  

  //Piano Setup

  piano.begin(RGBleds,leds);

  IrReceiver.begin(IR_PIN);
}


void loop() {
  //deal with piano playing
  playingStateMachine();

  //IR Sensor readings
  if(millis() > debounceDelay) {
    if (IrReceiver.decode()) {

        /*
          * !!!Important!!! Enable receiving of the next value,
          * since receiving has stopped after the end of the current received data packet.
          */
        IrReceiver.resume();
        debounceDelay = millis() + 1000;
        Serial.println(IrReceiver.decodedIRData.command);
        Serial.println("READING");
        pianoPlaybackTest(IrReceiver.decodedIRData.command);
    }
  } else if(IrReceiver.available())
  {
    if(IrReceiver.decode()) {
      IrReceiver.resume();
    }
  }

  //Display LCD readings.
  LCDReadings();

      // lcd.clear();
      // lcd.print("C-1");
      // lcd.display();

}

// Deals with LCD readings, what shold be displayed.
void LCDReadings() 
{
  lcd.setCursor(0,0);
  const char* playingSong = songSelector.getPlayingSong();
  const char* selectedSong = songSelector.getSelectedSong();
  if(strcmp(playingSong,selectedSong) == 0)
  {
    switch(playState)
    {
      case 1:
        lcd.write((uint8_t)0);
        break;
      case 2:
        lcd.write((uint8_t)1);
        break;
      default:
        lcd.write((uint8_t)2);
        break;
    }
  }
  else{
    lcd.print(" ");
  }
  // FIX LATER FOR DYNAIC INDEX LENGTHS !!!!!
  int numLen = std::to_string((int)songSelector.getSelIndex()).length();
  for(int i = 0; i < 14 - numLen; i++)
  {
    if( i >= strlen(selectedSong)) {
      lcd.print(" ");
    } 
    else {
      lcd.print(selectedSong[i]);
    }
  }
  lcd.print(" ");
  lcd.print(songSelector.getSelIndex());
  lcd.setCursor(0,1);
  lcd.print("CH:");
  lcd.print(songSelector.getChannel());
  lcd.print(" A:");
  lcd.print(songSelector.getPlayIndex());
   switch(playState)
    {
      case 1:
        lcd.write((uint8_t)0);
        break;
      case 2:
        lcd.write((uint8_t)1);
        break;
      default:
        lcd.write((uint8_t)2);
        break;
    }
  lcd.print(" ");
  for(int i = 0; i < 5; i++)
  {
    if( i >= strlen(playingSong)) {
      lcd.print(" ");
    } 
    else {
      lcd.print(playingSong[i]);
    }
  }
  lcd.display();


}

// Deals with playing songs, whether to idle, stop, play, etc.
void playingStateMachine() {
  static enum { S_IDLE, S_PLAYING, S_PAUSED, S_END } state = S_IDLE;

  switch (state)
  {
  case S_IDLE:    // now idle, wait for play signal.
    if(irPausePlay)
    {
      irPausePlay = false;
      int err;
      //attempt to load the current song
      err = SMF.load(songSelector.getPlayingSong());
      if (err != MD_MIDIFile::E_OK)
      {
        DEBUG(" - SMF load Error ", err);
        // digitalWrite(SMF_ERROR_LED, HIGH);
        state = S_IDLE;
        DEBUGS("\nS_IDLE");
      }
      else
      {
        DEBUGS("\nS_PLAYING");
        state = S_PLAYING;
      }
    }
    break;

  case S_PLAYING: // play the file
    // DEBUGS("\nS_PLAYING");
    if (!SMF.isEOF() && !irStop)
    {
      if(irPausePlay) 
      {
        irPausePlay = false;
        SMF.pause(true);
        state = S_PAUSED;
      }
      else if (SMF.getNextEvent());
        // tickMetronome();
        // Serial.println("TEST: SONG IN PROGRESS");
    }
    else
    {
      irStop = false;
      state = S_END;
    }
    break;

  case S_PAUSED:
    // if song is unplaused, go back to playing state
    if(irPausePlay)
    {
      irPausePlay = false;
      SMF.pause(false);
      state = S_PLAYING;
    } 
    // if song is stopped, go to end state.
    else if(irStop)
    {
      irStop = false;
      SMF.pause(false);
      state = S_END;
    }

    break;

  case S_END:   // done with this one
    DEBUGS("\nS_END");
    piano.resetKeys();
    SMF.close();
    midiSilence();
    FastLED.show();
    songSelector.resetActChan();
    state = S_IDLE;
    // DEBUGS("\nWAIT_BETWEEN");
    break;

  default:
    state = S_IDLE;
    break;
  }
  playState = state;
}

//decideds what to do based on each button press from the IR Remote
void pianoPlaybackTest(uint16_t code) {
  switch(code) {
    case remoteUp:
      songSelector.nextSong();
      Serial.println(songSelector.getSelectedSong());
      break;

    case remoteDown:
      songSelector.prevSong();
      Serial.println(songSelector.getSelectedSong());
      break;

    case remoteRight:
      bool act;
      bool * active;
      int tries;
      tries = 0;
      // check for the next active channel
      do {
        songSelector.nextChan();
        active = songSelector.getActChan();
        act = active[songSelector.getChannel()];
        Serial.println(act);
        tries++;
      }
      while (!act && tries < MAX_NUM_CHANNELS);

      piano.resetKeys();
      Serial.println(songSelector.getChannel());
      FastLED.show();
      break;

    case remoteLeft:
      tries = 0;
      //check for the prev active channel
      do {
      songSelector.prevChan();
      active = songSelector.getActChan();
      act = active[songSelector.getChannel()];
      Serial.println(act);
      tries++;
      }
      while (!act && tries < MAX_NUM_CHANNELS);

      piano.resetKeys();
      Serial.println(songSelector.getChannel());
      FastLED.show();
      break;

    case remoteEnterSave:
      songSelector.selectSong();
      irStop = true;
      break;
    
    case remotePlayPause:
      irPausePlay = true;
      break;

    case remoteStopMode:
      irStop = true;
      break;
    case remote0:
      piano.resetKeys();
      break;
    case remote1:
      piano.turnOnKey(60);
      break;
    case remote2:
      piano.turnOffKey(60);
      break;
    case remote3:
      piano.updateLEDS();
      break;
    case remote4:
      // piano.updateKey(0x80,60,255,millis());
      // piano.updateLEDS();
      FastLED.show();
      break;
      
    case remote5:
      piano.turnOnKey(72);
      piano.turnOnKey(73);
      break;
  }
}


