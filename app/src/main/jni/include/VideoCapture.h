#ifndef VIDEOCAPTURE_H
#define VIDEOCAPTURE_H

#include "librtmp/rtmp.h"
#include <functional>
#include "types.h"
#include <memory>
#include <thread>
#include <unistd.h>

using namespace std;
using namespace ZYP;

class VideoCapture {

    using NaluCallBack = function<void(NALU_TYPE,BYTE*,UINT32)>;
    using AudioCallBack = function<void(AUDIO_TYPE,AUDIO_CHANNEL_TYPE,BYTE*,UINT32)>;

public:
    VideoCapture();
    ~VideoCapture();

public:
    void setNaluCB(NaluCallBack cb);
    void setAudioCB(AudioCallBack cb);
    void setRtmpURL(const string & url);
    bool start();
    void stop();

private:
    bool init();
    void unInit();
    void pull();

    void handle_packet(RTMPPacket* packet);
    void handle_sps_pps_packet(RTMPPacket* packet);
    void handle_nalu_frame_packet(RTMPPacket* packet);

    void handle_audio_decode_info_packet(RTMPPacket* packet);
    void handle_audio_data_packet(RTMPPacket* packet);

    NaluCallBack naluCallBack;
    AudioCallBack audioCallBack;
    string rtmp_url;
    bool is_pulling;
    RTMP* rtmp;
    RTMPPacket* packet;
};

#endif // VIDEOCAPTURE_H
