//
//  AudioPlayerOutput.cpp
//  AudioQueueServiceDemo
//
//  Created by Jerry Wong's Mac Mini on 15/11/23.
//  Copyright © 2015年 Jerry Wong. All rights reserved.
//

#include "AudioPlayerOutput.h"

using namespace JWAudioToolkit;

AudioPlayerOutput::AudioPlayerOutput():AudioProcessorPipe() {
    
}

AudioPlayerOutput::AudioPlayerOutput(long pitch):AudioPlayerOutput(){
    mPitch = pitch;
}

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
    
    if (mPitch != 0) {
        UInt32 trueValue = 1;
        AudioQueueSetProperty(mQueue, kAudioQueueProperty_EnableTimePitch, &trueValue, sizeof(trueValue));
        UInt32 timePitchAlgorithm = kAudioQueueTimePitchAlgorithm_Spectral; // supports rate and pitch
        AudioQueueSetProperty(mQueue, kAudioQueueProperty_TimePitchAlgorithm, &timePitchAlgorithm, sizeof(timePitchAlgorithm));
        
        AudioQueueSetParameter(mQueue, kAudioQueueParam_Pitch, mPitch);
    }
    
    AudioQueueStart(mQueue, NULL);
}

void AudioPlayerOutput::fillSamplesInBuffer(AudioQueueBufferRef buffer) {
    if(mSamples){
        buffer->mAudioDataByteSize = mSamplesSize;
        memcpy(buffer->mAudioData, mSamples, mSamplesSize);
        mSamples = NULL;
    }
    AudioQueueEnqueueBuffer(mQueue, buffer, 0, NULL);
}

void AudioPlayerOutput::MyOutputBufferHandler(void *input, AudioQueueRef queue, AudioQueueBufferRef buffer){
    AudioPlayerOutput *player = (AudioPlayerOutput *)input;
    player->fillSamplesInBuffer(buffer);
}