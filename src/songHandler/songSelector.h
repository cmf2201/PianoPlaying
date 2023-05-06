#pragma once
#include <Arduino.h>



#define MAX_NUM_SONGS 30


class SongSelector
{
    private:
    
    char *_listOfSongs[MAX_NUM_SONGS];
    uint8_t _currentSelectedSong;
    uint8_t _currentSong;
    uint8_t _currentChannel;
    uint8_t _listlength;
    char _empty = ' ';

    protected:
    /* Adds num to the current index of the song, and modulo's the index if greater than or equal to the max number of songs.
    */
    void indexSong(int num);
    // current playing state of the song
    static enum { S_IDLE, S_PAUSED, S_PLAYING, S_END} state;

    public:
    SongSelector();
    // returns the name of the current selected Song
    String getSelectedSong();
    // moves the index to the next song
    void nextSong();
    //moves the index to the previous song
    void prevSong();
    //sends the current selected song to be played in the song player.
    void selectSong();
    // sets the list Of songs to the given list of char*
    void setSongs(char *allSongs[],int listlength);
    //returns playing song
    char * getPlayingSong();
    // returns current channel
    uint8_t getChannel();
    // sets current channel
    void setChannel(int channel);

};