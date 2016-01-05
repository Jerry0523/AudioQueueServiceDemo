//
//  AudioPlayerOutput.cpp
//  AudioQueueServiceDemo
//
//  Created by Jerry Wong's Mac Mini on 15/11/23.
//  Copyright © 2015年 Jerry Wong. All rights reserved.
//

#include "AudioPlayerOutput.h"

using namespace JWAudioToolkit;

void AudioPlayerOutput::putSamples(SInt16 *samples, UInt32 size) {
    mSamples = samples;
    mSamplesSize = size;
    if (!hasSetUp) {
        setup();
    }
}

void AudioPlayerOutput::clear(){
    AudioProcessorPipe::clear();
    hasSetUp = false;
    if (mSamples) {
        delete [] mSamples;
        mSamples = NULL;
    }
    if (mQueue) {
        AudioQueueDispose(mQueue, TRUE);
        mQueue = NULL;
    }
}

void AudioPlayerOutput::setup(){
    hasSetUp = true;
    assert(AudioQueueNewOutput(&format,
                               MyOutputBufferHandler,
                               this,
                               NULL,
                               NULL,
                               0,
                               &mQueue) == noErr);
    
    
    UInt32 size = sizeof(format);
    
    assert(AudioQueueGetProperty(mQueue,
                                   kAudioQueueProperty_StreamDescription,
                                   &format,
                                   &size) == noErr);
    int bufferByteSize = ComputeRecordBufferSize(&format, mQueue,kBufferDurationSeconds);
    for (int i = 0; i < kNumberRecordBuffers; ++i) {
        assert(AudioQueueAllocateBuffer(mQueue, bufferByteSize, &mBuffers[i]) == noErr);
        fillSamplesInBuffer(mBuffers[i]);
    }
    AudioQueueStart(mQueue, NULL);
}

void AudioPlayerOutput::fillSamplesInBuffer(AudioQueueBufferRef buffer) {
    if(mSamples){
        buffer->mAudioDataByteSize = mSamplesSize;
        memcpy(buffer->mAudioData, mSamples, mSamplesSize);
        delete [] mSamples;
        mSamples = NULL;
    }
    AudioQueueEnqueueBuffer(mQueue, buffer, 0, NULL);
}

void AudioPlayerOutput::MyOutputBufferHandler(void *input, AudioQueueRef queue, AudioQueueBufferRef buffer){
    AudioPlayerOutput *player = (AudioPlayerOutput *)input;
    player->fillSamplesInBuffer(buffer);
}