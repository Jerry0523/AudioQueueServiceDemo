//
//  AudioRecorderInput.cpp
//  AudioQueueServiceDemo
//
//  Created by Jerry Wong's Mac Mini on 15/11/23.
//  Copyright © 2015年 Jerry Wong. All rights reserved.
//

#include "AudioRecorderInput.h"
#include <functional>
#include <sys/signal.h>

using namespace JWAudioToolkit;

static AudioRecorderInput *INSTANCE;

AudioRecorderInput::AudioRecorderInput(AudioProcessorPipe *output){
    INSTANCE = this;
    running = false;
    
    format.mSampleRate = kRecordSampleRate;
    format.mChannelsPerFrame = kRecordChannels;
    format.mFormatID = kAudioFormatLinearPCM;
    
    format.mBitsPerChannel = kRecordBitsPerChannel;
    format.mFramesPerPacket = 1;
    format.mBytesPerFrame = (format.mBitsPerChannel / CHAR_BIT) * format.mChannelsPerFrame;
    format.mBytesPerPacket = format.mBytesPerFrame * format.mFramesPerPacket;
    format.mFormatFlags = kLinearPCMFormatFlagIsSignedInteger | kLinearPCMFormatFlagIsPacked;
    format.mReserved = 0;
    
    setOutput(output);
}

AudioRecorderInput::AudioRecorderInput(AudioProcessorPipe *output, AudioRecorderAnalysisCallbackFunc callBackFunc, void* userData):AudioRecorderInput(output){
    if (callBackFunc) {
        mCallBackFunc = callBackFunc;
        
        mChanelLevels = (AudioQueueLevelMeterState*)malloc(sizeof(AudioQueueLevelMeterState) * kRecordChannels);
        mMeterData = new AudioMeterTable;
        mTick = new itimerval;
        
        mMeterData->mMinDecibels = -80.0;
        mMeterData->mDecibelResolution = mMeterData->mMinDecibels / (400 - 1);
        mMeterData->mScaleFactor = 1 / mMeterData->mDecibelResolution;
        
        mMeterData->mTable = (float*)malloc(400*sizeof(float));
        
        double minAmp = DbToAmp(mMeterData->mMinDecibels);
        double ampRange = 1. - minAmp;
        double invAmpRange = 1. / ampRange;
        
        double rroot = 1. / 2.0;
        for (size_t i = 0; i < 400; ++i) {
            double decibels = i * mMeterData->mDecibelResolution;
            double amp = DbToAmp(decibels);
            double adjAmp = (amp - minAmp) * invAmpRange;
            mMeterData->mTable[i] = pow(adjAmp, rroot);
        }
        mUserData = userData;
    }
}

float AudioRecorderInput::parseDBValue(float inDecibels){
    if (inDecibels < mMeterData->mMinDecibels) return  0.;
    if (inDecibels >= 0.) return 1.;
    int index = (int)(inDecibels * mMeterData->mScaleFactor);
    return mMeterData->mTable[index];
}

AudioRecorderInput::~AudioRecorderInput(){
    stop();
    mCallBackFunc = NULL;
    if (mChanelLevels) {
         free(mChanelLevels);
    }
    if (mMeterData) {
        delete mMeterData;
    }
    if (mTick) {
        delete mTick;
    }
    mUserData = NULL;
}

void AudioRecorderInput::MyTickHander(int signal) {
    if(signal == SIGALRM && INSTANCE) {
        UInt32 data_sz = sizeof(AudioQueueLevelMeterState) * kRecordChannels;
        OSStatus status = AudioQueueGetProperty(INSTANCE->mQueue, kAudioQueueProperty_CurrentLevelMeterDB, INSTANCE->mChanelLevels, &data_sz);
        if (status == noErr) {
            float peakValue = INSTANCE->parseDBValue(INSTANCE->mChanelLevels[0].mPeakPower);
            float averageValue = INSTANCE->parseDBValue(INSTANCE->mChanelLevels[0].mAveragePower);
            if (INSTANCE->mCallBackFunc) {
                INSTANCE->mCallBackFunc(INSTANCE->mUserData, peakValue, averageValue);
            }
        }
    }
}

void AudioRecorderInput::start(){
    assert(AudioQueueNewInput(&format,
                              MyInputBufferHandler,
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
        assert(AudioQueueEnqueueBuffer(mQueue, mBuffers[i], 0, NULL) == noErr);
    }
    assert(AudioQueueStart(mQueue, NULL) == noErr);
    if (mCallBackFunc) {
        INSTANCE = this;
        UInt32 val = 1;
        if (AudioQueueSetProperty(mQueue, kAudioQueueProperty_EnableLevelMetering, &val, sizeof(UInt32)) == noErr) {
            signal(SIGALRM, MyTickHander);
            
            mTick->it_value.tv_sec = 0;
            mTick->it_value.tv_usec = 80000;
            
            mTick->it_interval.tv_sec = 0;
            mTick->it_interval.tv_usec = 80000;
            
            setitimer(ITIMER_REAL, mTick, NULL);
        }
    }
    running = true;
}

void AudioRecorderInput::stop(){
    if (running) {
        AudioQueueStop(mQueue, TRUE);
    }
    if (mQueue) {
        assert(AudioQueueDispose(mQueue, TRUE) == noErr);
        mQueue = NULL;
    }
    
    if (mCallBackFunc) {
        mCallBackFunc(mUserData, 0, 0);
        
        INSTANCE = NULL;
        mTick->it_value.tv_sec = 0;
        mTick->it_value.tv_usec = 0;
        
        mTick->it_interval.tv_sec = 0;
        mTick->it_interval.tv_usec = 0;
        setitimer(ITIMER_REAL, mTick, NULL);
    }
    
    AudioProcessorPipe *output = getOutput();
    if (output) {
        output->clear();
    }
    running = false;
}

void AudioRecorderInput::MyInputBufferHandler(void* inUserData,
                                         AudioQueueRef inAQ,
                                         AudioQueueBufferRef inBuffer,
                                         const AudioTimeStamp* inStartTime,
                                         UInt32 inNumPackets,
                                         const AudioStreamPacketDescription* inPacketDesc){
    AudioRecorderInput *recorder = (AudioRecorderInput *)inUserData;
    if (inNumPackets > 0) {
        AudioProcessorPipe *output = recorder->getOutput();
        if (output) {
            SInt16 *outBuffer = new SInt16[inBuffer->mAudioDataBytesCapacity];
            memcpy(outBuffer, inBuffer->mAudioData, inBuffer->mAudioDataBytesCapacity);
            output->putSamples(outBuffer, inBuffer->mAudioDataBytesCapacity);
        }
        
        recorder->mRecordPacket += inNumPackets;
    }
    if (recorder->running) {
        AudioQueueEnqueueBuffer(inAQ, inBuffer, 0, NULL);
    }
}
