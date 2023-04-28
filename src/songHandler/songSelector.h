#pragma once
#include <Arduino.h>



#define MAX_NUM_SONGS 30


class SongPlayer
{
    private:
    char *currentSong;
    uint8_t currentChannel;
    // current playing state of the song
    protected:
    static enum { S_IDLE, S_PAUSED, S_PLAYING, S_END} state;
    // pauses the song.
    void pause();
    // plays the song
    void play();

    public:
    SongPlayer();
    //pauses or plays the song depending on the state.
    void pausePlay();
    // stops the song
    void stop();
    // stops the previous song, and loads the given song as the new current song.
    void load(char *fname);
    // returns the current song playing
    char * getSong();
    // returns the current channel
    uint8_t getChannel();
    // sets current channel
    void setChannel(int channel);
};

class SongSelector
{
    private:
    
    char *_listOfSongs[MAX_NUM_SONGS];
    int _currentSelectedSong;
    int _listlength;
    char _empty = ' ';

    protected:
    /* Adds num to the current index of the song, and modulo's the index if greater than or equal to the max number of songs.
    */
    void indexSong(int num);

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
    // plays / pauses current song

    // stops current song

    private:
    SongPlayer _SongPlayer;

};