#include <Arduino.h>
#include "songSelector.h"

SongPlayer::SongPlayer() 
{

}

void SongPlayer::play() {
    
}

void SongPlayer::pausePlay() {
    if(state == S_PLAYING)
    {
        pause();
        state = S_PAUSED;
    }
    else if( state == S_IDLE || state == S_PAUSED) 
    {
        play();
        state = S_PLAYING;
    }
}

