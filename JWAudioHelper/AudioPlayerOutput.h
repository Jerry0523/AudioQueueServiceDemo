//
//  AudioPlayerOutput.h
//  AudioQueueServiceDemo
//
//  Created by Jerry Wong's Mac Mini on 15/11/23.
//  Copyright © 2015年 Jerry Wong. All rights reserved.
//

#ifndef AudioPlayerOutput_h
#define AudioPlayerOutput_h

#include "AudioToolkit.h"

namespace JWAudioToolkit
{

class AudioPlayerOutput:public AudioProcessorPipe
{
public:
    void putSamples(SInt16 *samples, UInt32 size);
    void clear();
    
private:
    bool hasSetUp;
    AudioQueueRef mQueue;
    AudioQueueBufferRef	mBuffers[kNumberRecordBuffers];
    SInt16 *mSamples;
    UInt32 mSamplesSize;
    void setup();
    
    static void MyOutputBufferHandler(void *input, AudioQueueRef queue, AudioQueueBufferRef buffer);
    
    void fillSamplesInBuffer(AudioQueueBufferRef buffer);
};
    
}

#endif /* AudioPlayerOutput_h */
