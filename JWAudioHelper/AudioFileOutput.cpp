//
//  AudioFileOutput.cpp
//  AudioQueueServiceDemo
//
//  Created by Jerry Wong's Mac Mini on 15/11/24.
//  Copyright © 2015年 Jerry Wong. All rights reserved.
//

#include "AudioFileOutput.h"

using namespace JWAudioToolkit;

AudioFileOutput::AudioFileOutput(const char *path):AudioProcessorPipe(){
    CFStringRef pathRef = CFStringCreateWithCString(kCFAllocatorDefault, path, kCFStringEncodingUTF8);
    mFileURL = CFURLCreateWithString(kCFAllocatorDefault, pathRef, NULL);
    CFRelease(pathRef);
}

AudioFileOutput::~AudioFileOutput() {
    if (mFileURL) {
        CFRelease(mFileURL);
    }
}

void AudioFileOutput::putSamples(SInt16 *samples, UInt32 size) {
    if (!mRecordFile) {
        assert(AudioFileCreateWithURL(mFileURL,
                                        kAudioFileWAVEType,
                                        &format,
                                        kAudioFileFlags_EraseFile,
                                        &mRecordFile) == noErr);
    }
    UInt32 ioNumPackets = size / format.mBytesPerPacket;
    assert(AudioFileWritePackets(mRecordFile,
                                 FALSE,
                                 size,
                                 NULL,
                                 packetIndex,
                                 &ioNumPackets,
                                 samples) == noErr);

    packetIndex += ioNumPackets;
    delete [] samples;

}

void AudioFileOutput::clear(){
    AudioProcessorPipe::clear();
    if (mRecordFile) {
        AudioFileClose(mRecordFile);
        mRecordFile = NULL;
    }
    
    packetIndex = 0;
}