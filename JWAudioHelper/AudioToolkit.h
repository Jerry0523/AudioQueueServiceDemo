//
//  AudioToolkit.h
//  AudioQueueServiceDemo
//
//  Created by Jerry Wong's Mac Mini on 15/11/23.
//  Copyright © 2015年 Jerry Wong. All rights reserved.
//

#ifndef AudioToolkit_h
#define AudioToolkit_h

#define kRecordSampleRate 44100
#define kRecordChannels 1
#define kRecordBitsPerChannel 16

#define kNumberRecordBuffers	3
#define kBufferDurationSeconds 0.5

#include <AudioToolbox/AudioToolbox.h>
#include <CoreFoundation/CoreFoundation.h>

namespace JWAudioToolkit
{
    
class AudioToolkitObject
{
public:
    void setFormat(AudioStreamBasicDescription aFormat){
        format = aFormat;
        if (output) {
            output->format = aFormat;
        }
    }
    
protected:
    AudioStreamBasicDescription format;
    
    virtual AudioToolkitObject* getOutput(){
        return output;
    }
    
    virtual void setOutput(AudioToolkitObject *pOutput){
        output = pOutput;
        if (output) {
            output->setFormat(format);
        }
    }
    
    static int	ComputeRecordBufferSize(const AudioStreamBasicDescription *format,AudioQueueRef queue, float seconds){
        int packets, frames, bytes = 0;
        frames = (int)ceil(seconds * format->mSampleRate);
        
        if (format->mBytesPerFrame > 0)
            bytes = frames * format->mBytesPerFrame;
        else {
            UInt32 maxPacketSize;
            if (format->mBytesPerPacket > 0)
                maxPacketSize = format->mBytesPerPacket;
            else {
                UInt32 propertySize = sizeof(maxPacketSize);
                assert(AudioQueueGetProperty(queue, kAudioQueueProperty_MaximumOutputPacketSize, &maxPacketSize, &propertySize) == noErr);
            }
            if (format->mFramesPerPacket > 0) {
                packets = frames / format->mFramesPerPacket;
            } else {
                packets = frames;
            }
            
            if (packets == 0) {
                packets = 1;
            }
            bytes = packets * maxPacketSize;
        }
        return bytes;
    }
private:
    AudioToolkitObject *output = NULL;
};
    
class AudioProcessorPipe:public AudioToolkitObject
{
public:
    
    virtual void putSamples(SInt16 *samples, UInt32 size) = 0;
    
    virtual void clear(){
        AudioProcessorPipe *outputPipe = getOutput();
        if (outputPipe) {
            outputPipe->clear();
        }
    }
    
    virtual ~AudioProcessorPipe(){
        clear();
        AudioProcessorPipe *outputPipe = getOutput();
        if (outputPipe) {
            delete outputPipe;
        }
    }
    
protected:
    AudioProcessorPipe(){
        setOutput(NULL);
    }
        
    AudioProcessorPipe(AudioProcessorPipe *pOutput){
        setOutput(pOutput);
    }
    
    virtual AudioProcessorPipe* getOutput(){
        return (AudioProcessorPipe*)AudioToolkitObject::getOutput();
    }
    
    
    virtual void setOutput(AudioProcessorPipe *pOutput){
        AudioProcessorPipe *outputPipe = getOutput();
        if (outputPipe) {
            outputPipe->clear();
        }
        AudioToolkitObject::setOutput(pOutput);
    }
};

class AudioInput:public AudioToolkitObject
{
public:
    virtual void start() = 0;
    virtual void stop() = 0;
        
    bool isRunning(){
        return running;
    }
        
    virtual AudioProcessorPipe* getOutput(){
        return (AudioProcessorPipe*)AudioToolkitObject::getOutput();
    }
    
    virtual void setOutput(AudioProcessorPipe *pOutput){
        AudioProcessorPipe *outputPipe = getOutput();
        if (outputPipe) {
            outputPipe->clear();
        }
        AudioToolkitObject::setOutput(pOutput);
    }
    
protected:
    bool running;
    virtual ~AudioInput(){
        AudioProcessorPipe *outputPipe = getOutput();
        if (outputPipe) {
            outputPipe->clear();
            delete outputPipe;
        }
    }
};
    
}


#endif /* AudioToolkit_h */
