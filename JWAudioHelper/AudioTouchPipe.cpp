//
//  AudioTouchPipe.cpp
//  AudioQueueServiceDemo
//
//  Created by Jerry Wong's Mac Mini on 15/11/23.
//  Copyright © 2015年 Jerry Wong. All rights reserved.
//

#include "AudioTouchPipe.h"

using namespace JWAudioToolkit;

AudioTouchPipe::AudioTouchPipe(AudioProcessorPipe *pOutput, float pitch):AudioProcessorPipe(pOutput){
    mSoundTouch = new soundtouch::SoundTouch();
    mSoundTouch->setSampleRate(kRecordSampleRate);
    mSoundTouch->setChannels(1);
    mSoundTouch->setTempoChange(0);
    mSoundTouch->setPitchSemiTones(pitch);
    mSoundTouch->setRateChange(0);
    mSoundTouch->setSetting(SETTING_SEQUENCE_MS, 40);
    mSoundTouch->setSetting(SETTING_SEEKWINDOW_MS, 15);
    mSoundTouch->setSetting(SETTING_OVERLAP_MS, 6);
}

AudioTouchPipe::~AudioTouchPipe(){
    delete mSoundTouch;
}

void AudioTouchPipe::putSamples(SInt16 *samples, UInt32 size) {
    AudioProcessorPipe *outputPipe = getOutput();
    if (samples != NULL && outputPipe) {
        SInt16 *output = new SInt16[size];
        mSoundTouch->putSamples(samples, size / 2.0);
        SInt16 *samples = new SInt16[size];
        int numSamples = 0;
        int idx = 0;
        do {
            memset(samples, 0, size);
            numSamples = mSoundTouch->receiveSamples(samples, size);
            memcpy(output + idx, samples, numSamples * 2);
            idx += numSamples * 2;
        } while (numSamples > 0);
        delete [] samples;
        outputPipe->putSamples(output, size);
    }
    delete samples;
}

void AudioTouchPipe::clear(){
    AudioProcessorPipe::clear();
    mSoundTouch->clear();
}