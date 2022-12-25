#include "VideoChannel.h"

VideoChannel::VideoChannel()
: x264_encode_picture(nullptr),
x264_video_codec(nullptr) {}

VideoChannel::~VideoChannel() {
    if(x264_encode_picture) {
        x264_picture_clean(x264_encode_picture);
        delete x264_encode_picture;
        x264_encode_picture = nullptr;
    }
}

void VideoChannel::setVideoEncoderParams(int width, int height, int fps, int bitrate) {
    this->width = width;
    this->height = height;
    this->fps = fps;
    this->bitrate = bitrate;
    y_byte_count = width * height;
    uv_byte_count = (y_byte_count >> 2);
    
    x264_param_t x264_param;
    
    x264_param_default_preset(&x264_param, "ultrafast", "zerolatency");
    
    x264_param.i_level_idc = 32;
    x264_param.i_csp = X264_CSP_I420;
    x264_param.i_width = this->width;
    x264_param.i_height = this->height;
    
    x264_param.rc.i_rc_method = X264_RC_ABR;
    x264_param.rc.i_bitrate = this->bitrate / 1000;
    x264_param.rc.i_vbv_max_bitrate = this->bitrate / 1000 * 1.2;
    x264_param.rc.i_vbv_buffer_size = this->bitrate / 1000;
    
    x264_param.i_fps_num = this->fps;
    x264_param.i_fps_den = 1;
    x264_param.i_timebase_num = x264_param.i_fps_den;
    x264_param.i_timebase_den = x264_param.i_fps_num;

    x264_param.b_repeat_headers = 1;
    x264_param.b_vfr_input = 0;
    x264_param.i_keyint_max = fps * 2;
    x264_param.i_bframe = 0;
    x264_param.i_threads = 1;

    if(x264_encode_picture) {
        x264_picture_clean(x264_encode_picture);
        delete x264_encode_picture;
        x264_encode_picture = nullptr;
    }

    x264_encode_picture = new x264_picture_t;
    x264_picture_alloc(x264_encode_picture, X264_CSP_I420, this->width, this->height);
    
    if(x264_video_codec) {
        x264_encoder_close(x264_video_codec);
        x264_video_codec = nullptr;
    }

    x264_video_codec = x264_encoder_open(&x264_param);
}

void VideoChannel::setRTMPPacketCallBack(RTMPPacketCallBack callback) {
    this->data_pack_up = callback;
}

void VideoChannel::encodeData(uint8_t* data) {
    memcpy(x264_encode_picture->img.plane[0], data, y_byte_count);
    for(int i=0; i < uv_byte_count; ++i) {
        *(x264_encode_picture->img.plane[1] + i) = *(data + y_byte_count + i*2 + 1);
        *(x264_encode_picture->img.plane[2] + i) = *(data + y_byte_count + i*2);
    }
    
    x264_nal_t* pp_nal;
    int pi_nal;
    x264_picture_t pic_out;

    x264_encoder_encode(x264_video_codec, &pp_nal, &pi_nal, x264_encode_picture, &pic_out);

    int sps_len;
    uint8_t sps[100];
    int pps_len;
    uint8_t pps[100];

    for(int i=0; i < pi_nal; ++i) {
        if(pp_nal[i].i_type == NAL_SPS) {
            sps_len = pp_nal[i].i_payload - 4;
            memcpy(sps, pp_nal[i].p_payload + 4, sps_len);
        }else if(pp_nal[i].i_type == NAL_PPS) {
            pps_len = pp_nal[i].i_payload - 4;
            memcpy(pps, pp_nal[i].p_payload + 4, pps_len);
            sendSpsPpsToRtmpServer(sps, sps_len, pps, pps_len);
        }else {
            sendFrameToRtmpServer(pp_nal[i].i_type, pp_nal[i].p_payload, pp_nal[i].i_payload);
        }
    }
}

void VideoChannel::sendSpsPpsToRtmpServer(uint8_t* sps, int sps_len, uint8_t* pps, int pps_len) {
    RTMPPacket* rtmp_packet = new RTMPPacket;    
    int rtmp_packet_size = 10 + 3 + sps_len + 3 + pps_len;
    RTMPPacket_Alloc(rtmp_packet, rtmp_packet_size);
    int next_pos = 0;
    rtmp_packet->m_body[next_pos++] = 0x17;
    rtmp_packet->m_body[next_pos++] = 0x00;
    rtmp_packet->m_body[next_pos++] = 0x00;
    rtmp_packet->m_body[next_pos++] = 0x00;
    rtmp_packet->m_body[next_pos++] = 0x00;
    rtmp_packet->m_body[next_pos++] = 0x01;
    rtmp_packet->m_body[next_pos++] = sps[1];
    rtmp_packet->m_body[next_pos++] = sps[2];
    rtmp_packet->m_body[next_pos++] = sps[3];
    rtmp_packet->m_body[next_pos++] = 0xFF;
    
    rtmp_packet->m_body[next_pos++] = 0xE1;
    rtmp_packet->m_body[next_pos++] = (sps_len >> 8) & 0xFF;
    rtmp_packet->m_body[next_pos++] = (sps_len & 0xFF);
    
    memcpy(rtmp_packet->m_body + next_pos, sps, sps_len);
    next_pos += sps_len;

    rtmp_packet->m_body[next_pos++] = 0x01;
    rtmp_packet->m_body[next_pos++] = (pps_len >> 8) & 0xFF;
    rtmp_packet->m_body[next_pos++] = (pps_len & 0xFF);

    memcpy(rtmp_packet->m_body + next_pos, pps, pps_len);
    next_pos += pps_len;

    rtmp_packet->m_packetType = RTMP_PACKET_TYPE_VIDEO;
    rtmp_packet->m_nBodySize = rtmp_packet_size;
    rtmp_packet->m_nChannel = 10;
    rtmp_packet->m_nTimeStamp = 0;
    rtmp_packet->m_hasAbsTimestamp = 0;
    rtmp_packet->m_headerType = RTMP_PACKET_SIZE_MEDIUM;
    
    data_pack_up(rtmp_packet);
}

void VideoChannel::sendFrameToRtmpServer(int type, uint8_t* p_payload, int payload) {
    if(p_payload[2] == 0x00) {
        // 00 00 00 01
        payload -= 4;
        p_payload += 4;    
    }else {
        // 00 00 01
        payload -= 3;
        p_payload += 3;
    }
    
    RTMPPacket* rtmp_packet = new RTMPPacket;
    int rtmp_packet_size = 9 + payload;
    RTMPPacket_Alloc(rtmp_packet, rtmp_packet_size);
    RTMPPacket_Reset(rtmp_packet);
    
    rtmp_packet->m_body[0] = 0x27;
    if(type == NAL_SLICE_IDR) {
        rtmp_packet->m_body[0] = 0x17;
    }

    rtmp_packet->m_body[1] = 0x01;
    rtmp_packet->m_body[2] = 0x00;
    rtmp_packet->m_body[3] = 0x00;
    rtmp_packet->m_body[4] = 0x00;
    
    rtmp_packet->m_body[5] = (payload >> 24) & 0xFF;
    rtmp_packet->m_body[6] = (payload >> 16) & 0xFF;
    rtmp_packet->m_body[7] = (payload >> 8) & 0xFF;
    rtmp_packet->m_body[8] = (payload & 0xFF);

    memcpy(rtmp_packet->m_body + 9, p_payload, payload);

    rtmp_packet->m_packetType = RTMP_PACKET_TYPE_VIDEO;
    rtmp_packet->m_nBodySize = rtmp_packet_size;
    rtmp_packet->m_nChannel = 10;
    rtmp_packet->m_hasAbsTimestamp = 0;
    rtmp_packet->m_headerType = RTMP_PACKET_SIZE_MEDIUM;
    
    data_pack_up(rtmp_packet);
}