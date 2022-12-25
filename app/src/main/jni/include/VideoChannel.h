#ifndef _VIDEO_CHANNEL_H_
#define _VIDEO_CHANNEL_H_

#include <inttypes.h>
#include "x264.h"
#include <memory>
#include <functional>
#include <memory.h>
#include <librtmp/rtmp.h>

class VideoChannel {
    
    using RTMPPacketCallBack = std::function<void(RTMPPacket*)>;

public:
    VideoChannel();
    ~VideoChannel();

public:
    void setVideoEncoderParams(int width, int height, int fps, int bitrate);
    void encodeData(uint8_t* data);
    void setRTMPPacketCallBack(RTMPPacketCallBack callback);

private:
    void sendSpsPpsToRtmpServer(uint8_t* sps, int sps_len, uint8_t* pps, int pps_len);
    void sendFrameToRtmpServer(int type, uint8_t* p_payload, int payload);

private:
    RTMPPacketCallBack data_pack_up;
    
    int width;
    int height;
    int fps;
    int bitrate;
    
    int y_byte_count;
    int uv_byte_count; 
    
    x264_picture_t* x264_encode_picture;
    x264_t* x264_video_codec;

};

#endif // _VIDEO_CHANNEL_H_