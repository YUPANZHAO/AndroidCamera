#ifndef _AUDIO_CHANNEL_H_
#define _AUDIO_CHANNEL_H_

#include "faac.h"
#include <memory.h>
#include <inttypes.h>
#include "librtmp/rtmp.h"
#include <thread>
#include <functional>

using namespace std;

class AudioChannel {

    using RTMPPacketCallBack = function<void(RTMPPacket*)>;

public:
    AudioChannel();
    ~AudioChannel();

public:
    void setAudioEncodeParams(int sampleRateInHz, int channelCfg);
    void setRTMPPacketCallBack(RTMPPacketCallBack callback);
    int getInputSamples();
    void encodeAudioData(uint8_t* data);
    RTMPPacket* getAudioDecodeInfo();

private:
    RTMPPacketCallBack data_pack_up;
    
    int channelCfg;
    unsigned long inputSamples;
    unsigned long maxOutputBytes;

    faacEncHandle faacEncHandle;
    uint8_t* faacEncodeOutputBuffer;

};

#endif // _AUDIO_CHANNEL_H_