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

int activeChannel = 1;

#define ONBOARD_LED 2

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
  if(pev->channel+1 == activeChannel){
    piano.updateKey(pev->data[0],pev->data[1],pev->data[2],millis());
    piano.updateLEDS();
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
  ev.data[ev.size++] = 0xb0;
  ev.data[ev.size++] = 120;
  ev.data[ev.size++] = 0;

  for (ev.channel = 0; ev.channel < 16; ev.channel++)
    midiCallback(&ev);
}



LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display

void pianoPlaybackTest(uint16_t code);



#define IR_PIN 14


int testKeys[] = {60,61,62,63,64,65,66,67,68,69,70,71,72,73};
int testVelocity[] = {};

int testVel = 255;
int currentKey = 0;

long debounceDelay = 0;

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
  // if (!SD.begin(SD_SELECT, SPI_SIXTEENTH_SPEED))
  // {
  //   DEBUGS("\nSD init fail!");
  //   while (true) ;
  // }

  // Initialize MIDIFile
  SMF.begin(&SD);
  SMF.setMidiHandler(midiCallback);
  SMF.setSysexHandler(sysexCallback);


  //LCD setup
  lcd.init();                      // initialize the lcd 
  lcd.backlight();

  //Piano Setup

  piano.begin(RGBleds,leds);

  IrReceiver.begin(IR_PIN);
}

// void tickMetronome(void)
// // flash a LED to the beat
// {
//   static uint32_t lastBeatTime = 0;
//   static boolean  inBeat = false;
//   uint16_t  beatTime;

//   beatTime = 60000/SMF.getTempo();    // msec/beat = ((60sec/min)*(1000 ms/sec))/(beats/min)
//   if (!inBeat)
//   {
//     if ((millis() - lastBeatTime) >= beatTime)
//     {
//       lastBeatTime = millis();
//       digitalWrite(ONBOARD_LED, HIGH);
//       inBeat = true;
//     }
//   }
//   else
//   {
//     if ((millis() - lastBeatTime) >= 100)	// keep the flash on for 100ms only
//     {
//       digitalWrite(ONBOARD_LED, LOW);
//       inBeat = false;
//     }
//   }
// }

void loop() {

  static enum { S_IDLE, S_PLAYING, S_END, S_WAIT_BETWEEN } state = S_IDLE;
  static uint16_t currTune = ARRAY_SIZE(tuneList);
  static uint32_t timeStart;

  switch (state)
  {
  case S_IDLE:    // now idle, set up the next tune
    {
      int err;

      DEBUGS("\nS_IDLE");

      // digitalWrite(READY_LED, LOW);
      // digitalWrite(SMF_ERROR_LED, LOW);

      currTune++;
      if (currTune >= ARRAY_SIZE(tuneList))
        currTune = 0;

      // use the next file name and play it
      DEBUG("\nFile: ", tuneList[currTune]);
      err = SMF.load(tuneList[currTune]);
      if (err != MD_MIDIFile::E_OK)
      {
        DEBUG(" - SMF load Error ", err);
        // digitalWrite(SMF_ERROR_LED, HIGH);
        timeStart = millis();
        state = S_WAIT_BETWEEN;
        DEBUGS("\nWAIT_BETWEEN");
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
    if (!SMF.isEOF())
    {
      if (SMF.getNextEvent());
        // tickMetronome();
        // Serial.println("TEST: SONG IN PROGRESS");
    }
    else
      state = S_END;
    break;

  case S_END:   // done with this one
    DEBUGS("\nS_END");
    SMF.close();
    midiSilence();
    timeStart = millis();
    state = S_WAIT_BETWEEN;
    DEBUGS("\nWAIT_BETWEEN");
    break;

  case S_WAIT_BETWEEN:    // signal finished with a dignified pause
    // digitalWrite(READY_LED, HIGH);
    if (millis() - timeStart >= WAIT_DELAY)
      state = S_IDLE;
    break;

  default:
    state = S_IDLE;
    break;
  }

  //LCD monitor print
  if(millis() > debounceDelay) {
  if (IrReceiver.decode()) {
      /*
        * !!!Important!!! Enable receiving of the next value,
        * since receiving has stopped after the end of the current received data packet.
        */
      IrReceiver.resume(); // Enable receiving of the next value
      debounceDelay = millis() + 500;
      /*
        * Finally, check the received data and perform actions according to the received command
        */
  }
  }

}

void pianoPlaybackTest(uint16_t code) {
  switch(code) {
    case remote0:
      lcd.clear();
      lcd.print("C-1");
      lcd.display();
      break;

    case remote1:
      lcd.clear();
      lcd.print("C0");
      lcd.display();
      break;

    case remote2:
      lcd.clear();
      lcd.print("C1");
      lcd.display();
      break;

    case remote3:
      lcd.clear();
      lcd.print("C2");
      lcd.display();
      break;

    case remote4:
      lcd.clear();
      lcd.print("C3");
      lcd.display();
      break;

    case remote5:
      lcd.clear();
      lcd.print("C4");
      lcd.display();
      break;

    case remote6:
      lcd.clear();
      lcd.print("C5");
      lcd.display();
      break;

    case remote7:
      lcd.clear();
      lcd.print("C6");
      lcd.display();
      break;

    case remote8:
      lcd.clear();
      lcd.print("C7");
      lcd.display();
      break;

    case remote9:
      lcd.clear();
      lcd.print("OFF");
      lcd.display();
      break;

    case remoteUp:
      if(activeChannel < 20) {
        activeChannel = activeChannel+1;
      }
      Serial.println(activeChannel);
      break;

    case remoteDown:
      if(activeChannel > 0) {
        activeChannel = activeChannel-1;
      }
      Serial.println(activeChannel);
      break;

    case remoteRight:
      currentKey = (currentKey + 1)%12;
      lcd.clear();
      lcd.print(currentKey);
      lcd.display();
      break;

    case remoteLeft:
      currentKey = (currentKey + 11)%12;
      lcd.clear();
      lcd.print(currentKey);
      lcd.display();
      break;

  }
}


