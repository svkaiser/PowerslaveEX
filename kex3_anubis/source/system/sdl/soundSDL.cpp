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
// DESCRIPTION:
//      Sound System (SDL)
//

#include "SDL.h"
#include "SDL_mixer.h"
#include "kexlib.h"

class kexSoundSDL : public kexSound
{
public:
    kexSoundSDL();
    ~kexSoundSDL();

    virtual void        Init(void);
    virtual void        Shutdown(void);

    static const int    NUM_CHANNELS;

private:
    int                 realFreq;
    Uint16              realFormat;
    int                 realChannels;
};

static kexSoundSDL soundSystem;
kexSound *kex::cSound = &soundSystem;

const int kexSoundSDL::NUM_CHANNELS = 64;

//
// kexSoundSDL::kexSoundSDL
//

kexSoundSDL::kexSoundSDL(void)
{
}

//
// kexSoundSDL::~kexSoundSDL
//

kexSoundSDL::~kexSoundSDL(void)
{
}

//
// kexSoundSDL::Init
//

void kexSoundSDL::Init(void)
{
    if(SDL_Init(SDL_INIT_AUDIO) < 0)
    {
        return;
    }

    if(Mix_OpenAudio(kexSound::cvarSampleRate.GetInt(), AUDIO_S16SYS, 2, GetSliceBufferSize()) < 0)
    {
        return;
    }

    Mix_QuerySpec(&realFreq, &realFormat, &realChannels);
    Mix_AllocateChannels(NUM_CHANNELS);

    bInitialized = true;
}

//
// kexSoundSDL::Shutdown
//

void kexSoundSDL::Shutdown(void)
{
    if(!bInitialized)
    {
        return;
    }

    Mix_CloseAudio();
    SDL_QuitSubSystem(SDL_INIT_AUDIO);

    bInitialized = false;
}
