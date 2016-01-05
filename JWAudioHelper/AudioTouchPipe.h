//
//  AudioTouchPipe.hpp
//  AudioQueueServiceDemo
//
//  Created by Jerry Wong's Mac Mini on 15/11/23.
//  Copyright © 2015年 Jerry Wong. All rights reserved.
//

#ifndef AudioTouchPipe_h
#define AudioTouchPipe_h

#include "AudioToolkit.h"
#include "SoundTouch.h"

namespace JWAudioToolkit
{

class AudioTouchPipe:public AudioProcessorPipe
{
public:
    AudioTouchPipe(AudioProcessorPipe *pOutput, float pitch);
    ~AudioTouchPipe();
    
    void putSamples(SInt16 *samples, UInt32 size);
    void clear();
    
private:
    soundtouch::SoundTouch *mSoundTouch;
};
    
}

#endif /* AudioTouchPipe_h */
