#pragma once
#include <Arduino.h>



#define MAX_NUM_SONGS 30
#define MAX_NUM_CHANNELS 16


class SongSelector
{
    private:
    
    char *_listOfSongs[MAX_NUM_SONGS];
    bool _activeChan[MAX_NUM_CHANNELS];
    uint8_t _currentSelectedSong = 0;
    uint8_t _currentSong = 0;
    uint8_t _currentChannel = 0;
    uint8_t _listlength = 0;
    char _empty = ' ';

    protected:
    /* Adds num to the current index of the song, and modulo's the index if greater than or equal to the max number of songs.
    */
    void indexSong(uint8_t num);
    /* Adds num to the current index of the channel, and modulo's the index if greater than or equal to the max number of channels.
    */
    void indexChan(uint8_t num);
    // current playing state of the song
    static enum { S_IDLE, S_PAUSED, S_PLAYING, S_END} state;

    public:
    SongSelector();
    // returns the name of the current selected Song
    char * getSelectedSong();
    // moves the index to the next song
    void nextSong();
    //moves the index to the previous song
    void prevSong();
    // moves the index to the next channel
    void nextChan();
    //moves the index to the previous channel
    void prevChan();
    //sends the current selected song to be played in the song player.
    void selectSong();
    //sets the list Of songs to the given list of char*
    void setSongs(char *allSongs[],int listlength);
    //returns playing song
    char * getPlayingSong();
    //returns all active channels
    bool * getActChan();
    //adds a channel to the active channels
    void addChan(int chan);
    //resets active channels
    void resetActChan();
    // returns current channel
    uint8_t getChannel();
    // returns playing song index
    uint8_t getPlayIndex();
    // returns selected song index
    uint8_t getSelIndex();
    // sets current channel
    void setChannel(int channel);

};