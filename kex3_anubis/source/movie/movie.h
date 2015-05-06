//
// Copyright(C) 2014-2015 Samuel Villarreal
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//

#ifndef __MOVIE_H__
#define __MOVIE_H__

#define MOVIE_AUDIO_BUFFER_SIZE     4096
#define MOVIE_AUDIO_BUFFER_COUNT    4

class kexMoviePlayer
{
public:
    kexMoviePlayer(void) {};
    ~kexMoviePlayer(void) {};

    virtual void StartVideoStream(const char *filename) {}
    virtual byte *GetAudioBufferInQueue(bool &bFinished) { return NULL; }
    virtual bool HasVideoStarted(void) { return false; }
    
private:
};

#endif
