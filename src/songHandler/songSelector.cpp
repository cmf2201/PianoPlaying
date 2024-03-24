#include <Arduino.h>
#include "songSelector.h"

SongSelector::SongSelector()
{

}

void SongSelector::indexSong(uint8_t num) {
    _currentSelectedSong = (_currentSelectedSong + num) % _listlength;
}

void SongSelector::indexChan(uint8_t num) {
    _currentChannel = (_currentChannel + num) % MAX_NUM_CHANNELS;
}


void SongSelector::setSongs(char *allSongs[],int listlength) {
    for(int i =0;i < MAX_NUM_SONGS; i++)
    {
        if(i < listlength)
        {
            _listOfSongs[i] = allSongs[i];
        } 
        else 
        {
            _listOfSongs[i] = &_empty;
        }
    }
    _listlength = listlength;
}

char * SongSelector::getSelectedSong() {
    return _listOfSongs[_currentSelectedSong];
}

void SongSelector::nextSong() {
    indexSong(1);
}

void SongSelector::prevSong() {
    indexSong(_listlength - 1);
}

void SongSelector::nextChan() {
    indexChan(1);
}

void SongSelector::prevChan() {
    indexChan(MAX_NUM_CHANNELS - 1);
}

void SongSelector::selectSong() {
    _currentSong = _currentSelectedSong;
}

char * SongSelector::getPlayingSong() {
    return _listOfSongs[_currentSong];
}

bool * SongSelector::getActChan() {
    return _activeChan;
}

void SongSelector::addChan(int chan) {
    if(_activeChan[chan] == false) {
        _activeChan[chan] = true;
    }
}

void SongSelector::resetActChan() {
    for(int i = 0; i < MAX_NUM_CHANNELS; i++) {
        _activeChan[i] = false;
    }
}

uint8_t SongSelector::getChannel() {
    return _currentChannel;
}
uint8_t SongSelector::getPlayIndex() {
    return _currentSong;
}

uint8_t SongSelector::getSelIndex() {
    return _currentSelectedSong;;
}

void SongSelector::setChannel(int channel) {
    if(channel <= 16)
    {
        _currentChannel = channel;
    }
}
