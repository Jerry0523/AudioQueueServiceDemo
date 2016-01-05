//
//  AudioRecorderInput.h
//  AudioQueueServiceDemo
//
//  Created by Jerry Wong's Mac Mini on 15/11/23.
//  Copyright © 2015年 Jerry Wong. All rights reserved.
//

#ifndef AudioRecorderInput_h
#define AudioRecorderInput_h

#include "AudioToolkit.h"
#include <sys/time.h>

namespace JWAudioToolkit
{

typedef void (*AudioRecorderAnalysisCallbackFunc)(void*, float, float);
typedef struct AudioMeterTable {
        float	mMinDecibels;
        float	mDecibelResolution;
        float	mScaleFactor;
        float	*mTable;
} AudioMeterTable;
    
inline double DbToAmp(double inDb) {
    return pow(10.0, 0.05 * inDb);
}
    
class AudioRecorderInput:public AudioInput{
    
public:
    AudioRecorderInput(AudioProcessorPipe *output);
    AudioRecorderInput(AudioProcessorPipe *output, AudioRecorderAnalysisCallbackFunc mCallBackFunc, void* userData);
    ~AudioRecorderInput();
    void start();
    void stop();
    
private:
    AudioQueueBufferRef	mBuffers[kNumberRecordBuffers];
    UInt64 mRecordPacket;
    AudioQueueRef mQueue;
    
    AudioRecorderAnalysisCallbackFunc mCallBackFunc = NULL;
    AudioQueueLevelMeterState *mChanelLevels = NULL;
    AudioMeterTable *mMeterData = NULL;
    itimerval *mTick = NULL;
    void * mUserData = NULL;
    
    float parseDBValue(float inDecibels);
    
    static void MyInputBufferHandler(	void *								inUserData,
                                     AudioQueueRef						inAQ,
                                     AudioQueueBufferRef					inBuffer,
                                     const AudioTimeStamp *				inStartTime,
                                     UInt32								inNumPackets,
                                     const AudioStreamPacketDescription*	inPacketDesc);
    static void MyTickHander(int signal);
};

}

#endif /* AudioRecorderInput_h */
