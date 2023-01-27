#include "VideoCapture.h"
#include <android/log.h>

VideoCapture::VideoCapture()
: naluCallBack(nullptr)
, audioCallBack(nullptr)
, rtmp_url("")
, is_pulling(false)
, rtmp(nullptr)
, packet(nullptr) {

}

VideoCapture::~VideoCapture() {
    stop();
}

void VideoCapture::setNaluCB(NaluCallBack cb) {
    this->naluCallBack = cb;
}

void VideoCapture::setAudioCB(AudioCallBack cb) {
    this->audioCallBack = cb;
}

void VideoCapture::setRtmpURL(const string & url) {
    this->rtmp_url = url;
}

bool VideoCapture::start() {
    if(is_pulling) return false;
    is_pulling = true;

    if(!this->init()) {
        is_pulling = false;
        return false;
    }

    this->pull();
    this->unInit();

    is_pulling = false;

    return true;
}

void VideoCapture::stop() {
    if(!is_pulling) return;
    is_pulling = false;
}

bool VideoCapture::init() {
    unInit();

    rtmp = RTMP_Alloc();
    RTMP_Init(rtmp);

    int ret = RTMP_SetupURL(rtmp, const_cast<char*>(rtmp_url.c_str()));
    if(!ret) return false;

    rtmp->Link.timeout = 5;
    rtmp->Link.lFlags |= RTMP_LF_LIVE;
    ret = RTMP_Connect(rtmp, NULL);
    if(!ret) return false;

    ret = RTMP_ConnectStream(rtmp, 0);
    if(!ret) return false;

    packet = new RTMPPacket;
    return true;
}

void VideoCapture::unInit() {
    if(rtmp) {
        RTMP_Close(rtmp);
        RTMP_Free(rtmp);
        rtmp = nullptr;
    }
    if(packet) {
        delete packet;
        packet = nullptr;
    }
}

void VideoCapture::pull() {
    while(is_pulling) {
        memset(packet, 0, sizeof(RTMPPacket));
        int ret = RTMP_ReadPacket(rtmp, packet);
        __android_log_print(ANDROID_LOG_INFO, "RTMP", "read packet ret: %d", ret);
        if(ret == 0) {
            // 若考虑网络问题造成的断流，需要修改rtmpdump库
            // continue;
            break;
        }else if(ret < 0) {
            break;
        }
        if(RTMPPacket_IsReady(packet)) {
            RTMP_ClientPacket(rtmp, packet);
            handle_packet(packet);
        }
    }
}

void VideoCapture::handle_packet(RTMPPacket* packet) {
    if(packet->m_packetType == RTMP_PACKET_TYPE_VIDEO) {
        bool sequence = (0x00 == (BYTE)packet->m_body[1]);
        if(sequence) {
            handle_sps_pps_packet(packet);
        }else {
            handle_nalu_frame_packet(packet);
        }
    }else if(packet->m_packetType == RTMP_PACKET_TYPE_AUDIO) {
        bool decodeinfo = (0x00 == (BYTE)packet->m_body[1]);
        if(decodeinfo) {
            handle_audio_decode_info_packet(packet);
        }else {
            handle_audio_data_packet(packet);
        }
    }
}

void VideoCapture::handle_sps_pps_packet(RTMPPacket* packet) {
    UINT32 offset = 10;
    UINT32 sps_num = packet->m_body[offset++] & 0x1F;
    for(UINT32 i=0; i < sps_num; ++i) {
        UINT32 num0 = packet->m_body[offset++] & 0xFF;
        UINT32 num1 = packet->m_body[offset++] & 0xFF;
        UINT32 sps_len = ((num0 << 8) | num1);
        naluCallBack(NALU_TYPE_SPS, (BYTE*)packet->m_body + offset, sps_len);
        offset += sps_len;
    }
    UINT32 pps_num = packet->m_body[offset++] & 0x1F;
    for(UINT32 i=0; i < pps_num; ++i) {
        UINT32 num0 = packet->m_body[offset++] & 0xFF;
        UINT32 num1 = packet->m_body[offset++] & 0xFF;
        UINT32 pps_len = ((num0 << 8) | num1);
        naluCallBack(NALU_TYPE_PPS, (BYTE*)packet->m_body + offset, pps_len);
        offset += pps_len;
    }
}

void VideoCapture::handle_nalu_frame_packet(RTMPPacket* packet) {
    UINT32 offset = 5;
    UINT32 num0 = packet->m_body[offset++] & 0xFF;
    UINT32 num1 = packet->m_body[offset++] & 0xFF;
    UINT32 num2 = packet->m_body[offset++] & 0xFF;
    UINT32 num3 = packet->m_body[offset++] & 0xFF;
    UINT32 data_len = ((num0 << 24) | (num1 << 16) | (num2 << 8) | num3);
    if((uint8_t)packet->m_body[0] == 0x17)
        naluCallBack(NALU_TYPE_IDR, (BYTE*)packet->m_body + offset, data_len);
    else
        naluCallBack(NALU_TYPE_OTHER, (BYTE*)packet->m_body + offset, data_len);
    offset += data_len;
}

void VideoCapture::handle_audio_decode_info_packet(RTMPPacket* packet) {
    UINT32 offset = 2;
    UINT32 data_len = packet->m_nBodySize - offset;
    if((uint8_t)packet->m_body[0] == 0xAE)
        audioCallBack(AUDIO_INFO, AUDIO_CHANNEL_MONO, (BYTE*)packet->m_body + offset, data_len);
    else
        audioCallBack(AUDIO_INFO, AUDIO_CHANNEL_STEREO, (BYTE*)packet->m_body + offset, data_len);
    offset += data_len;
}

void VideoCapture::handle_audio_data_packet(RTMPPacket* packet) {
    UINT32 offset = 2;
    UINT32 data_len = packet->m_nBodySize - offset;
    if((uint8_t)packet->m_body[0] == 0xAE)
        audioCallBack(AUDIO_DATA, AUDIO_CHANNEL_MONO, (BYTE*)packet->m_body + offset, data_len);
    else
        audioCallBack(AUDIO_DATA, AUDIO_CHANNEL_STEREO, (BYTE*)packet->m_body + offset, data_len);
    offset += data_len;
}
