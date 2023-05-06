#include <Arduino.h>
#include "songSelector.h"

SongSelector::SongSelector()
{
    
}

void SongSelector::indexSong(int num) {
    _currentSelectedSong = (_currentSelectedSong + num) % _listlength;
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
}

String SongSelector::getSelectedSong() {
    return (String) _listOfSongs[_currentSelectedSong];
}

void SongSelector::nextSong() {
    indexSong(1);
}

void SongSelector::prevSong() {
    indexSong(_listlength - 1);
}

void SongSelector::selectSong() {
    _currentSong = _currentSelectedSong;
}

char * SongSelector::getPlayingSong() {
    return _listOfSongs[_currentSong];
}

uint8_t SongSelector::getChannel() {
    return _currentChannel;
}

void SongSelector::setChannel(int channel) {
    if(channel <= 16)
    {
        _currentChannel = channel;
    }
}
