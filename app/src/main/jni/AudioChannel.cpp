#include "AudioChannel.h"
#include <android/log.h>

AudioChannel::AudioChannel()
: data_pack_up(nullptr)
, channelCfg(0)
, inputSamples(0)
, maxOutputBytes(0)
, faacEncHandle(nullptr)
, faacEncodeOutputBuffer(nullptr) {

}

AudioChannel::~AudioChannel() {
    if(faacEncodeOutputBuffer) {
        delete [] faacEncodeOutputBuffer;
        faacEncodeOutputBuffer = nullptr;
    }
    if(faacEncHandle) {
        delete faacEncHandle;
        faacEncHandle = nullptr;
    }
}

void AudioChannel::setAudioEncodeParams(int sampleRateInHz, int channelCfg) {
    this->channelCfg = channelCfg;
    
    faacEncHandle = faacEncOpen(sampleRateInHz, channelCfg, &inputSamples, &maxOutputBytes);

    faacEncConfigurationPtr configurationPtr = faacEncGetCurrentConfiguration(faacEncHandle);
    
    configurationPtr->mpegVersion = MPEG4;
    
    configurationPtr->aacObjectType = LOW;

    configurationPtr->inputFormat = FAAC_INPUT_16BIT;
    
    configurationPtr->outputFormat = 0;

    faacEncSetConfiguration(faacEncHandle, configurationPtr);

    faacEncodeOutputBuffer = new uint8_t [maxOutputBytes];
}

void AudioChannel::setRTMPPacketCallBack(RTMPPacketCallBack callback) {
    this->data_pack_up = callback;
}

int AudioChannel::getInputSamples() {
    return inputSamples;
}

void AudioChannel::encodeAudioData(uint8_t* data) {
    int len = faacEncEncode(  
                    faacEncHandle,
                    reinterpret_cast<int32_t*>(data),
                    inputSamples,
                    faacEncodeOutputBuffer,
                    maxOutputBytes);

    if(len > 0) {
        int rtmpPacketSzie = 2 + len;
        RTMPPacket* packet = new RTMPPacket;
        RTMPPacket_Alloc(packet, rtmpPacketSzie);
        
        packet->m_body[0] = (channelCfg == 1) ? 0xAE : 0xAF;
        packet->m_body[1] = 0x01;
        memcpy(packet->m_body + 2, faacEncodeOutputBuffer, len);

        packet->m_hasAbsTimestamp = 0;
        packet->m_nBodySize = rtmpPacketSzie;
        packet->m_packetType = RTMP_PACKET_TYPE_AUDIO;
        packet->m_nChannel = 0x11;
        packet->m_headerType = RTMP_PACKET_SIZE_LARGE;

        data_pack_up(packet);
    }
}

RTMPPacket* AudioChannel::getAudioDecodeInfo() {
    uint8_t* buf;
    unsigned long len;
    faacEncGetDecoderSpecificInfo(faacEncHandle, &buf, &len);
    
    int rtmpPacketSzie = 2 + len;
    RTMPPacket* packet = new RTMPPacket;
    RTMPPacket_Alloc(packet, rtmpPacketSzie);
    
    packet->m_body[0] = (channelCfg == 1) ? 0xAE : 0xAF;
    packet->m_body[1] = 0x00;
    memcpy(packet->m_body + 2, buf, len);

    packet->m_hasAbsTimestamp = 0;
    packet->m_nBodySize = rtmpPacketSzie;
    packet->m_packetType = RTMP_PACKET_TYPE_AUDIO;
    packet->m_nChannel = 0x11;
    packet->m_headerType = RTMP_PACKET_SIZE_LARGE;

    return packet;
}
