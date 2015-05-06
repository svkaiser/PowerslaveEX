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

#ifndef __SOUND_H__
#define __SOUND_H__

class kexSound
{
public:
    kexSound(void);

    virtual void            Init(void);
    virtual void            Shutdown(void);
    virtual void            Update(void);
    virtual void            UpdateSource(const int handle, const int volume, const int sep);
    virtual void            Play(void *data, const int volume, const int sep,
                                 kexObject *ref = NULL, bool bLooping = false);
    virtual void            Stop(const int handle);
    virtual bool            Playing(const int handle);
    virtual bool            SourceLooping(const int handle);
    virtual const int       NumSources(void) const;
    virtual kexObject       *GetRefObject(const int handle);
    virtual void            PlayMusic(const char *name, const bool bLoop = true);
    virtual void            StopMusic(void);
    virtual void            HookToMovieAudioStream(const int sampleRate, const int channels);
    virtual void            UnHookMovieAudioStream(void);

    static kexCvar          cvarSampleRate;
    static kexCvar          cvarSliceTime;
    static kexCvar          cvarVolume;
    static kexCvar          cvarMusicVolume;

protected:
    int                     GetSliceBufferSize(void);

    bool                    bInitialized;
};

#endif
