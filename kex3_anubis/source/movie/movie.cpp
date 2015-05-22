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
//      Movie Player
//

#include "kexlib.h"
#include "renderMain.h"
#include "movie.h"

extern "C"
{
    #include "libavcodec/avcodec.h"
    #include "libavformat/avformat.h"
    #include "libswscale/swscale.h"
    #include "libavutil/time.h"
}

//
// because apparently, we still need to use deprecated functions....
//
#if defined(__ICL) || defined (__INTEL_COMPILER)
    #define FF_DISABLE_DEPRECATION_WARNINGS __pragma(warning(push)) __pragma(warning(disable:1478))
    #define FF_ENABLE_DEPRECATION_WARNINGS  __pragma(warning(pop))
#elif defined(_MSC_VER)
    #define FF_DISABLE_DEPRECATION_WARNINGS __pragma(warning(push)) __pragma(warning(disable:4996))
    #define FF_ENABLE_DEPRECATION_WARNINGS  __pragma(warning(pop))
#else
    #define FF_DISABLE_DEPRECATION_WARNINGS _Pragma("GCC diagnostic ignored \"-Wdeprecated-declarations\"")
    #define FF_ENABLE_DEPRECATION_WARNINGS  _Pragma("GCC diagnostic warning \"-Wdeprecated-declarations\"")
#endif

//
// the buffer size passed into the post mix callback routine can vary, depending
// on the OS. I am expecting at least 4k tops but if needed, it can be bumped to 8k
//
#define MAX_AUDIO_FRAME_SIZE        192000

//=============================================================================
//
// Structs
//
//=============================================================================

//
// every video and audio packet that we recieve from the decoder is thrown into a queue
// which is all done by the thread routine. the drawer and post mix functions are
// responsible from accessing each entry in the queue
//

typedef struct avPacketData_s
{
    AVPacket                    *packet;
    struct avPacketData_s       *next;
    bool                        bEndMark;
} avQueuePacketData_t;

typedef struct
{
    avQueuePacketData_t         *first;
    kexThread::kMutex_t         mutex;
} avPacketQueue_t;

typedef struct avAudioQueueData_s
{
    uint8_t                     buffer[MOVIE_AUDIO_BUFFER_SIZE];
    int                         size;
    int                         offset;
    double                      timestamp;
    struct avAudioQueueData_s   *next;
} avAudioQueueData_t;

typedef struct
{
    avAudioQueueData_t          *first;
    kexThread::kMutex_t         mutex;
} avAudioQueue_t;

//=============================================================================
//
// kexMoviePlayerFFMpeg
//
//=============================================================================

class kexMoviePlayerFFMpeg : public kexMoviePlayer
{
public:
    kexMoviePlayerFFMpeg(void);
    ~kexMoviePlayerFFMpeg(void);

    virtual byte            *GetAudioBufferInQueue(bool &bFinished);
    virtual bool            HasVideoStarted(void);

    //=============================================================================
    //
    // Packet Querying
    //
    //=============================================================================
    avQueuePacketData_t     *AllocQueuePacketData(AVPacket *packet);
    avPacketQueue_t         *AllocQueuePacket(void);
    void                    PushPacketToQueue(avPacketQueue_t *packetQueue, AVPacket *packet);
    bool                    PopPacketFromQueue(avPacketQueue_t *packetQueue, AVPacket **packet);
    void                    DeletePacketQueue(avPacketQueue_t **packetQueue);

    //=============================================================================
    //
    // Audio Functions
    //
    //=============================================================================
    void                    FillAudioBuffer(AVPacket *packet);

    //=============================================================================
    //
    // Properties
    //
    //=============================================================================
    const uint64_t          GlobalPTS(void) { return globalPts; }
    const bool              HasAudio(void) { return bHasAudio; }
    const bool              UserExit(void) { return bUserExit; }
    const int               VideoStreamIdx(void) { return videoStreamIdx; }
    const int               AudioStreamIdx(void) { return audioStreamIdx; }
    avPacketQueue_t         *VideoPacketQueue(void) { return videoPacketQueue; }
    avPacketQueue_t         *AudioPacketQueue(void) { return audioPacketQueue; }

    AVFormatContext         *FormatCtx(void) { return formatCtx; }
    AVCodecContext          *VideoCodecCtx(void) { return videoCodecCtx; }
    AVCodecContext          *AudioCodecCtx(void) { return audioCodecCtx; }
    
private:

    //=============================================================================
    //
    // Audio Buffer Querying
    //
    //=============================================================================
    avAudioQueueData_t      *AllocAudioQueueData(uint8_t *buffer, const int size);
    void                    PushAudioToQueue(uint8_t *buffer, const int size);
    uint8_t                 *PopAudioFromQueue(const int size, double *timestamp, bool *bFilled);
    void                    DeleteAudioQueue(void);

    //=============================================================================
    //
    // Video/Audio Clock Comparison
    //
    //=============================================================================
    bool                    VideoClockBehind(void);

    //=============================================================================
    //
    // Video Functions
    //
    //=============================================================================
    void                    YUVToBuffer(AVFrame *frame);
    double                  UpdateVideoClock(AVFrame *frame, double pts);
    void                    ProcessNextVideoFrame(void);

    //=============================================================================
    //
    // Initialization and loading
    //
    //=============================================================================
    bool                    SetupCodecContext(AVCodecContext **context, AVCodec **codec, int index);
    bool                    LoadVideo(const char *filename);

    //=============================================================================
    //
    // General Functions
    //
    //=============================================================================
    void                    Shutdown(void);
    void                    DrawVideoStream(void);
    bool                    CapFrameRate(void);
    virtual void            StartVideoStream(const char *filename);

    // ffmpeg contexts
    struct SwsContext       *swsCtx;
    AVFormatContext         *formatCtx;
    AVCodecContext          *videoCodecCtx;
    AVCodecContext          *audioCodecCtx;

    // codecs
    AVCodec                 *videoCodec;
    AVCodec                 *audioCodec;

    // global video frame
    AVFrame                 *videoFrame;

    // global buffers
    uint8_t                 *videoBuffer;
    uint8_t                 *audioBuffer;

    // global timestamps
    int64_t                 currentPts;
    uint64_t                globalPts;
    double                  audioPts;

    // dimentions
    int                     reqWidth;
    int                     reqHeight;

    // stream indexes
    int                     videoStreamIdx;
    int                     audioStreamIdx;

    // main thread
    kexThread::kThread_t    thread;

    // texture to display video
    kexTexture              texture;

    // user pressed a button
    bool                    bUserExit;

    // is audio enabled?
    bool                    bHasAudio;

    // all video frames processed
    bool                    bVideoFinished;

    // all audio frames processed
    bool                    bAudioFinished;

    // keep track of holding down the key
    bool                    bUserPressed;

    // packet queues
    avPacketQueue_t         *videoPacketQueue;
    avPacketQueue_t         *audioPacketQueue;

    // audio buffer queue
    avAudioQueue_t          audioQueue;

    // clock speed
    double                  videoClock;
    double                  audioClock;

    // length of the frame in time
    double                  frameTime;
    double                  lastFrameTime;

    double                  remainingVideoTime;
    double                  remainingAudioTime;

    bool                    bPlaying;
};

static kexMoviePlayerFFMpeg moviePlayerLocal;
kexMoviePlayer *kex::cMoviePlayer = &moviePlayerLocal;

//
// GetBufferProc
//

FF_DISABLE_DEPRECATION_WARNINGS

static int GetBufferProc(struct AVCodecContext *c, AVFrame *pic)
{
    int ret = avcodec_default_get_buffer(c, pic);
    uint64_t *pts = (uint64_t*)av_malloc(sizeof(uint64_t));
    *pts = moviePlayerLocal.GlobalPTS();

    pic->opaque = pts;
    return ret;
}

//
// ReleaseBufferProc
//

static void ReleaseBufferProc(struct AVCodecContext *c, AVFrame *pic)
{
    if(pic)
    {
        av_freep(&pic->opaque);
    }

    avcodec_default_release_buffer(c, pic);
}

FF_ENABLE_DEPRECATION_WARNINGS

//
// kexMoviePlayerFFMpeg::kexMoviePlayerFFMpeg
//

kexMoviePlayerFFMpeg::kexMoviePlayerFFMpeg(void)
{
    this->bPlaying = false;
}

//
// kexMoviePlayerFFMpeg::~kexMoviePlayerFFMpeg
//

kexMoviePlayerFFMpeg::~kexMoviePlayerFFMpeg(void)
{
    Shutdown();
}

//=============================================================================
//
// Packet Querying
//
//=============================================================================

//
// kexMoviePlayerFFMpeg::AllocQueuePacketData
//

avQueuePacketData_t *kexMoviePlayerFFMpeg::AllocQueuePacketData(AVPacket *packet)
{
    avQueuePacketData_t *packetItem = (avQueuePacketData_t*)malloc(sizeof(avQueuePacketData_t));

    if(packet)
    {
        packetItem->packet = (AVPacket*)malloc(sizeof(AVPacket));
        memcpy(packetItem->packet, packet, sizeof(AVPacket));
        packetItem->bEndMark = false;
    }
    else
    {
        packetItem->packet = NULL;
        packetItem->bEndMark = true;
    }

    packetItem->next = NULL;
    
    return packetItem;
}

//
// kexMoviePlayerFFMpeg::AllocQueuePacket
//

avPacketQueue_t *kexMoviePlayerFFMpeg::AllocQueuePacket(void)
{
    avPacketQueue_t *packetQueue = (avPacketQueue_t*)malloc(sizeof(avPacketQueue_t));
    packetQueue->first = NULL;
    
    packetQueue->mutex = kex::cThread->AllocMutex();
    return packetQueue;
}

//
// kexMoviePlayerFFMpeg::PushPacketToQueue
//
// Adds a video packet the queue list
// Called from thread
//

void kexMoviePlayerFFMpeg::PushPacketToQueue(avPacketQueue_t *packetQueue, AVPacket *packet)
{
    avQueuePacketData_t **ptr;
    
    kex::cThread->LockMutex(packetQueue->mutex);
    ptr = &packetQueue->first;
    
    while(*ptr)
    {
        ptr = &(*ptr)->next;
    }
    
    *ptr = AllocQueuePacketData(packet);
    
    kex::cThread->UnlockMutex(packetQueue->mutex);
}

//
// kexMoviePlayerFFMpeg::PopPacketFromQueue
//

bool kexMoviePlayerFFMpeg::PopPacketFromQueue(avPacketQueue_t *packetQueue, AVPacket **packet)
{
    bool result = true;

    *packet = NULL;

    if(packetQueue->mutex)
    {
        kex::cThread->LockMutex(packetQueue->mutex);
    }

    if(packetQueue->first)
    {
        if(!packetQueue->first->bEndMark)
        {
            void *del;
            
            *packet = packetQueue->first->packet;
            del = packetQueue->first;
            packetQueue->first = packetQueue->first->next;
            
            free(del);
        }
        else
        {
            // needed to indicate that we reached the end
            result = false;
        }
    }
    
    if(packetQueue->mutex)
    {
        kex::cThread->UnlockMutex(packetQueue->mutex);
    }

    return result;
}

//
// kexMoviePlayerFFMpeg::DeletePacketQueue
//

void kexMoviePlayerFFMpeg::DeletePacketQueue(avPacketQueue_t **packetQueue)
{
    kex::cThread->DestroyMutex((*packetQueue)->mutex);
    (*packetQueue)->mutex = NULL;

    while(1)
    {
        AVPacket *packet;

        if(!PopPacketFromQueue(*packetQueue, &packet) || packet == NULL)
        {
            break;
        }
        
        av_free_packet(packet);
    }

    free(*packetQueue);
    *packetQueue = NULL;
}

//=============================================================================
//
// Audio Buffer Querying
//
//=============================================================================

//
// kexMoviePlayerFFMpeg::AllocAudioQueueData
//

avAudioQueueData_t *kexMoviePlayerFFMpeg::AllocAudioQueueData(uint8_t *buffer, const int size)
{
    avAudioQueueData_t *audioData = (avAudioQueueData_t*)malloc(sizeof(avAudioQueueData_t));

    audioData->size = size;
    audioData->next = NULL;
    audioData->offset = 0;
    audioData->timestamp = audioPts;

    memcpy(audioData->buffer, buffer, size);
    return audioData;
}

//
// kexMoviePlayerFFMpeg::PushAudioToQueue
//
// Adds a fixed amount of audio buffer to the queue.
// If the size is less than the max buffer size specified
// by MOVIE_AUDIO_BUFFER_SIZE, then the current buffer in
// the queue will be reused the next time this function
// is called until it's full
//

void kexMoviePlayerFFMpeg::PushAudioToQueue(uint8_t *buffer, const int size)
{
    avAudioQueueData_t **ptr;
    int bufsize = size;
    uint8_t *buf = buffer;
    
    kex::cThread->LockMutex(audioQueue.mutex);
    ptr = &audioQueue.first;
    
    while(bufsize > 0)
    {
        // we'll need to create a new audio queue here
        if(*ptr == NULL)
        {
            // exceeded the buffer limit?
            if(bufsize > MOVIE_AUDIO_BUFFER_SIZE)
            {
                int remaining = (bufsize - MOVIE_AUDIO_BUFFER_SIZE);

                // copy what we have
                *ptr = AllocAudioQueueData(buf, MOVIE_AUDIO_BUFFER_SIZE);
                bufsize = remaining;
                buf += MOVIE_AUDIO_BUFFER_SIZE;
            }
            else
            {
                *ptr = AllocAudioQueueData(buf, bufsize);
                // we don't need to do anything else
                break;
            }
        }

        // buffer not filled?
        if((*ptr)->size < MOVIE_AUDIO_BUFFER_SIZE)
        {
            int len = bufsize + (*ptr)->size;

            // exceeded the buffer limit?
            if(len > MOVIE_AUDIO_BUFFER_SIZE)
            {
                int remaining = (MOVIE_AUDIO_BUFFER_SIZE - (*ptr)->size);

                // copy what we have
                memcpy((*ptr)->buffer + (*ptr)->size, buf, remaining);
                (*ptr)->size = MOVIE_AUDIO_BUFFER_SIZE;

                bufsize -= remaining;
                buf += remaining;
            }
            else
            {
                // copy into the existing buffer
                memcpy((*ptr)->buffer + (*ptr)->size, buf, bufsize);
                (*ptr)->size += bufsize;

                // we don't need to do anything else
                kex::cThread->UnlockMutex(audioQueue.mutex);
                return;
            }
        }

        ptr = &(*ptr)->next;
    }
    
    kex::cThread->UnlockMutex(audioQueue.mutex);
}

//
// I_AVPopAudioFromQueue
//
// Retrieves an audio buffer from the queue and gets the
// timestamp that it's based on. The buffer in the queue
// won't be freed until the entire buffer has been read
// which 'filled' will be set to true
//
// Called from the Post-Mix callback routine
//

uint8_t *kexMoviePlayerFFMpeg::PopAudioFromQueue(const int size, double *timestamp, bool *bFilled)
{
    static uint8_t buffer[MOVIE_AUDIO_BUFFER_SIZE];
    uint8_t *result = NULL;
    
    if(bFilled)
    {
        *bFilled = false;
    }
    
    if(timestamp)
    {
        *timestamp = 0;
    }

    if(audioQueue.mutex)
    {
        kex::cThread->LockMutex(audioQueue.mutex);
    }

    if(audioQueue.first)
    {
        void *del;

        memcpy(buffer, audioQueue.first->buffer + audioQueue.first->offset, size);
        audioQueue.first->offset += size;
        result = buffer;

        if(timestamp)
        {
            *timestamp = audioQueue.first->timestamp;
        }

        // if the entire buffer was read, then we need to free this queue
        if(audioQueue.first->offset >= audioQueue.first->size)
        {
            del = audioQueue.first;
            audioQueue.first = audioQueue.first->next;
        
            free(del);
            
            if(bFilled)
            {
                *bFilled = true;
            }
        }
    }
    
    if(audioQueue.mutex)
    {
        kex::cThread->UnlockMutex(audioQueue.mutex);
    }

    return result;
}

//
// kexMoviePlayerFFMpeg::DeleteAudioQueue
//

void kexMoviePlayerFFMpeg::DeleteAudioQueue(void)
{
    kex::cThread->DestroyMutex(audioQueue.mutex);
    audioQueue.mutex = NULL;

    while(PopAudioFromQueue(MOVIE_AUDIO_BUFFER_SIZE, NULL, NULL) != NULL);
}

//=============================================================================
//
// Video/Audio Clock Comparison
//
//=============================================================================

//
// kexMoviePlayerFFMpeg::VideoClockBehind
//

bool kexMoviePlayerFFMpeg::VideoClockBehind(void)
{
    return (bHasAudio && (videoClock > 0 && audioClock > 0) &&
        (videoClock < audioClock));
}

//=============================================================================
//
// Audio Functions
//
//=============================================================================

//
// kexMoviePlayerFFMpeg::FillAudioBuffer
//
// Decodes an audio sample from the AVPacket pointer and then
// pushes it into the queue list for the post mix callback function
// to process.
//
// Called from thread
//

void kexMoviePlayerFFMpeg::FillAudioBuffer(AVPacket *packet)
{
    static AVFrame frame;
    int length;
    int dataSize;
    int lineSize;
    int frameDone = 0;
    uint16_t *samples;
    int n;
    int size;

    length = avcodec_decode_audio4(audioCodecCtx, &frame, &frameDone, packet);
    dataSize = av_samples_get_buffer_size(&lineSize,
                                          audioCodecCtx->channels,
                                          frame.nb_samples,
                                          audioCodecCtx->sample_fmt,
                                          1);

    if(dataSize <= 0)
    {
        // no data yet, need more frames
        return;
    }

    if(packet->pts != AV_NOPTS_VALUE)
    {
        audioClock = av_q2d(audioCodecCtx->time_base) * packet->pts;
    }

    n = 2 * audioCodecCtx->channels;
    audioPts += (double)dataSize / (double)(n * audioCodecCtx->sample_rate);

    samples = (uint16_t*)audioBuffer;

    if(frameDone)
    {
        int ls;
        int write_p = 0;
        int nb, ch;

        if(audioCodecCtx->sample_fmt == AV_SAMPLE_FMT_FLTP)
        {
            ls = lineSize / sizeof(float);
        }
        else if(audioCodecCtx->sample_fmt == AV_SAMPLE_FMT_S16P)
        {
            ls = lineSize / sizeof(int16_t);
        }

        // get the audio samples
        // WARNING: this only applies to AV_SAMPLE_FMT_FLTP and AV_SAMPLE_FMT_S16P
        for(nb = 0; nb < ls; nb++)
        {
            for(ch = 0; ch < audioCodecCtx->channels; ch++)
            {
                if(audioCodecCtx->sample_fmt == AV_SAMPLE_FMT_FLTP)
                {
                    samples[write_p] = (uint16_t)((float*)frame.extended_data[ch])[nb] * 32767;
                }
                else if(audioCodecCtx->sample_fmt == AV_SAMPLE_FMT_S16P)
                {
                    samples[write_p] = (uint16_t)((int16_t*)frame.extended_data[ch])[nb];
                }

                write_p++;
            }
        }

        size = (ls) * sizeof(uint16_t) * audioCodecCtx->channels;

        // add the buffer (partial or not) to the queue
        PushAudioToQueue((uint8_t*)samples, size);
    }
}

//=============================================================================
//
// Video Functions
//
//=============================================================================

//
// kexMoviePlayerFFMpeg::UVToBuffer
//

void kexMoviePlayerFFMpeg::YUVToBuffer(AVFrame *frame)
{
    int y, u, v;
    int r, g, b;
    int px, py;
    int i;
    byte *yptr = frame->data[0];
    byte *uptr = frame->data[1];
    byte *vptr = frame->data[2];
    
    i = 0;
    
    for(py = 0; py < reqHeight; py++)
    {
        for(px = 0; px < reqWidth; px++, i += 3)
        {
            y = yptr[py     * frame->linesize[0] + px    ];
            u = uptr[py / 2 * frame->linesize[1] + px / 2];
            v = vptr[py / 2 * frame->linesize[2] + px / 2];
            
            r = (int)((float)y + 1.402f   * ((float)v - 128.0f));
            g = (int)((float)y - 0.34414f * ((float)u - 128.0f) - 0.71414f * ((float)v - 128.0f));
            b = (int)((float)y + 1.772f   * ((float)u - 128.0f));
            
            videoBuffer[i    ] = MAX(MIN(r, 255), 0);
            videoBuffer[i + 1] = MAX(MIN(g, 255), 0);
            videoBuffer[i + 2] = MAX(MIN(b, 255), 0);
        }
    }
}

//
// kexMoviePlayerFFMpeg::UpdateVideoClock
//
// Try to sync the video clock with the given timestamp
// gathered from the frame packet
//

double kexMoviePlayerFFMpeg::UpdateVideoClock(AVFrame *frame, double pts)
{
    double frameDelay;

    if(pts != 0)
    {
        videoClock = pts;
    }
    else
    {
        pts = videoClock;
    }

    frameDelay = av_q2d(videoCodecCtx->time_base);
    frameDelay += frame->repeat_pict * (frameDelay * 0.5);

    videoClock += frameDelay;
    return pts;
}

//
// kexMoviePlayerFFMpeg::ProcessNextVideoFrame
//

void kexMoviePlayerFFMpeg::ProcessNextVideoFrame(void)
{
    int frameDone = 0;
    AVFrame *frame;
    AVPacket *packet;
    double pts = 0;
    bool behind = false;

#if 1
    if(bHasAudio)
    {
        if(videoClock > audioClock || audioClock <= 0)
        {
            // don't process if the audio clock hasn't started
            // or if the video clock is ahead though
            // this shouldn't be needed but just in case....
            return;
        }
    }
#endif
    
    frame = av_frame_alloc();

    if(bHasAudio)
    {
        behind = bAudioFinished ? true : VideoClockBehind();
    }

    // collect packets until we have a frame
    while(!frameDone || behind)
    {
        if(!PopPacketFromQueue(videoPacketQueue, &packet))
        {
            bVideoFinished = true;
            break;
        }
        
        if(packet == NULL)
        {
            break;
        }

        // get presentation timestamp
        pts = 0;
        globalPts = packet->pts;
        
        avcodec_decode_video2(videoCodecCtx, frame, &frameDone, packet);

        // get the decompression timestamp from this packet
        if(packet->dts == AV_NOPTS_VALUE && frame->opaque &&
            *(uint64_t*)frame->opaque != AV_NOPTS_VALUE)
        {
            pts = (double)*(uint64_t*)frame->opaque;
        }
        else if(packet->dts != AV_NOPTS_VALUE)
        {
            pts = (double)av_frame_get_best_effort_timestamp(frame);
        }
        else
        {
            pts = 0;
        }

        // approximate the timestamp
        pts *= av_q2d(videoCodecCtx->time_base);

        if(frameDone)
        {
            // update the video clock and frame time
            pts = UpdateVideoClock(frame, pts);
            frameTime = (pts - lastFrameTime) * 1000.0;
            lastFrameTime = pts;
            currentPts = av_gettime();
            
            if(bHasAudio)
            {
                // need to keep processing if we're behind
                // some frames may be skipped
                behind = VideoClockBehind();
            }
        }

        av_free_packet(packet);
    }
    
    if(frameDone)
    {
        // convert the decoded data to color data
        sws_scale(swsCtx,
                  (uint8_t const*const*)frame->data,
                  frame->linesize,
                  0,
                  videoCodecCtx->height,
                  videoFrame->data,
                  videoFrame->linesize);
        
        YUVToBuffer(frame);

        texture.Bind();
        texture.Update(videoBuffer);
    }
    
    av_free(frame);
}

//=============================================================================
//
// Initialization and loading
//
//=============================================================================

//
// kexMoviePlayerFFMpeg::SetupCodecContext
//

bool kexMoviePlayerFFMpeg::SetupCodecContext(AVCodecContext **context, AVCodec **codec, int index)
{
    if(index < 0)
    {
        kex::cSystem->Warning("kexMoviePlayerFFMpeg::SetupCodecContext: Couldn't find stream\n");
        return false;
    }
    
    // get pointer to the codec context for the stream
    *context = formatCtx->streams[index]->codec;
    
    // find the decoder for the stream
    *codec = avcodec_find_decoder((*context)->codec_id);
    
    if(*codec == NULL)
    {
        kex::cSystem->Warning("kexMoviePlayerFFMpeg::SetupCodecContext: Unsupported codec\n");
        return false;
    }

    // try to open codec
    if(avcodec_open2(*context, *codec, NULL) < 0)
    {
        kex::cSystem->Warning("kexMoviePlayerFFMpeg::SetupCodecContext: Couldn't open codec\n");
        return false;
    }

    return true;
}

//
// kexMoviePlayerFFMpeg::LoadVideo
//

bool kexMoviePlayerFFMpeg::LoadVideo(const char *filename)
{
    int i;
    int videoSize;
    int audioSize;
    kexStr filepath;

    // setup packet queues
    videoPacketQueue = AllocQueuePacket();
    audioPacketQueue = AllocQueuePacket();

    // setup audio queue
    audioQueue.mutex = kex::cThread->AllocMutex();
    audioQueue.first = NULL;

    bUserExit = false;
    bVideoFinished = false;
    bAudioFinished = false;

    audioPts = 0;
    videoClock = 0;
    audioClock = 0;

    filepath = kexStr::Format("%s\\%s", kex::cvarBasePath.GetValue(), filename);
    filepath.NormalizeSlashes();

    if(avformat_open_input(&formatCtx, filepath.c_str(), NULL, NULL))
    {
        kex::cSystem->Warning("kexMoviePlayerFFMpeg::LoadVideo: Couldn't load %s\n", filepath.c_str());
        return false;
    }

    if(avformat_find_stream_info(formatCtx, NULL))
    {
        kex::cSystem->Warning("kexMoviePlayerFFMpeg::LoadVideo: Couldn't find stream info\n");
        return false;
    }

#ifdef _DEBUG
    av_dump_format(formatCtx, 0, filepath.c_str(), 0);
#endif

    videoStreamIdx = -1;
    audioStreamIdx = -1;
    
    // find the video and audio stream
    for(i = 0; i < (int)formatCtx->nb_streams; i++)
    {
        switch(formatCtx->streams[i]->codec->codec_type)
        {
            case AVMEDIA_TYPE_VIDEO:
                if(videoStreamIdx == -1)
                {
                    videoStreamIdx = i;
                }
                break;

            case AVMEDIA_TYPE_AUDIO:
                if(audioStreamIdx == -1)
                {
                    audioStreamIdx = i;
                }
                break;
                
            default:
                break;
        }
    }
    
    if(videoStreamIdx < 0)
    {
        kex::cSystem->Warning("kexMoviePlayerFFMpeg::LoadVideo: Couldn't find video stream\n");
        return false;
    }

    if(!SetupCodecContext(&videoCodecCtx, &videoCodec, videoStreamIdx) ||
       !SetupCodecContext(&audioCodecCtx, &audioCodec, audioStreamIdx))
    {
        return false;
    }

    reqWidth = videoCodecCtx->width;
    reqHeight = videoCodecCtx->height;

FF_DISABLE_DEPRECATION_WARNINGS
    videoCodecCtx->get_buffer = GetBufferProc;
    videoCodecCtx->release_buffer = ReleaseBufferProc;
FF_ENABLE_DEPRECATION_WARNINGS

    videoFrame = av_frame_alloc();

    videoSize = avpicture_get_size(AV_PIX_FMT_YUV444P, reqWidth, reqHeight);
    audioSize = MAX_AUDIO_FRAME_SIZE + FF_INPUT_BUFFER_PADDING_SIZE;

    videoBuffer = (uint8_t*)av_calloc(1, videoSize * sizeof(uint8_t));
    audioBuffer = (uint8_t*)av_calloc(1, audioSize * sizeof(uint8_t));

    // setup texture
    texture.SetColorMode(TCR_RGB);
    texture.OriginalWidth() = reqWidth;
    texture.OriginalHeight() = reqHeight;
    texture.Width() = reqWidth;
    texture.Height() = reqHeight;

    texture.Upload(videoBuffer, TC_CLAMP, TF_LINEAR);

    swsCtx = sws_getContext(videoCodecCtx->width,
                            videoCodecCtx->height,
                            videoCodecCtx->pix_fmt,
                            reqWidth,
                            reqHeight,
                            AV_PIX_FMT_YUV444P,
                            SWS_BICUBIC,
                            NULL, NULL, NULL);
    
    // assign appropriate parts of buffer to image planes in videoFrame
    // Note that videoFrame is an AVFrame, but AVFrame is a superset
    // of AVPicture
    avpicture_fill((AVPicture*)videoFrame,
                   videoBuffer,
                   AV_PIX_FMT_YUV444P,
                   reqWidth,
                   reqHeight);
    
    currentPts = av_gettime();
    frameTime = 1000.0 / av_q2d(videoCodecCtx->framerate);
    lastFrameTime = frameTime;
    return true;
}

//=============================================================================
//
// General Functions
//
//=============================================================================

//
// kexMoviePlayerFFMpeg::GetAudioBufferInQueue
//
// Called from kexSound class once the audio thread has been hooked
//

byte *kexMoviePlayerFFMpeg::GetAudioBufferInQueue(bool &bFinished)
{
    uint8_t *buf = NULL;
    double timestamp;
    bool bFilled;
    
    bFinished = false;

    if(audioClock > 0 && videoClock <= 0)
    {
        // video clock needs to catch up
        return NULL;
    }

    if(!audioPacketQueue || bUserExit)
    {
        return NULL;
    }

    if((buf = PopAudioFromQueue(MOVIE_AUDIO_BUFFER_SIZE, &timestamp, &bFilled)))
    {
        bAudioFinished = false;

        // update the audio clock ONLY after the buffer in the queue
        // has been fully read
        if(bFilled)
        {
            // get the current audio clock time
            audioClock = timestamp;
        }
    }
    else
    {
        bAudioFinished = true;
    }

    bFinished = bAudioFinished;
    return (byte*)buf;
}

//
// kexMoviePlayerFFMpeg::HasVideoStarted
//

bool kexMoviePlayerFFMpeg::HasVideoStarted(void)
{
    return (audioClock <= 0);
}

//
// kexMoviePlayerFFMpeg::Shutdown
//

void kexMoviePlayerFFMpeg::Shutdown(void)
{
    if(!bPlaying)
    {
        return;
    }

    if(bHasAudio)
    {
        kex::cSound->UnHookMovieAudioStream();
    }

    DeletePacketQueue(&videoPacketQueue);
    DeletePacketQueue(&audioPacketQueue);
    DeleteAudioQueue();

    av_free(videoBuffer);
    av_free(audioBuffer);
    av_free(videoFrame);
    
    avcodec_close(videoCodecCtx);
    avcodec_close(audioCodecCtx);
    avformat_close_input(&formatCtx);
    sws_freeContext(swsCtx);

    texture.Delete();
    thread = NULL;

    bPlaying = false;
}

//
// kexMoviePlayerFFMpeg::DrawVideoStream
//

void kexMoviePlayerFFMpeg::DrawVideoStream(void)
{
    float wi, hi, ws, hs;
    float ri, rs;
    float scalew, scaleh;
    float xoffs = 0, yoffs = 0;

    dglClearColor(0, 0, 0, 1);
    kexRender::cBackend->ClearBuffer(GLCB_COLOR);

    ProcessNextVideoFrame();

    kexRender::cBackend->SetOrtho();

    ws = (float)kex::cSystem->VideoWidth();
    hs = (float)kex::cSystem->VideoHeight();
    wi = (float)reqWidth;
    hi = (float)reqHeight;

    rs = ws / hs;
    ri = wi / hi;

    if(rs > ri)
    {
        scalew = wi * hs / hi;
        scaleh = hs;
    }
    else
    {
        scalew = ws;
        scaleh = hi * ws / wi;
    }

    if(scalew < ws)
    {
        xoffs = (ws - scalew) / 2;
    }
    if(scaleh < hs)
    {
        yoffs = (hs - scaleh) / 2;
    }

    texture.Bind();

    kexRender::cVertList->BindDrawPointers();
    kexRender::cVertList->AddQuad(xoffs, yoffs, 0, scalew, scaleh, 0, 0, 1, 1, 255, 255, 255, 255);
    kexRender::cVertList->DrawElements();

    if(kex::cvarDeveloper.GetBool())
    {
        kex::cConsole->Draw();

        kexRender::cUtils->ClearDebugLine();
        kexRender::cUtils->PrintStatsText("video time", "%f", remainingVideoTime);
        kexRender::cUtils->PrintStatsText("audio time", "%f", remainingAudioTime);
    }

    dglFinish();
    kex::cSystem->SwapBuffers();
}

//
// kexMoviePlayerFFMpeg::CapFrameRate
//

bool kexMoviePlayerFFMpeg::CapFrameRate(void)
{
    static double extra = 0;
    uint64_t curTics = av_gettime();
    double elapsed = (double)(curTics - currentPts) / 1000.0;

    if(elapsed < frameTime - extra)
    {
        if((frameTime - extra) - elapsed > 3.0)
        {
            // give up a small timeslice
            kex::cTimer->Sleep(1);
        }
        return true;
    }

    remainingVideoTime = audioPts - videoClock;
    remainingAudioTime = audioPts - audioClock;

    extra = (elapsed - frameTime);

    if(extra < 0)
    {
        extra = 0;
    }

    return false;
}

//
// IteratePacketsThread
//

static int IteratePacketsThread(void *data)
{
    AVPacket packet;

    while(av_read_frame(moviePlayerLocal.FormatCtx(), &packet) >= 0 && !moviePlayerLocal.UserExit())
    {
        if(packet.stream_index == moviePlayerLocal.VideoStreamIdx())
        {
            // queue the video packet
            moviePlayerLocal.PushPacketToQueue(moviePlayerLocal.VideoPacketQueue(), &packet);
        }
        else if(moviePlayerLocal.HasAudio() && packet.stream_index == moviePlayerLocal.AudioStreamIdx())
        {
            if(moviePlayerLocal.AudioCodecCtx()->sample_fmt == AV_SAMPLE_FMT_FLTP ||
               moviePlayerLocal.AudioCodecCtx()->sample_fmt == AV_SAMPLE_FMT_S16P)
            {
                // queue the audio buffer from the packet
                moviePlayerLocal.FillAudioBuffer(&packet);
                av_free_packet(&packet);
            }
        }
        else
        {
            av_free_packet(&packet);
        }
    }

    // add end markers
    moviePlayerLocal.PushPacketToQueue(moviePlayerLocal.VideoPacketQueue(), NULL);
    moviePlayerLocal.PushPacketToQueue(moviePlayerLocal.AudioPacketQueue(), NULL);
    return 0;
}

//
// kexMoviePlayerFFMpeg::StartVideoStream
//

void kexMoviePlayerFFMpeg::StartVideoStream(const char *filename)
{
    static bool bInitialized = false;
    inputEvent_t *ev;

    // initialize ffmpeg if it hasn't already
    if(!bInitialized)
    {
        bHasAudio = true;

        av_register_all();
        bInitialized = true;
    }

    // clear out any remaining user inputs in the queue
    while((ev = kex::cSession->EventQueue().Pop()) != NULL)
    {
        if(ev->type == ev_keydown)
        {
            switch(ev->data1)
            {
            case KKEY_ESCAPE:
            case KKEY_RETURN:
            case KKEY_SPACE:
                // early out before we go through the trouble
                // of loading the video
                return;
            default:
                break;
            }
        }
    }

    globalPts = AV_NOPTS_VALUE;

    if(!LoadVideo(filename))
    {
        // can't find or load the video... oh well..
        return;
    }

    thread = kex::cThread->CreateThread("ffmpeg_thread", NULL, IteratePacketsThread);
    kex::cThread->WaitThread(thread, NULL);

    if(bHasAudio)
    {
        kex::cTimer->Sleep(500);
        kex::cSound->HookToMovieAudioStream(audioCodecCtx->sample_rate, audioCodecCtx->channels);
        kex::cTimer->Sleep(100);
    }

    bPlaying = true;

    while((!bVideoFinished || (bHasAudio && !bAudioFinished)) && !bUserExit)
    {
        kex::cInput->PollInput();

        while((ev = kex::cSession->EventQueue().Pop()) != NULL)
        {
            if(ev->type == ev_keydown)
            {
                switch(ev->data1)
                {
                case KKEY_ESCAPE:
                case KKEY_RETURN:
                case KKEY_SPACE:
                    if(bUserPressed == false)
                    {
                        bUserExit = true;
                        bUserPressed = true;
                    }
                    break;
                default:
                    break;
                }
            }
            else if(ev->type == ev_keyup)
            {
                bUserPressed = false;
            }
        }

        if(!CapFrameRate())
        {
            DrawVideoStream();
        }
    }

    Shutdown();

    // make sure everything is unbinded
    kexRender::cBackend->ClearBindedTexture();
    kex::cTimer->Sleep(500);
}
