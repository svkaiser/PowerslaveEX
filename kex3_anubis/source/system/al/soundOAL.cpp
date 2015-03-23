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
//      Sound System (OpenAL)
//

#include "al.h"
#include "alc.h"

#include "kexlib.h"
#include "gameObject.h"

//-----------------------------------------------------------------------------
//
// Wave File Class
//
//-----------------------------------------------------------------------------

class kexWavFile
{
public:
    kexWavFile(void);
    ~kexWavFile(void);

    int                                 GetFormat(void);
    bool                                CompareTag(const char *tag, int offset);
    void                                Allocate(const char *name, byte *data);
    void                                Delete(void);
    ALuint                              *GetBuffer(void) { return &buffer; }

    kexStr                              filePath;
    kexWavFile                          *next;

private:
    short                               formatCode;
    short                               channels;
    int                                 samples;
    int                                 bytes;
    short                               blockAlign;
    short                               bits;
    int                                 waveSize;
    byte                                *data;
    byte                                *waveFile;
    ALuint                              buffer;
};

//-----------------------------------------------------------------------------
//
// Sound Source
//
//-----------------------------------------------------------------------------

class kexSoundSource
{
    friend class kexSoundOAL;
public:
    kexSoundSource(void);
    ~kexSoundSource(void);

    bool                                Generate(void);
    void                                Play(void);
    void                                Stop(void);
    void                                Reset(void);
    void                                Free(void);
    void                                Delete(void);
    void                                Update(void);
    void                                UpdateParameters(void);

    bool                                InUse(void) const { return bInUse; }
    bool                                &Looping(void) { return bLooping; }

private:
    ALuint                              handle;
    int                                 startTime;
    bool                                bInUse;
    bool                                bPlaying;
    bool                                bLooping;
    float                               volume;
    float                               pan;
    float                               pitch;
    kexWavFile                          *wave;
    kexObject                           *refObject;
};

//-----------------------------------------------------------------------------
//
// OpenAL System
//
//-----------------------------------------------------------------------------

class kexSoundOAL : public kexSound
{
public:
    kexSoundOAL();
    ~kexSoundOAL();

    virtual void                    Init(void);
    virtual void                    Shutdown(void);
    virtual bool                    Playing(const int handle);
    virtual bool                    SourceLooping(const int handle);
    virtual void                    Play(void *data, const int volume, const int sep,
                                         kexObject *ref = NULL, bool bLooping = false);
    virtual void                    Stop(const int handle);
    virtual void                    UpdateSource(const int handle, const int volume, const int sep);
    virtual void                    Update(void);
    virtual const int               NumSources(void) const;
    virtual kexObject               *GetRefObject(const int handle);

    char                            *GetDeviceName(void);
    const int                       GetNumActiveSources(void) const { return activeSources; }

    static const int                SND_MAX_SOURCES;
    static kexHeapBlock             hb_sound;

private:
    kexSoundSource                  *GetAvailableSource(void);

    ALCdevice                       *alDevice;
    ALCcontext                      *alContext;
    kexSoundSource                  *sources;
    kexHashList<kexWavFile>         wavList;
    bool                            *sourcesActive;
    int                             activeSources;
};

static kexSoundOAL soundSystem;
kexSound *kex::cSound = &soundSystem;

const int kexSoundOAL::SND_MAX_SOURCES = 64;
kexHeapBlock kexSoundOAL::hb_sound("sound", false, NULL, NULL);

//
// printsoundinfo
//

COMMAND(printsoundinfo)
{
    kex::cSystem->CPrintf(COLOR_CYAN, "------------- Sound Info -------------\n");
    kex::cSystem->CPrintf(COLOR_GREEN, "Device: %s\n", soundSystem.GetDeviceName());
    kex::cSystem->CPrintf(COLOR_GREEN, "Available Sources: %i\n", soundSystem.GetNumActiveSources());
}

//-----------------------------------------------------------------------------
//
// Wave File Class Routines
//
//-----------------------------------------------------------------------------

//
// kexWavFile::kexWavFile
//

kexWavFile::kexWavFile(void)
{
}

//
// kexWavFile::~kexWavFile
//

kexWavFile::~kexWavFile(void)
{
}

//
// kexWavFile::CompareTag
//

bool kexWavFile::CompareTag(const char *tag, int offset)
{
    byte *buf = waveFile + offset;

    return
        (buf[0] == tag[0] &&
         buf[1] == tag[1] &&
         buf[2] == tag[2] &&
         buf[3] == tag[3]);
}

//
// kexWavFile::GetFormat
//

int kexWavFile::GetFormat(void)
{
    switch(channels)
    {
    case 1:
        switch(bits)
        {
        case 8:
            return AL_FORMAT_MONO8;
        case 16:
            return AL_FORMAT_MONO16;
        }
        break;
    case 2:
        switch(bits)
        {
        case 8:
            return AL_FORMAT_STEREO8;
        case 16:
            return AL_FORMAT_STEREO16;
        }
        break;
    default:
        kex::cSystem->Error("Snd_GetWaveFormat: Unsupported number of channels - %i", channels);
        return -1;
    }

    kex::cSystem->Error("Snd_GetWaveFormat: Unknown bits format - %i", bits);
    return -1;
}

//
// kexWavFile::Allocate
//

void kexWavFile::Allocate(const char *name, byte *data)
{
    filePath = name;
    waveFile = data;

    if(!CompareTag("RIFF", 0))
    {
        kex::cSystem->Error("kexWavFile::Allocate: RIFF header not found in %s", name);
    }
    if(!CompareTag("WAVE", 8))
    {
        kex::cSystem->Error("kexWavFile::Allocate: WAVE header not found in %s", name);
    }
    if(!CompareTag("fmt ", 12))
    {
        kex::cSystem->Error("kexWavFile::Allocate: fmt header not found in %s", name);
    }
    if(!CompareTag("data", 36))
    {
        kex::cSystem->Error("kexWavFile::Allocate: data header not found in %s", name);
    }

    if(*(data + 16) != 16)
    {
        kex::cSystem->Error("kexWavFile::Allocate: WAV chunk size must be 16 (%s)", name);
    }

    formatCode  = *(short*)(data + 20);
    channels    = *(short*)(data + 22);
    samples     = *(int*)(data + 24);
    bytes       = *(int*)(data + 28);
    blockAlign  = *(short*)(data + 32);
    bits        = *(short*)(data + 34);
    waveSize    = *(int*)(data + 40);
    data        = data + 44;

    alGetError();
    alGenBuffers(1, &buffer);

    if(alGetError() != AL_NO_ERROR)
    {
        kex::cSystem->Error("kexWavFile::Allocate: failed to create buffer for %s", name);
    }

    alBufferData(buffer, GetFormat(), data, waveSize, samples);
}

//
// kexWavFile::Delete
//

void kexWavFile::Delete(void)
{
    alDeleteBuffers(1, &buffer);
    buffer = 0;
}

//-----------------------------------------------------------------------------
//
// Sound Source Routines
//
//-----------------------------------------------------------------------------

//
// kexSoundSource::kexSoundSource
//

kexSoundSource::kexSoundSource(void)
{
}

//
// kexSoundSource::~kexSoundSource
//

kexSoundSource::~kexSoundSource(void)
{
}

//
// kexSoundSource::Generate
//

bool kexSoundSource::Generate(void)
{
    alGetError();
    alGenSources(1, &handle);

    if(alGetError() != AL_NO_ERROR)
    {
        return false;
    }

    startTime   = 0;
    bInUse      = false;
    bLooping    = false;
    bPlaying    = false;
    wave        = NULL;
    volume      = 1.0f;
    pan         = 0;
    refObject   = NULL;

    alSourcei(handle, AL_LOOPING, AL_FALSE);
    alSourcei(handle, AL_SOURCE_RELATIVE, AL_TRUE);
    alSourcef(handle, AL_GAIN, 1.0f);
    alSourcef(handle, AL_PITCH, 1.0f);
    return true;
}

//
// kexSoundSource::Stop
//

void kexSoundSource::Stop(void)
{
    alSourceStop(handle);

    if(wave)
    {
        alSourceUnqueueBuffers(handle, 1, wave->GetBuffer());
    }
}

//
// kexSoundSource::Free
//

void kexSoundSource::Free(void)
{
    bInUse      = false;
    bPlaying    = false;
    wave        = NULL;
    pan         = 0;
    volume      = 1.0f;
    pitch       = 1.0f;
    startTime   = 0;

    if(refObject && refObject->InstanceOf(&kexGameObject::info))
    {
        static_cast<kexGameObject*>(refObject)->RemoveRef();
    }

    refObject   = NULL;

    alSource3f(handle, AL_POSITION, 0, 0, 0);
}

//
// kexSoundSource::Delete
//

void kexSoundSource::Delete(void)
{
    if(bPlaying)
    {
        Stop();
        Free();
    }

    alSourcei(handle, AL_BUFFER, 0);
    alDeleteSources(1, &handle);

    handle = 0;
}

//
// kexSoundSource::Reset
//

void kexSoundSource::Reset(void)
{
    bInUse      = true;
    bPlaying    = false;
    volume      = 1.0f;
    pan         = 0;
    pitch       = 1.0f;
}

//
// kexSoundSource::UpdateParameters
//

void kexSoundSource::UpdateParameters(void)
{
    kexVec3 dir;

    kexVec3::ToAxis(&dir, 0, 0, kexMath::Deg2Rad(pan * 1.40625f), 0, 0);

    alSourcefv(handle, AL_POSITION, dir.ToFloatPtr());
    alSourcef(handle, AL_GAIN, volume);
}

//
// kexSoundSource::Play
//

void kexSoundSource::Play(void)
{
    alSourceQueueBuffers(handle, 1, wave->GetBuffer());
    alSourcei(handle, AL_SOURCE_RELATIVE, AL_TRUE);
    alSourcei(handle, AL_LOOPING, bLooping);

    UpdateParameters();
    alSourcePlay(handle);

    bPlaying = true;
}

//
// kexSoundSource::Update
//

void kexSoundSource::Update(void)
{
    ALint state;

    if(!bInUse)
    {
        return;
    }

    alGetSourcei(handle, AL_SOURCE_STATE, &state);
    if(state != AL_PLAYING)
    {
        Stop();
        Free();
    }
}

//-----------------------------------------------------------------------------
//
// OpenAL System Routines
//
//-----------------------------------------------------------------------------

//
// kexSoundOAL::kexSoundOAL
//

kexSoundOAL::kexSoundOAL(void)
{
    sourcesActive = new bool[SND_MAX_SOURCES];
    sources = new kexSoundSource[SND_MAX_SOURCES];

    memset(sourcesActive, 0, SND_MAX_SOURCES);
}

//
// kexSoundOAL::~kexSoundOAL
//

kexSoundOAL::~kexSoundOAL(void)
{
    delete[] sourcesActive;
    delete[] sources;
}

//
// kexSoundOAL::Init
//

void kexSoundOAL::Init(void)
{
    int i;

    alDevice = alcOpenDevice(NULL);
    if(!alDevice)
    {
        kex::cSystem->Error("kexSoundOAL::Init: Failed to create OpenAL device");
    }

    alContext = alcCreateContext(alDevice, NULL);
    if(!alContext)
    {
        kex::cSystem->Error("kexSoundOAL::Init: Failed to create OpenAL context");
    }

    if(!alcMakeContextCurrent(alContext))
    {
        kex::cSystem->Error("kexSoundOAL::Init: Failed to set current context");
    }

    for(i = 0; i < SND_MAX_SOURCES; ++i)
    {
        kexSoundSource *sndSrc = &sources[activeSources];

        if(sndSrc->Generate() == false)
        {
            break;
        }

        activeSources++;
    }

    if(activeSources <= 0)
    {
        kex::cSystem->Warning("No sources available for sound system\n");
        return;
    }

    alListener3f(AL_POSITION, 0, 0, 0);
    bInitialized = true;

    kex::cSystem->Printf("Sound System Initialized (%s)\n", GetDeviceName());
}

//
// kexSoundOAL::Shutdown
//

void kexSoundOAL::Shutdown(void)
{
    if(!bInitialized)
    {
        return;
    }

    int i;
    kexWavFile *wavFile;

    kex::cSystem->Printf("Shutting down audio\n");

    for(i = 0; i < activeSources; i++)
    {
        sources[i].Delete();
    }

    for(i = 0; i < MAX_HASH; i++)
    {
        for(wavFile = wavList.GetData(i); wavFile; wavFile = wavList.Next())
        {
            wavFile->Delete();
        }
    }

    alcMakeContextCurrent(NULL);
    alcDestroyContext(alContext);
    alcCloseDevice(alDevice);

    Mem_Purge(kexSoundOAL::hb_sound);

    bInitialized = false;
}

//
// kexSoundOAL::GetDeviceName
//

char *kexSoundOAL::GetDeviceName(void)
{
    return (char*)alcGetString(alDevice, ALC_DEVICE_SPECIFIER);
}

//
// kexSoundOAL::GetAvailableSource
//

kexSoundSource *kexSoundOAL::GetAvailableSource(void)
{
    int i;
    kexSoundSource *src = NULL;

    for(i = 0; i < activeSources; i++)
    {
        kexSoundSource *sndSrc = &sources[i];

        if(sndSrc->InUse())
        {
            continue;
        }

        src = sndSrc;
        src->Reset();
        break;
    }

    return src;
}

//
// kexSoundOAL::Play
//

void kexSoundOAL::Play(void *data, const int volume, const int sep, kexObject *ref, bool bLooping)
{
    kexSoundSource *src;
    kexWavFile *wavFile = NULL;
    char *name;

    if(!bInitialized)
    {
        return;
    }

    if(!(src = GetAvailableSource()))
    {
        return;
    }

    name = (char*)data;

    if(!(wavFile = wavList.Find(name)))
    {
        byte *wavdata;

        if(kex::cPakFiles->OpenFile(name, &wavdata, kexSoundOAL::hb_sound) == 0)
        {
            return;
        }

        wavFile = wavList.Add(name, kexSoundOAL::hb_sound);
        wavFile->Allocate(name, wavdata);
    }

    src->wave = wavFile;
    src->volume = ((float)volume / 128.0f) * cvarVolume.GetFloat();
    src->pan = (float)sep;
    src->pitch = 1;
    src->Looping() = bLooping;
    src->refObject = ref;

    if(ref && ref->InstanceOf(&kexGameObject::info))
    {
        static_cast<kexGameObject*>(ref)->AddRef();
    }

    src->Play();
}

//
// kexSoundOAL::Stop
//

void kexSoundOAL::Stop(const int handle)
{
    if(handle < 0 || handle >= activeSources)
    {
        return;
    }

    sources[handle].Stop();
    sources[handle].Free();
}

//
// kexSoundOAL::Playing
//

bool kexSoundOAL::Playing(const int handle)
{
    ALint state;
    alGetSourcei(handle, AL_SOURCE_STATE, &state);

    return (state == AL_PLAYING);
}

//
// kexSoundOAL::SourceLooping
//

bool kexSoundOAL::SourceLooping(const int handle)
{
    if(handle < 0 || handle >= activeSources)
    {
        return false;
    }

    return sources[handle].Looping();
}

//
// kexSoundOAL::UpdateSource
//

void kexSoundOAL::UpdateSource(const int handle, const int volume, const int sep)
{
    sources[handle].volume = ((float)volume / 128.0f) * cvarVolume.GetFloat();
    sources[handle].pan = (float)sep;

    sources[handle].UpdateParameters();
}

//
// kexSoundOAL::Update
//

void kexSoundOAL::Update(void)
{
    for(int i = 0; i < activeSources; i++)
    {
        sources[i].Update();
    }
}

//
// kexSoundOAL::NumSources
//

const int kexSoundOAL::NumSources(void) const
{
    return activeSources;
}

//
// kexSoundOAL::GetRefObject
//

kexObject *kexSoundOAL::GetRefObject(const int handle)
{
    if(handle < 0 || handle >= activeSources)
    {
        return NULL;
    }

    return sources[handle].refObject;
}
