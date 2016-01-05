//
//  AudioFileOutput.h
//  AudioQueueServiceDemo
//
//  Created by Jerry Wong's Mac Mini on 15/11/24.
//  Copyright © 2015年 Jerry Wong. All rights reserved.
//

#ifndef AudioFileOutput_h
#define AudioFileOutput_h

#include "AudioToolkit.h"

namespace JWAudioToolkit
{
class AudioFileOutput:public AudioProcessorPipe
{
public:
    AudioFileOutput(const char *path);
    void putSamples(SInt16 *samples, UInt32 size);
    void clear();
    ~AudioFileOutput();
        
private:
    AudioFileID	mRecordFile;
    CFURLRef mFileURL;
    SInt64 packetIndex = 0;
};
}

#endif /* AudioFileOutput_h */
