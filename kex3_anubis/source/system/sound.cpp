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
//      Base Sound API
//

#include "kexlib.h"

kexCvar kexSound::cvarSampleRate("snd_samplerate", CVF_INT|CVF_CONFIG, "22050", "Sound sample rate to use for digital output (Hz)");
kexCvar kexSound::cvarSliceTime("snd_maxslicetime_ms", CVF_INT|CVF_CONFIG, "16", "Sound buffer size passed per tick");
kexCvar kexSound::cvarVolume("snd_volume", CVF_FLOAT|CVF_CONFIG, "1", 0, 1, "Sound volume");
kexCvar kexSound::cvarMusicVolume("snd_musicvolume", CVF_FLOAT|CVF_CONFIG, "1", 0, 1, "Music Volume");

//
// kexSound::kexSound
//

kexSound::kexSound(void)
{
    this->bInitialized = false;
}

//
// kexSound::Init
//

void kexSound::Init(void)
{
}

//
// kexSound::Shutdown
//

void kexSound::Shutdown(void)
{
}

//
// kexSound::Update
//

void kexSound::Update(void)
{
}

//
// kexSound::GetSliceBufferSize
//

int kexSound::GetSliceBufferSize(void)
{
    int time = cvarSampleRate.GetInt() * cvarSliceTime.GetInt();

    return kexMath::RoundPowerOfTwo(kexMath::MSec2Sec(time));
}

//
// kexSound::UpdateSource
//

void kexSound::UpdateSource(const int handle, const int volume, const int sep)
{
}

//
// kexSound::Play
//

void kexSound::Play(void *data, const int volume, const int sep, kexObject *ref, bool bLooping)
{
}

//
// kexSound::Stop
//

void kexSound::Stop(const int handle)
{
}

//
// kexSound::PlayMusic
//

void kexSound::PlayMusic(const char *name, const bool bLoop)
{
}

//
// kexSound::StopMusic
//

void kexSound::StopMusic(void)
{
}

//
// kexSound::Playing
//

bool kexSound::Playing(const int handle)
{
    return false;
}

//
// kexSound::SourceLooping
//

bool kexSound::SourceLooping(const int handle)
{
    return false;
}

//
// kexSound::NumSources
//

const int kexSound::NumSources(void) const
{
    return -1;
}

//
// kexSound::GetRefObject
//

kexObject *kexSound::GetRefObject(const int handle)
{
    return NULL;
}

//
// kexSound::HookToMovieAudioStream
//

void kexSound::HookToMovieAudioStream(const int sampleRate, const int channels)
{
}

//
// kexSound::UnHookMovieAudioStream
//

void kexSound::UnHookMovieAudioStream(void)
{
}
